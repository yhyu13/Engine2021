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
