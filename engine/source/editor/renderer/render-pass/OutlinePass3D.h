#pragma once
#include "engine/renderer/render-pass/RenderPass3D.h"

namespace longmarch
{
	/**
	 * @brief Picking system render pass
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
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
		std::shared_ptr<FrameBuffer> OutlineFrameBuffer;
	};
}