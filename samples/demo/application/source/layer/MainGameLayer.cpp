#include "application-precompiled-header.h"
#include "MainGameLayer.h"
#include "engine/audio/AudioManager.h"
#include "engine/ecs/header/header.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include "events/EventType.h"
#include "ecs/header/header.h"
#include "ui/header.h"

// Specifically AIControllerCom
#include "ai/AIDemoShowCase.h"

longmarch::MainGameLayer::MainGameLayer()
	: Layer("MainGameLayer")
{
	Renderer3D::Init();
}

void longmarch::MainGameLayer::Init()
{
	APP_TIME("App Initialization");
	InitFramework();
	InitGameWorld();
	BuildTestScene();
	BuildRenderPipeline();
	{
		// Register _3DEngineWidgetManager
		ServiceLocator::ProvideSingleton<_3DGameWidgetManager>(APP_WIG_MAN_NAME, MemoryManager::Make_shared<_3DGameWidgetManager>());
	}
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
	{
		auto queue = EventQueue<EngineIOEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::LOAD_SCENE_BEGIN, &MainGameLayer::_ON_LOAD_SCENE_BEGIN));
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::LOAD_SCENE, &MainGameLayer::_ON_LOAD_SCENE));
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::LOAD_SCENE_END, &MainGameLayer::_ON_LOAD_SCENE_END));

		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::SAVE_SCENE_BEGIN, &MainGameLayer::_ON_SAVE_SCENE_BEGIN));
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::SAVE_SCENE, &MainGameLayer::_ON_SAVE_SCENE));
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineIOEventType::SAVE_SCENE_END, &MainGameLayer::_ON_SAVE_SCENE_END));
	}
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<MainGameLayer>(this, EngineEventType::ENG_WINDOW_INTERRUTPTION, &MainGameLayer::_ON_WINDOW_INTERRUPT));
	}
}

void TEST_DS_Lights();
void TEST_BRDF_Materials();
void TEST_Skeletal_Animations();
void longmarch::MainGameLayer::BuildTestScene()
{
	TEST_DS_Lights();
	TEST_BRDF_Materials();
	TEST_Skeletal_Animations();
}

/*
Load engine frameworks
*/

void longmarch::MainGameLayer::InitFramework()
{
	ServiceLocator::Register<MainObjectFactory>("ObjectFactory");
	{
		APP_TIME("Loading resources");
		LoadResources();
	}
}

/*
Load all game resources
*/

void longmarch::MainGameLayer::LoadResources()
{
	auto objectFactory = ServiceLocator::GetSingleton<ObjectFactory>("ObjectFactory");
	objectFactory->LoadResources(FileSystem::ResolveProtocol("$asset:archetype/resource.json"), nullptr);
}

/*
Load start up screen
*/

void longmarch::MainGameLayer::InitGameWorld()
{
	// 2, Init game world.
	// This step must happen after EventQueue clear as systems in mainGameWorld->Init() would register for event handler at that stage)

	auto filepath = FileSystem::ResolveProtocol("$asset:archetype/scene-game.json");

	GameWorld::GetInstance(true, "", filepath);

	//AudioManager::GetInstance()->PlaySoundByName("bgm0", AudioVector3{ 0,0,0 }, -10, 1);
}

void longmarch::MainGameLayer::OnUpdate(double ts)
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

void longmarch::MainGameLayer::OnAttach()
{
	PRINT("MainGameLayer attached!");
}

void longmarch::MainGameLayer::OnDetach()
{
	PRINT("MainGameLayer detached!");
}

void longmarch::MainGameLayer::OnImGuiRender()
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
	auto manager = ServiceLocator::GetSingleton<BaseGameWidgetManager>(APP_WIG_MAN_NAME);
	manager->BeginFrame();
	manager->RenderUI();
	manager->EndFrame();
	GameWorld::GetCurrent()->RenderUI();
	ImGui::PopFont();
}

void longmarch::MainGameLayer::PreUpdate(double ts)
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

void longmarch::MainGameLayer::Update(double ts)
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadUpdate(ts);
#else
	GameWorld::GetCurrent()->Update(ts);
	GameWorld::GetCurrent()->Update2(ts);
	GameWorld::GetCurrent()->Update3(ts);
#endif // MULTITHREAD_UPDATE
}

void longmarch::MainGameLayer::JoinAll()
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadJoin();
#endif // MULTITHREAD_UPDATE
}

void longmarch::MainGameLayer::PreRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PreRenderUpdate(ts);
}

void longmarch::MainGameLayer::Render(double ts)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
	{
		GPU_TIME(Total_Render);
		GameWorld::GetCurrent()->Render(ts);
		m_Data.mainRenderPipeline(ts);
		GameWorld::GetCurrent()->Render2(ts);
		Renderer3D::SubmitFrameBufferToScreen();
	}
		break;
	default:
		return;
	}
}

