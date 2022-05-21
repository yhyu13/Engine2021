#include "engine-precompiled-header.h"
#include "VulkanRendererAPI.h"

void longmarch::VulkanRendererAPI::Init()
{
}

void longmarch::VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
}

void longmarch::VulkanRendererAPI::SetClearColor(const glm::vec4& color)
{
}

void longmarch::VulkanRendererAPI::ClearColorOnly()
{
}

void longmarch::VulkanRendererAPI::ClearDepthOnly()
{
}

void longmarch::VulkanRendererAPI::Clear()
{
}

void longmarch::VulkanRendererAPI::DrawTriangleIndexed(const std::shared_ptr<VertexArray>& vertexArray)
{
}

void longmarch::VulkanRendererAPI::DrawLineIndexed(const std::shared_ptr<VertexArray>& vertexArray)
{
}

void longmarch::VulkanRendererAPI::DrawTriangleIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
{
}

void longmarch::VulkanRendererAPI::DrawLineIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
{
}

void longmarch::VulkanRendererAPI::MultiDrawTriangleIndexedIndirect(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexedIndirectCommandBuffer>& commandBuffer)
{
}

void longmarch::VulkanRendererAPI::DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
}

void longmarch::VulkanRendererAPI::PlaceMemoryBarrier(MemoryBarrierBitEnum e)
{
}

void longmarch::VulkanRendererAPI::PolyModeFill()
{
}

void longmarch::VulkanRendererAPI::PolyModeLine()
{
}

void longmarch::VulkanRendererAPI::PolyLineWidth(uint32_t width)
{
}

void longmarch::VulkanRendererAPI::PolyOffset(bool enabled, float factor, float units)
{
}

void longmarch::VulkanRendererAPI::BindDefaultFrameBuffer()
{
}

void longmarch::VulkanRendererAPI::TransferColorBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
{
}

void longmarch::VulkanRendererAPI::TransferColorBit(uint32_t src, uint32_t src_tex, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_tex, uint32_t dest_w, uint32_t dest_h)
{
}

void longmarch::VulkanRendererAPI::TransferDepthBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
{
}

void longmarch::VulkanRendererAPI::TransferStencilBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
{
}

void longmarch::VulkanRendererAPI::Reverse_Z(bool b)
{
}

void longmarch::VulkanRendererAPI::DepthTest(bool enabled, bool write)
{
}

void longmarch::VulkanRendererAPI::DepthFunc(longmarch::RendererAPI::CompareEnum e)
{
}

void longmarch::VulkanRendererAPI::DepthClamp(bool enabled)
{
}

void longmarch::VulkanRendererAPI::StencilTest(bool enabled, bool write)
{
}

void longmarch::VulkanRendererAPI::StencilFunc(longmarch::RendererAPI::CompareEnum e)
{
}

void longmarch::VulkanRendererAPI::CullFace(bool enabled, bool front)
{
}

void longmarch::VulkanRendererAPI::Blend(bool enabled)
{
}

void longmarch::VulkanRendererAPI::BlendFunc(longmarch::RendererAPI::BlendFuncEnum e)
{
}
