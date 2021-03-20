#include "engine-precompiled-header.h"
#include "OutlinePass3D.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/BaseEngineWidgetManager.h"

void longmarch::OutlinePass::Init()
{
	OutlineFrameBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::Float16);
}

void longmarch::OutlinePass::BeginRenderPass()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
	{
		// TODO : render stencil based outline rendering
		if (OutlineFrameBuffer->GetBufferSize() != Renderer3D::s_Data.window_size)
		{
			OutlineFrameBuffer = FrameBuffer::Create(Renderer3D::s_Data.window_size.x, Renderer3D::s_Data.window_size.y, FrameBuffer::BUFFER_FORMAT::Float16);
		}

		auto cam_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
		auto camera = m_parentWorld->GetTheOnlyEntityWithType(cam_type);
		auto cam = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

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
		RenderCommand::StencilTest(true, true);
		RenderCommand::CullFace(true, false);

		Vec2u traget_resoluation = OutlineFrameBuffer->GetBufferSize();
		RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
		RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));
		OutlineFrameBuffer->Bind();
		RenderCommand::Clear();

		// 1. Draw objects to stencil
		RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::ALWAYS);

		auto shader_name = "ShadowBuffer";
		Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
		Renderer3D::s_Data.CurrentShader->Bind();
		Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", cam->GetReverseZViewProjectionMatrix());
		m_parentWorld->ForEach(
			es,
			[&shader_name, &cam](EntityDecorator e)
		{
			auto particle = e.GetComponent<Particle3DCom>();
			bool isParticle = particle.Valid();

			auto scene = e.GetComponent<Scene3DCom>();
			scene->SetShaderName(shader_name);
			scene->SetShouldDraw(true, true);

			if (!scene->IsHideInGame() && scene->IsCastShadow())
			{
				if (isParticle)
				{
					// TODO : outline particle, need to write a specialized shader
					/*particle->SetRendering(scene->GetShouldDraw());
					particle->PrepareDrawWithViewMatrix(cam->GetViewMatrix());
					scene->Draw(particle.GetPtr());*/
				}
				else
				{
					scene->Draw();
				}
			}
		}
		);

		// 2. Transfer stencil buffer to default frame buffer
		constexpr int default_framebuffer_rendererID = 0;
		RenderCommand::TransferStencilBit(
			OutlineFrameBuffer->GetRendererID(),
			OutlineFrameBuffer->GetBufferSize().x,
			OutlineFrameBuffer->GetBufferSize().y,

			default_framebuffer_rendererID,
			Renderer3D::s_Data.window_size.x,
			Renderer3D::s_Data.window_size.y
		);

		// 3. Draw slighly larger objects with outline color to default frame buffer
		RenderCommand::DepthTest(false, false);
		RenderCommand::StencilTest(true, false);
		RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::NEQUAL);

		RenderCommand::BindDefaultFrameBuffer();
		traget_resoluation = Renderer3D::s_Data.window_size;
		RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

		shader_name = "OutlineShader";
		Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
		Renderer3D::s_Data.CurrentShader->Bind();
		Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", cam->GetReverseZViewProjectionMatrix());
		m_parentWorld->ForEach(
			es,
			[&shader_name, &cam](EntityDecorator e)
		{
			// Up scale
			auto trans = e.GetComponent<Transform3DCom>();
			auto orig_scale = trans->GetLocalScale();
			trans->SetLocalScale(orig_scale * 1.05f);

			auto particle = e.GetComponent<Particle3DCom>();
			bool isParticle = particle.Valid();

			auto scene = e.GetComponent<Scene3DCom>();
			scene->SetShaderName(shader_name);
			scene->SetShouldDraw(true, true);

			if (!scene->IsHideInGame() && scene->IsCastShadow())
			{
				if (isParticle)
				{
					// TODO : outline particle, need to write a specialized shader
					/*particle->SetRendering(scene->GetShouldDraw());
					particle->PrepareDrawWithViewMatrix(cam->GetViewMatrix());
					scene->Draw(particle.GetPtr());*/
				}
				else
				{
					scene->Draw();
				}
			}

			// Reset to original scale
			trans->SetLocalScale(orig_scale);
		}
		);

		RenderCommand::StencilTest(false, false);
		Renderer3D::s_Data.RENDER_PIPE = render_pipe_original;
		Renderer3D::s_Data.RENDER_MODE = render_mode_original;
	}
	break;
	}
}

void longmarch::OutlinePass::EndRenderPass()
{
}
