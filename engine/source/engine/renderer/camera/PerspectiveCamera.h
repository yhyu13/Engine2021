#pragma once

#include "engine/Math/Geommath.h"

namespace longmarch
{
	enum class PerspectiveCameraType
	{
		FIRST_PERSON,
		LOOK_AT,
	};

	struct PerspectiveCameraSetting
	{
		float aspectRatioWbyH = { 1.0f };
		float fovy_rad = { PI * 0.5f };
		float nearZ = { .1f };
		float farZ = { 2000.0f };
		Vec2u viewportOrigin{ 0 }; //!< top left corner (in window space with no title bar, not in display/screen space)
		Vec2u viewportSize{ 1920,1080 };
	};

	class PerspectiveCamera
	{
	public:
		PerspectiveCamera() = default;
		explicit PerspectiveCamera(float fovy_rad, float aspectRatioWbyH, float nearZ, float farZ);
		void OnUpdate();
		void SetProjection(float fovy_rad, float aspectRatioWbyH, float nearZ, float farZ);
		void RecalculateProjectionMatrix();
		void RecalculateViewMatrix();
		void RecalculateViewFrustum();
		//! Viewport origin should be in window space that does not take title bar into account (with top left corner as the origin)
		void SetViewPort(const Vec2u& origin, const Vec2u& size);
		//! Given a cursor position in window space, output the ray origin and ray direction (mainly used for ray casting)
		bool GenerateRayFromCursorSpace(const Vec2u& in_cursor_pos, bool clip_viewport, bool invert_y, Vec3f& out_ray_origin, Vec3f& out_ray_dir) const;
		//! Given a contact plane equation, output the world space location of the ray shoot from cursor position
		bool CursorSpaceToWorldSpace(const Vec2u& in_cursor_pos, const Vec4f& in_plane, bool clip_viewport, bool invert_y, Vec3f& out_world_pos) const;
		//! Cursor space is defined as : origin at top left or bottom left, and dimension equals to resolution. If you defined top left as the origin, then invert_y must be true
		bool WorldSpaceToCursorSpace(const Vec3f& in_world_pos, bool invert_y, Vec2u& out_cursor_pos) const;
		//! Screen space is simply the clip space xy coordinates
		bool WorldSpaceToScreenSpace(const Vec3f& in_world_pos, Vec2f& out_ss_pos) const;
		//! Cursor space is defined as : origin at top left or bottom left, and dimension equals to resolution.If you defined top left as the origin, then invert_y must be true. Screen space is simply the clip space xy coordinates
		bool CursorSpaceToScreenSpace(const Vec2u& in_cursor_pos, bool clip_viewport, bool invert_y, Vec2f& out_ss_pos) const;
		const ViewFrustum GetViewFrustumInWorldSpace() const;

		void SetZoom(float z)
		{
			localZoom = (z > 0.1f) ? z : 0.1f;
		}
		float GetZoom() const
		{
			return localZoom;
		}
		void SetWorldPosition(const Vec3f& pos)
		{
			worldPosition = pos;
		}
		const Vec3f& GetWorldPosition() const
		{
			return worldPosition;
		}
		void SetGocalRotation(const Quaternion& rot)
		{
			globalRotation = glm::normalize(rot);
		}
		const Quaternion& GetGlobalRotation() const
		{
			return globalRotation;
		}
		void SetLookAt(const Vec3f& eye, const Vec3f& at)
		{
			worldPosition = eye;
			globalRotation = Geommath::FromVectorPair(Geommath::WorldFront, at - eye);
		}
		const ViewFrustum& GetViewFrustumInViewSpace() const
		{
			return m_frustumPlane;
		}
		const Mat4& GetViewMatrix() const
		{
			return m_ViewMatrix;
		}
		const Mat4& GetProjectionMatrix() const
		{
			return m_ProjectionMatrix;
		}
		const Mat4& GetReverseZProjectionMatrix() const
		{
			return m_ReverseZProjectionMatrix;
		}
		const Mat4 GetViewProjectionMatrix() const
		{
			return m_ProjectionMatrix * m_ViewMatrix;
		}
		const Mat4 GetPrevViewProjectionMatrix() const
		{
			return m_PrevProjectionMatrix * m_PrevViewMatrix;
		}
		const Mat4 GetReverseZViewProjectionMatrix() const
		{
			return m_ReverseZProjectionMatrix * m_ViewMatrix;
		}
		const Mat4 GetPrevReverseZViewProjectionMatrix() const
		{
			return m_PrevReverseZProjectionMatrix * m_PrevViewMatrix;
		}

	private:
		ViewFrustum m_frustumPlane;
		Mat4 m_PrevProjectionMatrix;
		Mat4 m_ProjectionMatrix;
		Mat4 m_PrevReverseZProjectionMatrix;
		Mat4 m_ReverseZProjectionMatrix;
		Mat4 m_PrevViewMatrix;
		Mat4 m_ViewMatrix;
		Quaternion globalRotation = { Geommath::UnitQuat };
		Vec3f worldPosition;
		float localZoom = { 0.1f };

	public:
		PerspectiveCameraType type = { PerspectiveCameraType::FIRST_PERSON };
		PerspectiveCameraSetting cameraSettings;
	};
}