void longmarch::MainGameLayer::PostRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PostRenderUpdate(ts);
}

void longmarch::MainGameLayer::BuildRenderPipeline()
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

void longmarch::MainGameLayer::_ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::MainGameLayer::_ON_LOAD_SCENE(EventQueue<EngineIOEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineLoadSceneEvent>(e);
	auto& filepath = event->m_filepath;
	// Generate new game world instance, which frees the old world // TODO delayed remove old world
	GameWorld::GetInstance(event->m_makeCurrent, "", filepath);
}

void longmarch::MainGameLayer::_ON_LOAD_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e)
{
	Renderer3D::BuildAllMesh();
	Renderer3D::BuildAllMaterial();
	Renderer3D::BuildAllTexture();
	//AudioManager::GetInstance()->PlaySoundByName("bgm0", AudioVector3{ 0,0,0 }, -10, 1);
}

void longmarch::MainGameLayer::_ON_SAVE_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::MainGameLayer::_ON_SAVE_SCENE(EventQueue<EngineIOEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineSaveSceneEvent>(e);
	auto& filepath = event->m_filepath;
	auto world = static_cast<GameWorld*>(event->m_gameworld);
	auto objectFactory = ServiceLocator::GetSingleton<ObjectFactory>("ObjectFactory");
	objectFactory->SaveGameWorldScene(filepath, world);
}

void longmarch::MainGameLayer::_ON_SAVE_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::MainGameLayer::_ON_WINDOW_INTERRUPT(EventQueue<EngineEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineWindowInterruptionEvent>(e);
	GameWorld::GetCurrent()->SetPause(!event->m_isFocused && Engine::GetPaused());
}

void TEST_BRDF_Materials()
{
	auto& engineConfiguration = FileSystem::GetCachedJsonCPP(FileSystem::ResolveProtocol("$root:engine-config.json"));
	uint32_t num_materials = engineConfiguration["test"]["Test-material-number"].asUInt();
	if (num_materials == 0)
	{
		return;
	}

	auto world = GameWorld::GetCurrent();
	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	auto sphere_mesh = rm->TryGet("sphere")->Get()->Copy();

	int slice = glm::roundEven(powf(num_materials, 1.0f / 3.0f)) / 2;
	auto rotation_root_1 = world->GenerateEntity3DNoCollision((EntityType)(EngineEntityType::DYNAMIC_OBJ), true, true);
	{
		// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
		auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
		trans_root->SetGlobalPos(Vec3f(0, 0, 10));
		auto scene = rotation_root_1.GetComponent<Scene3DCom>();
		scene->SetVisiable(false);
	}

	for (int x(-slice); x <= slice; ++x)
	{
		for (int y(-slice); y <= slice; ++y)
		{
			for (int z(-slice); z <= slice; ++z)
			{
				auto entity = world->GenerateEntity3D((EntityType)(EngineEntityType::DYNAMIC_OBJ), true, false);
				{
					// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
					auto trans = entity.GetComponent<Transform3DCom>();
					auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
					trans->SetParentModelTr(trans_root->GetSuccessionModelTr(trans));
					trans->SetRelativeToParentPos((10.0f * (slice / 2.0f)) * Vec3f(x, y, z));
					trans->SetLocalScale(Vec3f(0.1));
					auto scene = entity.GetComponent<Scene3DCom>();
					scene->SetVisiable(true);
					sphere_mesh->ModifyAllMaterial([&](Material* material)
					{
						material->Kd = Vec3f(0.2);
						material->roughness = 1.0f * glm::clamp(float(x + slice) / float(2 * slice), 0.01f, .99f);
						material->metallic = 1.0f * glm::clamp(float(z + slice) / float(2 * slice), 0.01f, .99f);
						material->emissive = false;
					});
					scene->SetSceneData(sphere_mesh);

					auto root_childrenCom_1 = world->GetComponent<ChildrenCom>(rotation_root_1);
					root_childrenCom_1->AddEntity(entity);
				}
			}
		}
	}
}

