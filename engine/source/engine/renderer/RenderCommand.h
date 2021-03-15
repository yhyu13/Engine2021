#pragma once

#include "RendererAPI.h"
#include "engine/core/EngineCore.h"

namespace longmarch
{
	class ENGINE_API RenderCommand
	{
	public:
		NONINSTANTIABLE(RenderCommand);

		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void ClearColorOnly()
		{
			s_RendererAPI->ClearColorOnly();
		}

		inline static void ClearDepthOnly()
		{
			s_RendererAPI->ClearDepthOnly();
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static unsigned int CreateAndBindFBO()
		{
			return s_RendererAPI->CreateAndBindFBO();
		}

		inline static void DestoryAndUnBindFBO(unsigned int fbo)
		{
			s_RendererAPI->DestoryAndUnBindFBO(fbo);
		}

		inline static void DrawTriangleIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawTriangleIndexed(vertexArray);
		}

		inline static void DrawLineIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawLineIndexed(vertexArray);
		}

		inline static void DrawTriangleIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
		{
			s_RendererAPI->DrawTriangleIndexedInstanced(vertexArray, count);
		}

		inline static void DrawLineIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
		{
			s_RendererAPI->DrawLineIndexedInstanced(vertexArray, count);
		}

		inline static void MultiDrawTriangleIndexedIndirect(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexedIndirectCommandBuffer>& commandBuffer)
		{
			s_RendererAPI->MultiDrawTriangleIndexedIndirect(vertexArray, commandBuffer);
		}

		inline static void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
		{
			s_RendererAPI->DispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		}

		inline static void PolyModeFill()
		{
			s_RendererAPI->PolyModeFill();
		}

		inline static void PolyModeLine()
		{
			s_RendererAPI->PolyModeLine();
		}

		inline static void PolyOffset(bool enabled, float factor, float units)
		{
			s_RendererAPI->PolyOffset(enabled, factor, units);
		}

		inline static void BindDefaultFrameBuffer()
		{
			s_RendererAPI->BindDefaultFrameBuffer();
		}

		inline static void TransferAllBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
		{
			s_RendererAPI->TransferColorBit(src, src_w, src_h, dest, dest_w, dest_h);
			s_RendererAPI->TransferDepthBit(src, src_w, src_h, dest, dest_w, dest_h);
			s_RendererAPI->TransferStencilBit(src, src_w, src_h, dest, dest_w, dest_h);
		}

		//! Linear blend color bit
		inline static void TransferColorBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
		{
			s_RendererAPI->TransferColorBit(src, src_w, src_h, dest, dest_w, dest_h);
		}

		//! Linear blend color bit
		inline static void TransferColorBit(uint32_t src, uint32_t src_tex, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_tex, uint32_t dest_w, uint32_t dest_h)
		{
			s_RendererAPI->TransferColorBit(src, src_tex, src_w, src_h, dest, dest_tex, dest_w, dest_h);
		}

		//! Nearest blend depth bit
		inline static void TransferDepthBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
		{
			s_RendererAPI->TransferDepthBit(src, src_w, src_h, dest, dest_w, dest_h);
		}

		//! Nearest blend stencil bit
		inline static void TransferStencilBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
		{
			s_RendererAPI->TransferStencilBit(src, src_w, src_h, dest, dest_w, dest_h);
		}

		inline static void Reverse_Z(bool b)
		{
			s_RendererAPI->Reverse_Z(b);
		}

		inline static void DepthTest(bool test, bool write)
		{
			s_RendererAPI->DepthTest(test, write);
		}

		inline static void DepthFunc(longmarch::RendererAPI::CompareEnum e)
		{
			s_RendererAPI->DepthFunc(e);
		}

		inline static void DepthClamp(bool enabled)
		{
			s_RendererAPI->DepthClamp(enabled);
		}

		inline static void StencilTest(bool test, bool write)
		{
			s_RendererAPI->StencilTest(test, write);
		}

		inline static void StencilFunc(longmarch::RendererAPI::CompareEnum e)
		{
			s_RendererAPI->StencilFunc(e);
		}

		inline static void CullFace(bool enabled, bool front)
		{
			s_RendererAPI->CullFace(enabled, front);
		}

		inline static void Blend(bool enabled)
		{
			s_RendererAPI->Blend(enabled);
		}

		inline static void BlendFunc(longmarch::RendererAPI::BlendFuncEnum e)
		{
			s_RendererAPI->BlendFunc(e);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}