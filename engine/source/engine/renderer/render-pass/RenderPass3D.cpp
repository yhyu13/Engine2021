#include "engine-precompiled-header.h"
#include "RenderPass3D.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/ecs/components/3d/Particle3DCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"

bool longmarch::RenderPass3D::ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
{
	if (!m_vfcParam.enableVFCulling)
	{
		return false;
	}
	else
	{
		return BoudingVolume->VFCTest(m_vfcParam.VFinViewSpace, m_vfcParam.WorldSpaceToViewSpace);
	}
}

bool longmarch::RenderPass3D::DistanceCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
{
	if (!m_distanceCParam.enableDistanceCulling)
	{
		return false;
	}
	else
	{
		return BoudingVolume->DistanceTest(m_distanceCParam.center, m_distanceCParam.Near, m_distanceCParam.Far);
	}
}

void longmarch::RenderPass3D::SetVFCullingParam(bool enable, const ViewFrustum& VFinViewSpace, const Mat4& WorldSpaceToViewSpace)
{
	m_vfcParam.enableVFCulling = enable;
	m_vfcParam.VFinViewSpace = VFinViewSpace;
	m_vfcParam.WorldSpaceToViewSpace = WorldSpaceToViewSpace;
}

void longmarch::RenderPass3D::SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far)
{
	m_distanceCParam.enableDistanceCulling = enable;
	m_distanceCParam.center = center;
	m_distanceCParam.Near = Near;
	m_distanceCParam.Far = Far;
}

void longmarch::RenderPass3D::RenderWithCullingTest()
{
	for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE)
	{
		RenderWithModeOpaque(renderObj);
	}
	for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT)
	{
		RenderWithModeTransparent(renderObj);
	}
}

void longmarch::RenderPass3D::RenderWithModeOpaque(Renderer3D::RenderObj_CPU& renderObj)
{
	auto scene = renderObj.entity.GetComponent<Scene3DCom>();
	auto body = renderObj.entity.GetComponent<Body3DCom>();
	if (body.Valid())
	{
		if (const auto& bv = body->GetBoundingVolume(); bv)
		{
			if (DistanceCullingTest(bv))
			{
				scene->SetShouldDraw(false, false);
			}
			else if (ViewFustrumCullingTest(bv))
			{
				scene->SetShouldDraw(false, false);
			}
		}
	}
	scene->Draw(m_drawBind);
}

void longmarch::RenderPass3D::RenderWithModeTransparent(Renderer3D::RenderObj_CPU& renderObj)
{
	auto particle = renderObj.entity.GetComponent<Particle3DCom>();
	auto scene = renderObj.entity.GetComponent<Scene3DCom>();
	auto body = renderObj.entity.GetComponent<Body3DCom>();

	bool isParticle = particle.Valid();
	bool hasBody = body.Valid();

	if (hasBody)
	{
		if (const auto& bv = body->GetBoundingVolume(); bv)
		{
			if (DistanceCullingTest(bv))
			{
				scene->SetShouldDraw(false, false);
			}
			else if (ViewFustrumCullingTest(bv))
			{
				scene->SetShouldDraw(false, false);
			}
			if (isParticle)
			{
				particle->SetRendering(scene->GetShouldDraw());
			}
		}
	}
	if (isParticle)
	{
		particle->PrepareDrawDataWithViewMatrix(m_vfcParam.WorldSpaceToViewSpace);
		scene->Draw(particle.GetPtr(), m_drawBind_Particle);
	}
	else
	{
		scene->Draw(m_drawBind);
	}
}
