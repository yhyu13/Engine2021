#pragma once
#include "engine/EngineEssential.h"
#include "engine/math/Geommath.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/physics/Shape.h"

namespace longmarch
{
	/**
	 * @brief Base class of render pass
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class RenderPass3D : public BaseAtomicClassNC
	{
	protected:
		struct VFCParam
		{
			bool enableVFCulling{ false };
			ViewFrustum VFinViewSpace;
			Mat4 WorldSpaceToViewSpace;
		};
		struct DistanceCParam
		{
			bool enableDistanceCulling{ false };
			Vec3f center;
			float Near;
			float Far;
		};

	public:
		NONCOPYABLE(RenderPass3D);
		RenderPass3D() = default;

		inline void SetParentWorld(GameWorld* world)
		{
			m_parentWorld = world;
		}

		virtual void Init() = 0;
		virtual void BeginRenderPass() = 0;
		virtual void EndRenderPass() = 0;

	protected:
		virtual void UpdateShader() { throw NotImplementedException(); };
		virtual void Render() { throw NotImplementedException(); };
		virtual void SubmitBatch() { throw NotImplementedException(); };
		virtual void ClearBatch() { throw NotImplementedException(); };
		virtual void Draw(const Renderer3D::RenderData_CPU& data) { throw NotImplementedException(); };
		virtual void DrawParticle(const Renderer3D::ParticleInstanceDrawData& data) { throw NotImplementedException(); };

		virtual void SetVFCullingParam(bool enable, const ViewFrustum& VFinViewSpace, const Mat4& WorldSpaceToViewSpace);
		virtual void SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far);
		virtual bool ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume);
		virtual bool DistanceCullingTest(const std::shared_ptr<Shape>& BoudingVolume);

		virtual void RenderWithCullingTest();
		virtual void RenderWithModeOpaque(Renderer3D::RenderObj_CPU& renderObj);
		virtual void RenderWithModeTransparent(Renderer3D::RenderObj_CPU& renderObj);

	protected:
		VFCParam m_vfcParam;
		DistanceCParam m_distanceCParam;
		std::function<void(const Renderer3D::RenderData_CPU&)> m_drawBind;
		std::function<void(const Renderer3D::ParticleInstanceDrawData&)> m_drawBind_Particle;
		std::function<void()> m_submitBatchBind;
		std::function<void()> m_clearBatchBind;
		GameWorld* m_parentWorld{ nullptr };
	};
}