#pragma once
#include "engine/renderer/render-pass/RenderPass3D.h"

namespace longmarch
{
	/**
	 * @brief Picking system render pass
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class PickingPass final : public RenderPass3D
	{
	public:
		NONCOPYABLE(PickingPass);
		PickingPass() = default;
		virtual void Init() override;
		virtual void BeginRenderPass() override;
		virtual void EndRenderPass() override;

		Entity GetPickedEntity() const;

	protected:
		virtual void UpdateShader() override;
		virtual void Render() override;

	private:
		void SubmitBatch();
		void ClearBatch();
		void Draw(const Renderer3D::RenderData_CPU& data);

		//! Return true if picking a valid entity
		bool GetPickedResult(uint32_t* id) const;

	private:
		constexpr static uint32_t texture_dim = { 1u }; //!< The picking system simply need 1 entity, so a 1x1 texture should suffice.
		constexpr static size_t max_batch{ 1024u };

	private:
		//! Implement Draw() and bind it here
		std::function<void(const Renderer3D::RenderData_CPU&)> m_drawBind;
		//! Implement SubmitBatch() and bind it here
		std::function<void()> m_submitBatchBind;
		//! Implement ClearBatch() and bind it here
		std::function<void()> m_clearBatchBind;

		LongMarch_Vector<int> m_multiDraw_PickingEntityId; // Picking
		LongMarch_Vector<Mat4> m_multiDraw_PickingTr; // Picking
		std::shared_ptr<ShaderStorageBuffer> m_multiDraw_ssbo_PickingModelTrsBuffer{ nullptr }; // Picking
		std::shared_ptr<ShaderStorageBuffer> m_multiDraw_ssbo_PickingEntityIdsBuffer{ nullptr }; // Picking

		PerspectiveCamera m_pickingCam;
		std::uint8_t m_blitData[texture_dim * texture_dim * 4u]{ 0u };
		Entity m_pickedEntity;
		bool m_shouldRead{ false };

		std::shared_ptr<Shader> m_pickingShader;
		std::shared_ptr<Scene3DNode> m_particlePickingMesh{ nullptr };
		std::shared_ptr<FrameBuffer> m_renderTarget{ nullptr };
		std::shared_ptr<Texture2D> m_renderTexture{ nullptr };
	};
}