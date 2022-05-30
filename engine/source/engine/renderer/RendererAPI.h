#pragma once

#include "VertexArray.h"
#include "engine/core/EngineCore.h"
#include "engine/math/Geommath.h"

namespace longmarch 
{
	class ENGINE_API RendererAPI
	{
	public:
		NONCOPYABLE(RendererAPI);
		RendererAPI() = default;

		enum class API : uint8_t
		{
			None = 0, 
			OpenGL = 1,
			Vulkan = 2
		};

		enum class PolyMode : uint8_t
		{
			NONE = 0, 
			FILL,
			LINE
		};

		enum class CompareEnum : uint8_t
		{
			LESS = 0,
			LEQUAL,
			EQUAL,
			NEQUAL,
			GEQUAL,
			GREATER,
			ALWAYS,
			NEVER
		};

		enum class DataTypeEnum : uint8_t
		{
			UNSIGNED_INT = 0,
			UNSIGNED_SHORT,
		};

		enum class BlendFuncEnum : uint8_t
		{
			ADDITION = 0,  //!< The blending function that simply adds two colors.
			MULTIPLICATION,  //!< The blending function that multiplies source and destination color and alpha.
			ALPHA_BLEND_1, //!< The transparency blending function by alpha, which also sets the result alpha value to the same as the source(aka. new color)'s alpha
			ALPHA_BLEND_2  //!< The transparency blending function by alpha, which also blends alpha values by alpha blending (i.e. result.a = src.a + (1-src.a) * dest.a)
		};

		enum class MemoryBarrierBitEnum : uint8_t
		{
			SHADER_STORAGE_BUFFER_BARRIER = 0,
			SHADER_IMAGE_ACCESS_BARRIER,
			SHADER_TEXTURE_FETCH_BARRIER,
		};

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void ClearColorOnly() = 0;
		virtual void ClearDepthOnly() = 0;
		virtual void Clear() = 0;

		virtual void DrawTriangleIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;
		virtual void DrawLineIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;
		virtual void DrawTriangleIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count) = 0;
		virtual void DrawLineIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count) = 0;
		virtual void MultiDrawTriangleIndexedIndirect(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexedIndirectCommandBuffer>& commandBuffer) = 0;
		
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) = 0;
		virtual void PlaceMemoryBarrier(MemoryBarrierBitEnum e) = 0;

		virtual void PolyModeFill() = 0;
		virtual void PolyModeLine() = 0;
		virtual void PolyLineWidth(uint32_t width) = 0;
		virtual void PolyOffset(bool enabled, float factor, float units) = 0;

		virtual void BindDefaultFrameBuffer() = 0;

		virtual void TransferColorBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) = 0;
		virtual void TransferColorBit(uint32_t src, uint32_t src_tex, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_tex, uint32_t dest_w, uint32_t dest_h) = 0;
		virtual void TransferDepthBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) = 0;
		virtual void TransferStencilBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h) = 0;

		virtual void Reverse_Z(bool b) = 0;

		virtual void DepthTest(bool enabled, bool write) = 0;
		virtual void DepthFunc(longmarch::RendererAPI::CompareEnum e) = 0;
		virtual void DepthClamp(bool enabled) = 0;

		virtual void StencilTest(bool enabled, bool write) = 0;
		virtual void StencilFunc(longmarch::RendererAPI::CompareEnum e) = 0;

		virtual void CullFace(bool enabled, bool front) = 0;
		virtual void Blend(bool enabled) = 0;
		virtual void BlendFunc(longmarch::RendererAPI::BlendFuncEnum e) = 0;

		virtual void PushDebugMarker(const char* msg) = 0;
		virtual void PopDebugMarker() = 0;

		inline static API WhichAPI() { return s_eAPI; }

	protected:
		inline static API s_eAPI { API::None };
	};
}
