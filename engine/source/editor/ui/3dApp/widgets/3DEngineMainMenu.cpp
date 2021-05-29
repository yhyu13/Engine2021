#include "engine-precompiled-header.h"
#include "3DEngineMainMenu.h"
#include "../3DEngineWidgetManager.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include "editor/ui/common/widgets/EngineEditorHUD.h"

#include <imgui/addons/implot/implot_items.cpp>
#include <imgui/addons/implot/implot_demo.cpp>

void longmarch::_3DEngineMainMenu::Render()
{
	WIDGET_TOGGLE(KEY_F11);
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);
	if (!ImGui::Begin("Engine Main Menu", &m_IsVisible))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::End();
		return;
	}

	constexpr int yoffset_item = 5;
	{
		static bool bDemoPage = false;
		if (ImGui::Button("ImGui Demo Page"))
		{
			bDemoPage = !bDemoPage;
		}
		if (bDemoPage)
		{
			ImGui::ShowDemoWindow();
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		static bool bplotDemoPage = false;
		if (ImGui::Button("ImPlot Demo Page"))
		{
			bplotDemoPage = !bplotDemoPage;
		}
		if (bplotDemoPage)
		{
			ImPlot::ShowDemoWindow();
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		if (ImGui::Button("Engine Profiler page"))
		{
			bool visible = manager->GetVisible("ProfilerPage");
			visible = !visible;
			manager->SetVisible("ProfilerPage", visible);
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));

	{
		// Game setting
		RenderEngineSettingMenu();
	}

	ImGui::Dummy(ImVec2(0, yoffset_item));

	{
		// Graphics setting
		RenderEngineGraphicSettingMenu();
	}

	manager->CaptureMouseAndKeyboardOnHover(true);
	manager->PopWidgetStyle();
	ImGui::End();
}

void longmarch::_3DEngineMainMenu::RenderEngineSettingMenu()
{
	if (ImGui::TreeNode("Engine Settings"))
	{
		constexpr int yoffset_item = 5;

		auto settingEventQueue = EventQueue<EngineSettingEventType>::GetInstance();

		static auto engineConfiguration = FileSystem::GetNewJsonCPP("$root:engine-config.json");
		static bool checkInterrutionHandle = engineConfiguration["engine"]["Pause-on-unfocused"].asBool();

		if (ImGui::Button("Reset engine settings to default values"))
		{
			auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
			auto editor_hud = static_cast<EngineEditorHUD*>(manager->GetWidget("0_HUD"));
			editor_hud->bStyleDark = true;
			editor_hud->fStyleAlpha = 1.0f;

			checkInterrutionHandle = engineConfiguration["engine"]["Pause-on-unfocused"].asBool();
			{
				auto e = MemoryManager::Make_shared<ToggleWindowInterrutpionEvent>(checkInterrutionHandle);
				settingEventQueue->Publish(e);
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
			auto editor_hud = static_cast<EngineEditorHUD*>(manager->GetWidget("0_HUD"));

			auto style = editor_hud->bStyleDark;
			if (ImGui::Checkbox("Dark Theme", &style))
			{
				editor_hud->bStyleDark = style;
			}
			auto alpha = editor_hud->fStyleAlpha;
			if (ImGui::SliderFloat("Alpha", &alpha, 0.01f, 1.0f, "%.2f"))
			{
				editor_hud->fStyleAlpha = alpha;
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));

		if (ImGui::Checkbox("Enable pause on window unfocused", &checkInterrutionHandle))
		{
			{
				auto e = MemoryManager::Make_shared<ToggleWindowInterrutpionEvent>(checkInterrutionHandle);
				settingEventQueue->Publish(e);
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));

		if (ImGui::BeginTabBar("Engine Settings"))
		{
			ImGui::EndTabBar();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}

void longmarch::_3DEngineMainMenu::RenderEngineGraphicSettingMenu()
{
	Renderer3D::EditorRenderGraphicsSettings();
}