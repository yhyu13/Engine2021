#include "engine-precompiled-header.h"
#include "OutlinePass3D.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/common/BaseEngineWidgetManager.h"

// Choose between stencil or polyoffset methods
//#define USE_STENCIL
#define USE_POLYOFFSET

void longmarch::OutlinePass::Init()
{
	m_outlineFrameBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);

	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	m_particlePickingMesh = rm->TryGet("unit_sphere")->Get()->Copy();
}

void longmarch::OutlinePass::BeginRenderPass()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
	{
		// TODO : render stencil based outline rendering
		if (m_outlineFrameBuffer->GetBufferSize() != Renderer3D::s_Data.window_size)
		{
			m_outlineFrameBuffer = FrameBuffer::Create(Renderer3D::s_Data.window_size.x, Renderer3D::s_Data.window_size.y, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
		}

		auto cam_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
		auto camera_entity = m_parentWorld->GetTheOnlyEntityWithType(cam_type);
		auto camera = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera_entity)->GetCamera();

		auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
		auto es = manager->GetAllSelectedEntityBuffered();

		auto render_pipe_original = Renderer3D::s_Data.RENDER_PIPE;
		auto render_mode_original = Renderer3D::s_Data.RENDER_MODE;
		// Config render settings
		Renderer3D::s_Data.RENDER_PASS = Renderer3D::RENDER_PASS::SHADOW; // use shadow render pass to draw to stencil buffer
		Renderer3D::s_Data.RENDER_PIPE = Renderer3D::RENDER_PIPE::FORWARD;
		Renderer3D::s_Data.RENDER_MODE = Renderer3D::RENDER_MODE::CANONICAL;

		RenderCommand::PolyModeFill();
		RenderCommand::Blend(false);
		RenderCommand::DepthTest(true, true);
#ifdef USE_STENCIL
		RenderCommand::StencilTest(true, true);
#endif
		RenderCommand::CullFace(true, false);

		Vec2u traget_resoluation = m_outlineFrameBuffer->GetBufferSize();
		RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
		RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));
		m_outlineFrameBuffer->Bind();
		RenderCommand::Clear();

		// 1. Draw objects to stencil
		// PolyOffset method would draw objects with pulled out depth
#ifdef USE_STENCIL
		RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::ALWAYS);
#endif
#ifdef USE_POLYOFFSET
		(Renderer3D::s_Data.enable_reverse_z) ?
			RenderCommand::PolyOffset(true, 10.f, 1.0f) :
			RenderCommand::PolyOffset(true, -10.f, 1.0f);
#endif

		// Use shadow buffer shader to fill in depth value or to quickly draw to the stencil buffer
		auto shader_name = "ShadowBuffer_Canonical";
		Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
		Renderer3D::s_Data.CurrentShader->Bind();
		Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", (Renderer3D::s_Data.enable_reverse_z) ? camera->GetReverseZViewProjectionMatrix() : camera->GetViewProjectionMatrix());
		Renderer3D::s_Data.CurrentShader->SetInt("enable_ReverseZ", Renderer3D::s_Data.enable_reverse_z);

		m_parentWorld->ForEach(
			es,
			[this, &shader_name, &camera](EntityDecorator e)
		{
			auto scene = e.GetComponent<Scene3DCom>();
			scene->SetShaderName(shader_name);
			scene->SetShouldDraw(true, true);
			scene->Draw();
		}
		);

		// 2. Transfer stencil buffer to final frame buffer
		auto fbo = Renderer3D::s_Data.gpuBuffer.CurrentFinalFrameBuffer;
#ifdef USE_STENCIL
		RenderCommand::TransferStencilBit(
			m_outlineFrameBuffer->GetRendererID(),
			m_outlineFrameBuffer->GetBufferSize().x,
			m_outlineFrameBuffer->GetBufferSize().y,

			fbo->GetRendererID(),
			fbo->GetBufferSize().x,
			fbo->GetBufferSize().y
		);
#endif
#ifdef USE_POLYOFFSET
		RenderCommand::TransferDepthBit(
			m_outlineFrameBuffer->GetFrameBufferID(),
			m_outlineFrameBuffer->GetBufferSize().x,
			m_outlineFrameBuffer->GetBufferSize().y,

			fbo->GetFrameBufferID(),
			fbo->GetBufferSize().x,
			fbo->GetBufferSize().y
		);
#endif

		// 3. Draw slighly larger objects with outline color to default frame buffer
		// PolygonOffset method would draw poly lines
		RenderCommand::DepthTest(true, false);
#ifdef USE_STENCIL
		RenderCommand::StencilTest(true, false);
		RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::NEQUAL);
#endif
#ifdef USE_POLYOFFSET
		(Renderer3D::s_Data.enable_reverse_z) ?
			RenderCommand::DepthFunc(longmarch::RendererAPI::CompareEnum::GEQUAL) :
			RenderCommand::DepthFunc(longmarch::RendererAPI::CompareEnum::LEQUAL);
		RenderCommand::PolyModeLine();
		RenderCommand::PolyOffset(false, 0, 0);
		RenderCommand::PolyLineWidth(2);
#endif

		fbo->Bind();
		traget_resoluation = fbo->GetBufferSize();
		RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

		shader_name = "OutlineShader";
		Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
		Renderer3D::s_Data.CurrentShader->Bind();
		Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", camera->GetReverseZViewProjectionMatrix());

		m_parentWorld->ForEach(
			es,
			[&shader_name, &camera](EntityDecorator e)
		{
#ifdef USE_STENCIL
			// Up scale
			auto trans = e.GetComponent<Transform3DCom>();
			auto orig_scale = trans->GetLocalScale();
			trans->SetLocalScale(orig_scale * 1.05f);
#endif
			auto scene = e.GetComponent<Scene3DCom>();
			scene->SetShaderName(shader_name);
			scene->SetShouldDraw(true, true);
			scene->Draw();
#ifdef USE_STENCIL
			// Reset to original scale
			trans->SetLocalScale(orig_scale);
#endif
		}
		);

#ifdef USE_STENCIL
		RenderCommand::StencilTest(false, false);
#endif
#ifdef USE_POLYOFFSET
		(Renderer3D::s_Data.enable_reverse_z) ?
			RenderCommand::DepthFunc(longmarch::RendererAPI::CompareEnum::GREATER) :
			RenderCommand::DepthFunc(longmarch::RendererAPI::CompareEnum::LESS);
		RenderCommand::PolyModeFill(); 
		RenderCommand::PolyLineWidth(1);
#endif
		Renderer3D::s_Data.RENDER_PIPE = render_pipe_original;
		Renderer3D::s_Data.RENDER_MODE = render_mode_original;
	}
	break;
	}
}

void longmarch::OutlinePass::EndRenderPass()
{
}
