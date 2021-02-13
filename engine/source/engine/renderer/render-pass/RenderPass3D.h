#pragma once
#include "engine/EngineEssential.h"
#include "engine/math/Geommath.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/physics/Shape.h"

namespace longmarch
{
	/**
	 * @brief Base class of render pass
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class RenderPass3D : public BaseAtomicClassNC
	{
	protected:
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

	public:
		NONCOPYABLE(RenderPass3D);
		RenderPass3D() = default;

		virtual void Init() = 0;
		virtual void BeginRenderPass() = 0;
		virtual void EndRenderPass() = 0;

	protected:
		virtual void UpdateShader() = 0;
		virtual void Render() = 0;
		virtual void SubmitBatch() = 0;
		virtual void ClearBatch() = 0;
		virtual void RenderOne(Renderer3D::RenderObj_CPU& renderObj) = 0;
		virtual void Draw(const Renderer3D::RenderData_CPU& data) = 0;

		inline void SetVFCullingParam(bool enable, const ViewFrustum& VFinViewSpace, const Mat4& WorldSpaceToViewSpace)
		{
			m_vfcParam.enableVFCulling = enable;
			m_vfcParam.VFinViewSpace = VFinViewSpace;
			m_vfcParam.WorldSpaceToViewSpace = WorldSpaceToViewSpace;
		}

		inline void SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far)
		{
			m_distanceCParam.enableDistanceCulling = enable;
			m_distanceCParam.center = center;
			m_distanceCParam.Near = Near;
			m_distanceCParam.Far = Far;
		}

		inline virtual void RenderWithCulling()
		{
			for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE)
			{
				auto scene = renderObj.entity.GetComponent<Scene3DCom>();
				auto body = renderObj.entity.GetComponent<Body3DCom>();
				if (body.Valid())
				{
					if (const auto& bv = body->GetBV(); bv)
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
				RenderOne(renderObj);
			}
		}

		inline bool ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
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

	protected:
		VFCParam m_vfcParam;
		DistanceCParam m_distanceCParam;
		std::function<void(const Renderer3D::RenderData_CPU&)> m_drawBind;
		std::function<void()> m_submitBatchBind;
		std::function<void()> m_clearBatchBind;
		PerspectiveCamera* m_camera;
		size_t m_max_batch = { 1024u };
	};
}