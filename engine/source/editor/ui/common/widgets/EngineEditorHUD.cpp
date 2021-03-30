#include "engine-precompiled-header.h"
#include "EngineEditorHUD.h"
#include "../BaseEngineWidgetManager.h"
#include "engine/ecs/header/header.h"

#include <imgui/addons/implot/implot.h>

longmarch::EngineEditorHUD::EngineEditorHUD()
{
	m_IsVisible = true;
	m_aboutEditorPopup = []() {};
	m_saveBeforeLoadPopup = []() {}; // TODO
	m_saveBeforeQuitPopup = []() {}; // TODO

	m_JsonSaveSceneFileDialog.SetTitle("Json Save Explorer");
	m_JsonSaveSceneFileDialog.SetTypeFilters({ ".json" });
	m_JsonSaveSceneFileDialog.SetPwd(FileSystem::ResolveProtocol("$asset:archetype/"));

	m_JsonLoadSceneFileDialog.SetTitle("Json Load Explorer");
	m_JsonLoadSceneFileDialog.SetTypeFilters({ ".json" });
	m_JsonLoadSceneFileDialog.SetPwd(FileSystem::ResolveProtocol("$asset:archetype/"));
}

void longmarch::EngineEditorHUD::Render()
{
	// Setup Dear ImGui style
	ImGuiUtil::SetupEngineImGuiStyle();

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowBgAlpha(0.0f);

	constexpr auto window_flags = ImGuiWindowFlags_MenuBar \
		| ImGuiWindowFlags_NoFocusOnAppearing \
		| ImGuiWindowFlags_NoDecoration \
		| ImGuiWindowFlags_NoScrollWithMouse \
		| ImGuiWindowFlags_NoDocking \
		| ImGuiWindowFlags_NoMove \
		| ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (!ImGui::Begin("EngineEditorHUD", &m_IsVisible, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::PopStyleVar(3);
		ImGui::End();
		return;
	}
	ImGui::PopStyleVar(3);
	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("HUDDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	ShowEngineFPS();
	ShowEngineMainMenuBar();
	ShowGameWorldLevelTab();
	ShowPopUps();
	HandleFileDialog();

	ImGui::End();
}

void longmarch::EngineEditorHUD::ShowEngineFPS()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	float frameTime = FramerateController::GetInstance()->GetFrameTime();
	ImVec2 fpsWindowSize = ImVec2(90, 50);
	ImVec2 waveWindowPos = ImVec2(viewport->Pos.x + viewport->Size.x / 2 - fpsWindowSize.x / 2, viewport->Pos.y + 50);
	ImGui::SetNextWindowPos(waveWindowPos);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtil::ColGreen);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("FPS", fpsWindowSize, false, ImGuiUtil::menuFlag);
	ImGui::SetCursorPosX(0.0f);
	ImGui::SetCursorPosY(ImGui::GetFontSize() / 4);
	ImGui::Text("%.2f FPS \n%.2f ms", 1.0f / frameTime, frameTime * 1e3);
	ImGui::EndChild();
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}

void longmarch::EngineEditorHUD::ShowEngineMainMenuBar()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowEngineMenuFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ShowEngineMenuEdit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			ShowEngineMenuWindow();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ShowEngineMenuHelp();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
}

