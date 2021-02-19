#pragma once
#include "engine/renderer/render-pass/RenderPass3D.h"

namespace longmarch
{
	/**
	 * @brief Picking system render pass
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class PickingPass final : public RenderPass3D
	{
	public:
		NONCOPYABLE(PickingPass);
		PickingPass() = default;
		inline void SetParentWorld(GameWorld* world)
		{
			m_parentWorld = world;
		}
		virtual void Init() override;
		virtual void BeginRenderPass() override;
		virtual void EndRenderPass() override;

		Entity GetPickedEntity() const;

	protected:
		virtual void UpdateShader() override;
		virtual void Render() override;
		virtual void SubmitBatch() override;
		virtual void ClearBatch() override;
		virtual void Draw(const Renderer3D::RenderData_CPU& data) override;
		virtual void DrawParticle(const Renderer3D::ParticleInstanceDrawData& data) override;

	private:
		//! Return true if picking a valid entity
		bool GetPickedResult(uint32_t* id) const;

	private:
		constexpr static uint32_t texture_dim = { 1u }; //!< The picking system simply need 1 entity, so a 1x1 texture should suffice.

	private:
		LongMarch_Vector<int> m_multiDraw_PickingEntityId; // Picking
		LongMarch_Vector<Mat4> m_multiDraw_PickingTr; // Picking
		std::shared_ptr<ShaderStorageBuffer> m_multiDraw_ssbo_PickingModelTrsBuffer{ nullptr }; // Picking
		std::shared_ptr<ShaderStorageBuffer> m_multiDraw_ssbo_PickingEntityIdsBuffer{ nullptr }; // Picking

		PerspectiveCamera m_pickingCam;
		Entity m_pickedEntity;
		GameWorld* m_parentWorld{ nullptr };
		std::shared_ptr<FrameBuffer> m_renderTarget{ nullptr };
		std::shared_ptr<Texture2D> m_renderTexture{ nullptr };
		std::uint8_t m_blitData[texture_dim * texture_dim * 4u]{ 0u };
		bool m_shouldRead{ false };
	};
}