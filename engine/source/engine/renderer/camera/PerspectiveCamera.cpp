#include "engine-precompiled-header.h"
#include "PerspectiveCamera.h"
#include "engine/window/Window.h"

longmarch::PerspectiveCamera::PerspectiveCamera(float fovy_rad, float aspectRatioWbyH, float nearZ, float farZ)
{
	SetProjection(fovy_rad, aspectRatioWbyH, nearZ, farZ);
}

void longmarch::PerspectiveCamera::OnUpdate()
{
	SetProjection(cameraSettings.fovy_rad, (Window::Resizable) ? float(Window::width) / float(Window::height) : cameraSettings.aspectRatioWbyH, cameraSettings.nearZ, cameraSettings.farZ);
	RecalculateProjectionMatrix();
	RecalculateViewMatrix();
	RecalculateViewFrustum();
}

void longmarch::PerspectiveCamera::SetProjection(float fovy_rad, float aspectRatioWbyH, float nearZ, float farZ)
{
	cameraSettings.fovy_rad = fovy_rad;
	cameraSettings.aspectRatioWbyH = aspectRatioWbyH;
	cameraSettings.nearZ = nearZ;
	cameraSettings.farZ = farZ;
}

void longmarch::PerspectiveCamera::RecalculateProjectionMatrix()
{
	m_PrevProjectionMatrix = m_ProjectionMatrix;
	m_ProjectionMatrix = Geommath::ProjectionMatrixZeroOne(cameraSettings.fovy_rad, cameraSettings.aspectRatioWbyH, cameraSettings.nearZ, cameraSettings.farZ);

	m_PrevReverseZProjectionMatrix = m_ReverseZProjectionMatrix;
	// Reverse Z project could adopt infinite far plane
	m_ReverseZProjectionMatrix = Geommath::ReverseZProjectionMatrixZeroOne(cameraSettings.fovy_rad, cameraSettings.aspectRatioWbyH, cameraSettings.nearZ, cameraSettings.farZ, true);
}

void longmarch::PerspectiveCamera::RecalculateViewMatrix()
{
	m_PrevViewMatrix = m_ViewMatrix;
	switch (type)
	{
	case longmarch::PerspectiveCameraType::LOOK_AT:
	{
		// m_ViewMatrix = Geommath::LookAtWorld(worldPosition, worldPosition + globalRotation * Geommath::WorldFront, Vec3f(0, 0, localZoom)); // Bad at looking straight up or straight down using lookat matrix due to gimbal lock
		m_ViewMatrix = Geommath::ToTranslateMatrix(Vec3f(0, 0, -localZoom)) * Geommath::ViewMatrix(worldPosition, globalRotation);
	}
	break;
	case longmarch::PerspectiveCameraType::FIRST_PERSON:
	{
		// m_ViewMatrix = Geommath::LookAtWorld(worldPosition, worldPosition + globalRotation * Geommath::WorldFront); // Bad at looking straight up or straight down using lookat matrix due to gimbal lock
		m_ViewMatrix = Geommath::ViewMatrix(worldPosition, globalRotation);
	}
	break;
	}
}

void longmarch::PerspectiveCamera::RecalculateViewFrustum()
{
	// Since the reverse Z projection has the same fruistum plane as the normal porjection, we calculate one
	m_frustumPlane = Geommath::Frustum::FromProjection(m_ProjectionMatrix);
}

void longmarch::PerspectiveCamera::SetViewPort(const Vec2u& origin, const Vec2u& size)
{
	cameraSettings.viewportOrigin = origin;
	cameraSettings.viewportSize = size;
}

