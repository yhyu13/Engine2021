#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/ecs/components/ChildrenCom.h"

namespace longmarch
{
#define MULTITHREAD_PRE_RENDER_UPDATE

	class Scene3DComSys final : public BaseComponentSystem 
	{
	public:
		enum class RenderMode
		{
			SCENE = 0,
			SHADOW,
		};
	public:
		NONCOPYABLE(Scene3DComSys);
		COMSYS_DEFAULT_COPY(Scene3DComSys);

		Scene3DComSys();
		//! Prepare lights and scene rendering data in pre-render phase
		virtual void PreRenderUpdate(double dt) override;
		virtual void Render();
		
		void RenderOpaqueObj(); 
		void RenderTransparentObj();

		inline void SetRenderMode(RenderMode mode)
		{
			m_RenderMode = mode;
		}

		inline void SetVFCullingParam(bool enable, const ViewFrustum& VF, const Mat4& WorldSpaceToViewFrustum)
		{
			m_vfcParam.enableVFCulling = enable;
			m_vfcParam.VF = VF;
			m_vfcParam.WorldSpaceToViewFrustum = WorldSpaceToViewFrustum;
		}

		inline void SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far)
		{
			m_distanceCParam.enableDistanceCulling = enable;
			m_distanceCParam.center = center;
			m_distanceCParam.Near = Near;
			m_distanceCParam.Far = Far;
		}

		inline void SetRenderShaderName(const std::string& shaderName)
		{
			m_RenderShaderName = shaderName;
		}

	private:
		void PrepareScene(double dt);
		void RecursivePrepareScene(double dt, const Entity& parent, Transform3DCom* parentTr, ChildrenCom* childChildrenCom, unsigned int level);
		void RenderWithModeOpaque(Renderer3D::RenderObj_CPU& renderObj);
		void RenderWithModeTransparent(Renderer3D::RenderObj_CPU& renderObj);

		inline bool ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
		{
			if (!m_vfcParam.enableVFCulling)
			{
				return false;
			}
			else
			{
				return BoudingVolume->VFCTest(m_vfcParam.VF, m_vfcParam.WorldSpaceToViewFrustum);
			}
		}

		inline bool DistanceCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
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

	private:
		struct VFCParam
		{
			bool enableVFCulling = { false };
			ViewFrustum VF;
			Mat4 WorldSpaceToViewFrustum;
		};
		VFCParam m_vfcParam;
		struct DistanceCParam
		{
			bool enableDistanceCulling = { false };
			Vec3f center;
			float Near;
			float Far;
		};
		DistanceCParam m_distanceCParam;
		std::string m_RenderShaderName = { "" };
		RenderMode m_RenderMode = { RenderMode::SCENE };
		bool m_enableDebugDraw{ true };
#ifdef MULTITHREAD_PRE_RENDER_UPDATE
		AtomicQueue<std::shared_future<void>> m_threadJob;
#endif
	};
}