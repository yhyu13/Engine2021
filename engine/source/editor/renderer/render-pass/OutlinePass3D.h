#pragma once
#include "engine/renderer/render-pass/RenderPass3D.h"

namespace longmarch
{
	/**
	 * @brief Picking system render pass
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class OutlinePass final : public RenderPass3D
	{
	public:
		NONCOPYABLE(OutlinePass);
		OutlinePass() = default;

		virtual void Init() override;
		virtual void BeginRenderPass() override;
		virtual void EndRenderPass() override;

	private:
		std::shared_ptr<FrameBuffer> m_outlineFrameBuffer{ nullptr };
		std::shared_ptr<Scene3DNode> m_particlePickingMesh{ nullptr };
	};
}