void longmarch::EngineEditorHUD::ShowEngineMenuFile()
{
	ImGui::MenuItem("Load & Save", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::MenuItem("Open", "Ctrl+O"))
	{
		m_JsonLoadSceneFileDialog.Open();
	}
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S"))
	{
		m_JsonSaveSceneFileDialog.Open();
	}
	if (ImGui::MenuItem("Save As.."))
	{
		m_JsonSaveSceneFileDialog.Open();
	}
	ImGui::Separator();
	if (ImGui::BeginMenu("Options"))
	{
		static bool enabled = true;
		ImGui::MenuItem("Enabled", "", &enabled);
		ImGui::BeginChild("child", ImVec2(0, 60), true);
		for (int i = 0; i < 10; i++)
			ImGui::Text("Scrolling Text %d", i);
		ImGui::EndChild();
		static float f = 0.5f;
		static int n = 0;
		static bool b = true;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::Checkbox("Check", &b);
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Exit"))
	{
		Engine::SetQuit(true);
	}
}

void longmarch::EngineEditorHUD::ShowEngineMenuEdit()
{
	ImGui::MenuItem("History", NULL, false, false);
	if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
	if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
	ImGui::Separator();
	ImGui::MenuItem("Edit", NULL, false, false);
	if (ImGui::MenuItem("Cut", "CTRL+X")) {}
	if (ImGui::MenuItem("Copy", "CTRL+C")) {}
	if (ImGui::MenuItem("Paste", "CTRL+V")) {}
}

void longmarch::EngineEditorHUD::ShowEngineMenuWindow()
{
	ImGui::MenuItem("Level Editor", NULL, false, false);
	ImGui::Separator();
	ImGui::MenuItem("General", NULL, false, false);
}

void longmarch::EngineEditorHUD::ShowEngineMenuHelp()
{
	ImGui::MenuItem("Application", NULL, false, false);
	if (ImGui::MenuItem("About GSWY Engine Editor"))
	{
		m_aboutEditorPopup = [this]()
		{
			if (!ImGui::IsPopupOpen("AboutEnginePopup"))
			{
				ImGui::OpenPopup("AboutEnginePopup");
			}

			if (ImGui::BeginPopupModal("AboutEnginePopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("4a games Engine Editor \n Version : 0.10.0 \n Branch: develop_engine_proof \n Build: Nov 2020 \n Graphics RHI: OpenGL 4.5");
				if (ImGui::Button("Close", ImVec2(80, 0)))
				{
					ImGui::CloseCurrentPopup();
					m_aboutEditorPopup = []() {};
				}
				ImGui::EndPopup();
			}
		};
	}
}

void longmarch::EngineEditorHUD::ShowGameWorldLevelTab()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();

	ImGui::Begin("Game World Level", nullptr, ImGuiWindowFlags_NoDecoration);
	ImGui::BeginTabBar("Level", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip | ImGuiTabBarFlags_FittingPolicyMask_);

	auto& levels = manager->m_gameWorldLevels;
	static auto num_visible_counter = [](const auto& levels)->size_t
	{
		size_t ret(0u);
		for (auto& [_, __, isVisible, ___] : levels)
		{
			if (isVisible)
			{
				++ret;
			}
		}
		return ret;
	};
	for (auto& [name, isSelect, isVisible, shouldRemove] : levels)
	{
		// Skip closed tab items
		if (!isVisible)
		{
			continue;
		}
		bool prev_isVisible = isVisible;
		if (ImGui::BeginTabItem(name.c_str(), &isVisible, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
		{
			isSelect = true;
			ImGui::EndTabItem();
		}
		else
		{
			isSelect = false;
		}
		// Leave at least one visible game level tab when a tab closing event is triggered
		if (prev_isVisible && !isVisible)
		{
			shouldRemove = true;
			if (num_visible_counter(levels) == 0)
			{
				isVisible = true;
				shouldRemove = false;
			}
		}
	}

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
	ImGui::EndTabBar();
	ImGui::End();
}

void longmarch::EngineEditorHUD::ShowPopUps()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();

	m_aboutEditorPopup();
	m_saveBeforeLoadPopup();
	m_saveBeforeQuitPopup();

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
}

void longmarch::EngineEditorHUD::HandleFileDialog()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	{
		m_JsonSaveSceneFileDialog.Display();
		if (m_JsonSaveSceneFileDialog.HasSelected())
		{
			auto filePath = m_JsonSaveSceneFileDialog.GetSelected().string();
			DEBUG_PRINT("Selected save file: " + filePath);
			m_JsonSaveSceneFileDialog.ClearSelected();
			{
				auto queue = EventQueue<EngineIOEventType>::GetInstance();
				{
					auto e = MemoryManager::Make_shared<EngineSaveSceneBeginEvent>(filePath, GameWorld::GetCurrent());
					queue->Publish(e);
				}
				{
					auto e = MemoryManager::Make_shared<EngineSaveSceneEvent>(filePath, GameWorld::GetCurrent());
					queue->Publish(e);
				}
				{
					auto e = MemoryManager::Make_shared<EngineSaveSceneEndEvent>(filePath, GameWorld::GetCurrent());
					queue->Publish(e);
				}
			}
		}
	}

	{
		m_JsonLoadSceneFileDialog.Display();
		if (m_JsonLoadSceneFileDialog.HasSelected())
		{
			auto filePath = m_JsonLoadSceneFileDialog.GetSelected();
			auto str_filePath = m_JsonLoadSceneFileDialog.GetSelected().string();
			// If we are to load the same world as the current world, set_current should be true in order to always have a valid current world
			bool set_current = (filePath.filename().string() == GameWorld::GetCurrent()->GetName());
			DEBUG_PRINT("Selected load file: " + str_filePath);
			m_JsonLoadSceneFileDialog.ClearSelected();
			{
				auto queue = EventQueue<EngineIOEventType>::GetInstance();
				{
					auto e = MemoryManager::Make_shared<EngineLoadSceneBeginEvent>(str_filePath, set_current);
					queue->Publish(e);
				}
				{
					auto e = MemoryManager::Make_shared<EngineLoadSceneEvent>(str_filePath, set_current);
					queue->Publish(e);
				}
				{
					auto e = MemoryManager::Make_shared<EngineLoadSceneEndEvent>(str_filePath, set_current);
					queue->Publish(e);
				}
			}
		}
	}	
	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
}