void TEST_DS_Lights()
{
	auto& engineConfiguration = FileSystem::GetCachedJsonCPP(FileSystem::ResolveProtocol("$root:engine-config.json"));
	uint32_t num_lights = engineConfiguration["test"]["Test-light-number"].asUInt();
	if (num_lights == 0)
	{
		return;
	}

	auto world = GameWorld::GetCurrent();
	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	auto sphere_mesh = rm->TryGet("sphere")->Get()->Copy();

	uint32_t slice = MAX(num_lights / 32, 1);
	for (uint32_t i(0); i < slice; ++i)
	{
		uint32_t lights = MIN(32, num_lights);
		float z = RAND_F(-10, 30);
		float rotation_radius = RAND_F(5, 30);

		auto rotation_root_1 = world->GenerateEntity3DNoCollision((EntityType)(EngineEntityType::DYNAMIC_OBJ), true, true);
		{
			// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
			auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
			trans_root->SetGlobalPos(Vec3f(0, 0, z));
			trans_root->SetGlobalRotVel((DEG2RAD * (Vec3f(0, 0, RAND_F(-360, 360)))));
			auto scene = rotation_root_1.GetComponent<Scene3DCom>();
			scene->SetVisiable(false);
		}

		for (uint32_t i(0); i < lights; ++i)
		{
			float angle = (float)i / lights * PI2;
			float radius = RAND_F(2, 10);

			auto entity = world->GenerateEntity3DNoCollision((EntityType)(EngineEntityType::POINT_LIGHT), true, false);
			{
				// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
				LightCom lightCom;
				lightCom.shadow.bCastShadow = false;
				lightCom.pointLight.radius = radius;
				entity.AddComponent(lightCom);

				auto trans = entity.GetComponent<Transform3DCom>();
				auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
				trans->SetParentModelTr(trans_root->GetSuccessionModelTr(trans));
				trans->SetRelativeToParentPos(Vec3f(cosf(angle) * rotation_radius, sinf(angle) * rotation_radius, 0));
				trans->SetLocalScale(Vec3f(0.025f)); // Set scale of mesh
				auto scene = entity.GetComponent<Scene3DCom>();
				scene->SetVisiable(true);
				auto color = Vec3f(RAND_F(0.1, 2));
				color[i % 3] = RAND_F(1, 5);
				sphere_mesh->ModifyAllMaterial([&](Material* material)
				{
					material->Kd = color;
					material->emissive = true;
				});
				scene->SetSceneData(sphere_mesh);

				auto root_childrenCom_1 = world->GetComponent<ChildrenCom>(rotation_root_1);
				root_childrenCom_1->AddEntity(entity);
			}
		}
	}
}

void TEST_Skeletal_Animations()
{
	auto& engineConfiguration = FileSystem::GetCachedJsonCPP(FileSystem::ResolveProtocol("$root:engine-config.json"));
	uint32_t num_animation = engineConfiguration["test"]["Test-skeletal-animation-number"].asUInt();
	if (num_animation == 0)
	{
		return;
	}

	auto world = GameWorld::GetCurrent();
	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	auto locomotion_scene_node = rm->TryGet("Locomotion_prefab")->Get()->Copy();

	int slice = glm::roundEven(powf(num_animation, 1.0f / 3.0f)) / 2;
	auto rotation_root_1 = world->GenerateEntity3DNoCollision((EntityType)(EngineEntityType::DYNAMIC_OBJ), true, true);
	{
		// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
		auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
		trans_root->SetGlobalPos(Vec3f(0, 0, 10));
		auto scene = rotation_root_1.GetComponent<Scene3DCom>();
		scene->SetVisiable(false);
	}

	for (int x(-slice); x <= slice; ++x)
	{
		for (int y(-slice); y <= slice; ++y)
		{
			for (int z(-slice); z <= slice; ++z)
			{
				auto entity = world->GenerateEntity3D((EntityType)(GameEntityType::NPC), true, false);
				{
					// WARNING : all component decorator and component pointer should be LOCAL because their address are subjected to change if reallocation is triggered
					auto trans = entity.GetComponent<Transform3DCom>();
					auto trans_root = rotation_root_1.GetComponent<Transform3DCom>();
					trans->SetParentModelTr(trans_root->GetSuccessionModelTr(trans));
					trans->SetRelativeToParentPos((3.0f * (slice / 2.0f)) * Vec3f(x, y, z));
					trans->SetRelativeToParentRot(Geommath::RotationMat(Geommath::ROT_AXIS::X, PI * 0.5f));
					trans->SetRelativeToParentScale(Vec3f(0.02));
					trans->SetLocalScale(Vec3f(0.02));
					auto scene = entity.GetComponent<Scene3DCom>();
					scene->SetVisiable(true);
					scene->SetSceneData(locomotion_scene_node);

					Animation3DCom anima(entity);
					anima.m_animaRef = ResourceManager<Animation3D>::GetInstance()->TryGet("Locomotion_prefab")->Get();
					anima.m_animaRef->skeletonRef = ResourceManager<Skeleton>::GetInstance()->TryGet("Locomotion_prefab")->Get();
					anima.currentAnimName = (RAND_F(0.0, 1.0) <= 0.5) ? "Idle" : "C_Walk_IP";
					anima.playBackSpeed = RAND_F(0.5, 2.0);
					entity.AddComponent(anima);

					auto root_childrenCom_1 = world->GetComponent<ChildrenCom>(rotation_root_1);
					root_childrenCom_1->AddEntity(entity);
				}
			}
		}
	}
}
