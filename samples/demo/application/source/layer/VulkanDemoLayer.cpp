#include "application-precompiled-header.h"
#include "VulkanDemoLayer.h"
#include "engine/audio/AudioManager.h"
#include "engine/ecs/header/header.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include "events/EventType.h"
#include "ecs/header/header.h"
#include "ui/header.h"

longmarch::VulkanDemoLayer::VulkanDemoLayer()
	: Layer("VulkanDemoLayer")
{
}

void longmarch::VulkanDemoLayer::Init()
{
	APP_TIME("App Initialization");
	BuildTestScene();
	BuildRenderPipeline();
	{
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		float base_font_size = 20.0f;
		auto path = FileSystem::ResolveProtocol("$asset:ImGui/ComicNeue-Bold.ttf").string();
		m_Data.m_font_1k = io.Fonts->AddFontFromFileTTF(path.c_str(), base_font_size);
		m_Data.m_font_2k = io.Fonts->AddFontFromFileTTF(path.c_str(), 1.414f * base_font_size);
		m_Data.m_font_4k = io.Fonts->AddFontFromFileTTF(path.c_str(), 2.0f * base_font_size);
		ImGui::UploadFontTexture();
	}
}

void longmarch::VulkanDemoLayer::BuildTestScene()
{
}

void longmarch::VulkanDemoLayer::OnUpdate(double ts)
{
	double dt = GameWorld::GetCurrent()->IsPaused() ? 0.0 : ts;
	{
		{
			APP_TIME("Pre System update");
			PreUpdate(dt);
		}
		{
			APP_TIME("PreRender update");
			PreRenderUpdate(dt);
		}
		{
			APP_TIME("System update");
			Update(dt);
		}
		{
			APP_TIME("Render");
			Render(dt);
		}
		{
			JoinAll();
		}
		{
			APP_TIME("PostRender update");
			PostRenderUpdate(dt);
		}
	}
}

void longmarch::VulkanDemoLayer::OnAttach()
{
	PRINT("VulkanDemoLayer attached!");
}

void longmarch::VulkanDemoLayer::OnDetach()
{
	PRINT("VulkanDemoLayer detached!");
}

void longmarch::VulkanDemoLayer::OnImGuiRender()
{
	APP_TIME("ImGUI Render");
	ImFont* font;
	{
		if (auto height = Engine::GetWindow()->GetHeight(); height > 2000)
		{
			font = m_Data.m_font_4k;
		}
		else if (height > 1400)
		{
			font = m_Data.m_font_2k;
		}
		else
		{
			font = m_Data.m_font_1k;
		}
	}
	ImGui::PushFont(font);

	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	ImGui::ShowDemoWindow(&show_demo_window);
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	GameWorld::GetCurrent()->RenderUI();
	ImGui::PopFont();
}

void longmarch::VulkanDemoLayer::PreUpdate(double ts)
{
	{
		auto queue = EventQueue<GameEventType>::GetInstance();
		queue->Update(ts);
	}
	{
		auto queue = EventQueue<GameDebugEventType>::GetInstance();
		queue->Update(ts);
	}
	{
		auto queue = EventQueue<GameSettingEventType>::GetInstance();
		queue->Update(ts);
	}
}

void longmarch::VulkanDemoLayer::Update(double ts)
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadUpdate(ts);
#else
	GameWorld::GetCurrent()->Update(ts);
#endif // MULTITHREAD_UPDATE
}

void longmarch::VulkanDemoLayer::JoinAll()
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadJoin();
#endif // MULTITHREAD_UPDATE
}

void longmarch::VulkanDemoLayer::PreRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PreRenderUpdate(ts);
}

void longmarch::VulkanDemoLayer::Render(double ts)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
	{
		GPU_TIME(Total_Render);
		GameWorld::GetCurrent()->PreRenderPass(ts);
		m_Data.mainRenderPipeline(ts);
		GameWorld::GetCurrent()->PostRenderPass(ts);
		Renderer3D::SubmitFrameBufferToScreen();
	}
	break;
	default:
		return;
	}
}

void longmarch::VulkanDemoLayer::PostRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PostRenderUpdate(ts);
}

void longmarch::VulkanDemoLayer::BuildRenderPipeline()
{
	Renderer3D::BuildAllMesh();
	Renderer3D::BuildAllMaterial();
	Renderer3D::BuildAllTexture();

	m_Data.mainRenderPipeline = [this](double ts)
	{
		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::INGAME:
			e_type = (EntityType)EngineEntityType::PLAYER_CAMERA;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		// TODO move render pipeline for INGame to application's layer, and use application layer in application mode
		auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
		auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::INGAME:
		{
			const auto& prop = Engine::GetWindow()->GetWindowProperties();
			cam->SetViewPort(Vec2u(0), Vec2u(prop.m_width, prop.m_height));
			if (prop.IsResizable)
			{
				cam->cameraSettings.aspectRatioWbyH = float(prop.m_width) / float(prop.m_height);
			}
		}
		break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		if (Renderer3D::ShouldRendering())
		{
			// callbacks for scene rendering
			auto scene3DComSys = static_cast<Scene3DComSys*>(GameWorld::GetCurrent()->GetComponentSystem("Scene3DComSys"));
			std::function<void()> f_render_opaque = std::bind(&Scene3DComSys::RenderOpaqueObj, scene3DComSys);
			std::function<void()> f_render_translucent = std::bind(&Scene3DComSys::RenderTransparentObj, scene3DComSys);
			std::function<void(bool, const ViewFrustum&, const Mat4&)> f_setVFCullingParam = std::bind(&Scene3DComSys::SetVFCullingParam, scene3DComSys, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			std::function<void(bool, const Vec3f&, float, float)> f_setDistanceCullingParam = std::bind(&Scene3DComSys::SetDistanceCullingParam, scene3DComSys, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			std::function<void(const std::string&)> f_setRenderShaderName = std::bind(&Scene3DComSys::SetRenderShaderName, scene3DComSys, std::placeholders::_1);

			{
				Renderer3D::BeginRendering(cam);
				{
					GPU_TIME(Shadow_Pass);
					APP_TIME("Shadow pass");
					Renderer3D::BeginShadowing(cam, f_render_opaque, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndShadowing();
				}
				{
					GPU_TIME(Opaque_Scene_pass);
					APP_TIME("Opaque Scene pass");
					Renderer3D::BeginOpaqueScene(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueScene();
				}
				{
					GPU_TIME(Opaque_Lighting_pass);
					APP_TIME("Opaque Lighting pass");
					Renderer3D::BeginOpaqueLighting(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueLighting();
				}
				{
					GPU_TIME(Transparent_Scene_pass);
					APP_TIME("Transparent Scene pass");
					Renderer3D::BeginTransparentSceneAndLighting(cam, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndTransparentSceneAndLighting();
				}
				{
					GPU_TIME(Postprocessing_pass);
					APP_TIME("Postprocessing pass");
					Renderer3D::BeginPostProcessing();
					Renderer3D::EndPostProcessing();
				}
				Renderer3D::EndRendering();
			}
		}
	};
}
