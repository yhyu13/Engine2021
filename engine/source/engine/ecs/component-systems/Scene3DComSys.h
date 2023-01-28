#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/ecs/components/ChildrenCom.h"

namespace longmarch
{

	class Scene3DComSys final : public BaseComponentSystem 
	{
	public:
		NONCOPYABLE(Scene3DComSys);
		COMSYS_DEFAULT_COPY(Scene3DComSys);

		Scene3DComSys();
		//! Prepare lights and scene rendering data in pre-render phase
		virtual void PreRenderUpdate(double dt) override;
		virtual void PreRenderPass() override;
		
		void RenderOpaqueObj(); 
		void RenderTransparentObj();

		void SetVFCullingParam(bool enable, const ViewFrustum& VF, const Mat4& WorldSpaceToViewFrustum);
		void SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far);
		void SetRenderShaderName(const std::string& shaderName);

	private:
		void PrepareScene(double dt);
		void RecursivePrepareScene(const Entity& parent, Transform3DCom* parentTrCom, ChildrenCom* parentChildrenCom);
		void RenderWithModeOpaque(Renderer3D::RenderObj_CPU& renderObj);
		void RenderWithModeTransparent(Renderer3D::RenderObj_CPU& renderObj);
		void RenderWithModeParticle(Renderer3D::RenderObj_CPU& renderObj);

		bool ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume);
		bool DistanceCullingTest(const std::shared_ptr<Shape>& BoudingVolume);

	private:
		struct VFCParam
		{
			bool enableVFCulling = { false };
			ViewFrustum VFinViewSpace;
			Mat4 WorldSpaceToViewSpace;
		};
		struct DistanceCParam
		{
			bool enableDistanceCulling = { false };
			Vec3f center;
			float Near;
			float Far;
		};
		VFCParam m_vfcParam;
		DistanceCParam m_distanceCParam;
		std::string m_RenderShaderName;
		bool m_enableDebugDraw{ true };
		ConcurrentQueueNC<std::shared_future<void>> m_threadJobs;
	};
}