bool longmarch::PerspectiveCamera::ScreenSpaceToWorldSpace(const Vec2u& in_ss_pos, bool clip_viewport, bool invert_y, const Vec4f& in_plane, Vec3f& out_world_pos) const
{
	Vec3f pick_ray_dir;
	Vec3f pick_ray_origin;
	// Convert the screen coordinates to a ray.
	if (!GenerateRayFromScreenSpace(in_ss_pos, clip_viewport, invert_y, pick_ray_origin, pick_ray_dir))
	{
		return false;
	}

	// Get the length of the 'adjacent' side of the virtual triangle formed
	// by the direction and normal.
	float proj_ray_length = Geommath::Plane::DotNormal(in_plane, pick_ray_dir);
	if (glm::abs(proj_ray_length) <= glm::epsilon<float>())
	{
		return false;
	}

	// Calculate distance to plane along its normal
	float distance = Geommath::Plane::Distance(in_plane, pick_ray_origin);

	// If both the "direction" and origin are on the same side of the plane
	// then we can't possibly intersect (perspective rule only)
	{
		int nSign1 = (distance > 0) ? 1 : (distance < 0) ? -1 : 0;
		int nSign2 = (proj_ray_length > 0) ? 1 : (proj_ray_length < 0) ? -1 : 0;
		if (nSign1 == nSign2)
		{
			return false;
		}
	} // End if perspective

	  // Calculate the actual interval (Distance along the adjacent side / length of
	  // adjacent side).
	distance /= -proj_ray_length;

	// Store the results
	out_world_pos = pick_ray_origin + (pick_ray_dir * distance);

	// Success!
	return true;
}

bool longmarch::PerspectiveCamera::GenerateRayFromScreenSpace(const Vec2u& in_ss_pos, bool clip_viewport, bool invert_y, Vec3f& out_ray_origin, Vec3f& out_ray_dir) const
{
	if (clip_viewport && (glm::any(glm::lessThanEqual(in_ss_pos, cameraSettings.viewportOrigin))
		|| glm::any(glm::greaterThanEqual(in_ss_pos, cameraSettings.viewportOrigin + cameraSettings.viewportSize))))
	{
		return false;
	}

	// Ensure we have an up-to-date projection and view matrix

	// Transform the pick position from view port space into camera space
	Vec3f far_;
	far_.x = (((2.0f * (in_ss_pos.x - cameraSettings.viewportOrigin.x)) / (cameraSettings.viewportSize.x)) - 1.0f);
	far_.y = (((2.0f * (in_ss_pos.y - cameraSettings.viewportOrigin.y)) / (cameraSettings.viewportSize.y)) - 1.0f);
	far_.z = 1.f;
	if (invert_y)
	{
		far_.y *= -1.0f;
	}
	Vec3f near_(far_.xy, 0);

	const auto& in_pv = Geommath::SmartInverse(GetViewProjectionMatrix());
	far_ = Geommath::Mat4ProdVec3(in_pv, far_);
	near_ = Geommath::Mat4ProdVec3(in_pv, near_);
	out_ray_origin = worldPosition;
	out_ray_dir = Geommath::Normalize(far_ - near_);
	return true;
}

bool longmarch::PerspectiveCamera::WorldSpaceToScreenSpace(const Vec3f& in_world_pos, Vec2u& out_ss_pos, bool invert_y) const
{
	const auto& pv = GetViewProjectionMatrix();
	auto ss = Geommath::Mat4ProdVec3(pv, in_world_pos);
	ss = 0.5f * ss + 0.5f; // [-1,1] to [0,1]
	if (invert_y)
	{
		ss.y = (1.0 - ss.y);
	}
	out_ss_pos = ss.xy * Vec2f(cameraSettings.viewportSize) + Vec2f(cameraSettings.viewportOrigin);
	return true;
}

const ViewFrustum longmarch::PerspectiveCamera::GetViewFrustumInWorldSpace() const
{
	ViewFrustum ret = GetViewFrustumInViewSpace();
	const auto& worldSpaceToViewFrustumSpace = GetViewMatrix();
	// Original: const auto& plane_tr = Geommath::SmartInverseTranspose(Geommath::SmartInverse(worldSpaceToViewFrustumSpace));
	// Simplified as: glm::transpose(worldSpaceToViewFrustumSpace);
	// Could be event more simplified as worldSpaceToViewFrustumSpace to take advantage of row vector operation
	const auto& plane_tr = worldSpaceToViewFrustumSpace;
	for (auto& pl : ret.planes)
	{
		pl = Geommath::Plane::Normalize(pl * plane_tr);
	}
	return ret;
}