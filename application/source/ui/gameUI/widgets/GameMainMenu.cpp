#include "application-precompiled-header.h"
#include "GameMainMenu.h"
#include "ui/gameUI/BaseGameWidgetManager.h"

void AAAAgames::GameMainMenu::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseGameWidgetManager>(APP_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize);
	if (!ImGui::Begin("Main Menu", &m_IsVisible))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::PopStyleColor(1);
		ImGui::End();
		return;
	}

	int yoffset_item = 5;
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
		if (ImGui::Button("Application Profiler page"))
		{
			bool b = manager->GetVisible("ProfilerPage");
			manager->SetVisible("ProfilerPage", !b);
		}
	}

	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		RenderGameSettingMenu();
	}

	manager->CaptureMouseAndKeyboardOnMenu();
	manager->PopWidgetStyle();
	ImGui::PopStyleColor(1);
	ImGui::End();
}

void AAAAgames::GameMainMenu::RenderGameSettingMenu()
{
	if (ImGui::TreeNode("Game Settings"))
	{
		if (ImGui::BeginTabBar("Game Settings"))
		{
			if (ImGui::Button("Reset game settings to default values"))
			{
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}