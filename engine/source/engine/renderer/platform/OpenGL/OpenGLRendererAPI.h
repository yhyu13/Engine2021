#pragma once

#include "../../RendererAPI.h"
#include "OpenGLUtil.h"

namespace longmarch {
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		NONCOPYABLE(OpenGLRendererAPI);
		OpenGLRendererAPI() = default;
		static RendererAPI* GetInstance()
		{
			static OpenGLRendererAPI instance;
			return static_cast<RendererAPI*>(&instance);
		}

		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void ClearColorOnly() override;
		virtual void ClearDepthOnly() override;
		virtual void Clear() override;

		virtual unsigned int CreateAndBindFBO() override;
		virtual void DestoryAndUnBindFBO(unsigned int fbo) override;

		virtual void DrawTriangleIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
		virtual void DrawLineIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
		virtual void DrawTriangleIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count) override;
		virtual void DrawLineIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count) override;
		virtual void MultiDrawTriangleIndexedIndirect(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexedIndirectCommandBuffer>& commandBuffer) override;

		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) override;

		virtual void PolyModeFill() override;
		virtual void PolyModeLine() override;
		virtual void PolyOffset(bool enabled, float factor, float units);

		virtual void BindDefaultFrameBuffer() override;;

		virtual void TransferColorBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) override;
		virtual void TransferColorBit(uint32_t src, uint32_t src_tex, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_tex, uint32_t dest_w, uint32_t dest_h) override;
		virtual void TransferDepthBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) override;
		virtual void TransferStencilBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) override;

		virtual void Reverse_Z(bool b) override;

		virtual void DepthTest(bool enabled, bool write) override;
		virtual void DepthFunc(longmarch::RendererAPI::CompareEnum e) override;
		virtual void DepthClamp(bool enabled) override;

		virtual void StencilTest(bool enabled, bool write) override;
		virtual void StencilFunc(longmarch::RendererAPI::CompareEnum e) override;

		virtual void CullFace(bool enabled, bool front) override;
		virtual void Blend(bool enabled) override;
		virtual void BlendFunc(longmarch::RendererAPI::BlendFuncEnum e) override;

	private:
		inline static PolyMode s_polyMode = PolyMode::NONE;
	};
}