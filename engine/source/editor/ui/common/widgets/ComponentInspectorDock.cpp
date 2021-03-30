#include "engine-precompiled-header.h"
#include "ComponentInspectorDock.h"
#include "../BaseEngineWidgetManager.h"
#include "engine/ecs/header/header.h"

longmarch::ComponentInspectorDock::ComponentInspectorDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 300, 600 });
	m_addComPopup = []() {};
	m_removeComPopup = []() {};
}

void longmarch::ComponentInspectorDock::Render()
{
	WIDGET_TOGGLE(KEY_F11);
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);

	auto es = manager->GetAllSelectedEntity();

	// TODO : Support for inspecting multi-entites
	bool bShouldShow = (es.size() == 1);
	if (!ImGui::Begin("Component Inspector", &m_IsVisible, ImGuiWindowFlags_HorizontalScrollbar) || !bShouldShow)
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::End();
		return;
	}

	auto entity_0 = *es.begin();
	constexpr int yoffset_item = 5;
	{
		m_addComPopup();
		if (ImGui::Button("Add Component"))
		{
			// TODO Pop Add Component Option
			m_addComPopup = [this, entity_0]()
			{
				if (!ImGui::IsPopupOpen("AddComTypePopup"))
				{
					ImGui::OpenPopup("AddComTypePopup");
				}

				if (ImGui::BeginPopupModal("AddComTypePopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
				{
					auto list_string = ObjectFactory::s_instance->GetAllComponentName();
					std::sort(list_string.begin(), list_string.end());
					const auto list_char_ptr = LongMarch_StrVec2ConstChar(list_string);

					static int selected_com_type = 0;
					if (ImGui::Combo("Component Type", &selected_com_type, &list_char_ptr[0], list_char_ptr.size()))
					{
					}
					if (ImGui::Button("Add", ImVec2(80, 0)))
					{
						ObjectFactory::s_instance->AddComponentByName(list_char_ptr[selected_com_type], EntityDecorator{ entity_0 , GameWorld::GetCurrent() });
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(80, 0)))
					{
						ImGui::CloseCurrentPopup();
						m_addComPopup = []() {};
					}
					ImGui::EndPopup();
				}
			};
		}
	}

	ImGui::SameLine();
	{
		m_removeComPopup();
		if (ImGui::Button("Remove Component"))
		{
			{
				auto entity_0 = *es.begin();
				m_removeComPopup = [this, entity_0]()
				{
					if (!ImGui::IsPopupOpen("RemoveComTypePopup"))
					{
						ImGui::OpenPopup("RemoveComTypePopup");
					}

					if (ImGui::BeginPopupModal("RemoveComTypePopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
					{
						auto list_string = ObjectFactory::s_instance->GetAllComponentName();
						std::sort(list_string.begin(), list_string.end());
						const auto list_char_ptr = LongMarch_StrVec2ConstChar(list_string);

						static int selected_com_type = 0;
						if (ImGui::Combo("Component Type", &selected_com_type, &list_char_ptr[0], list_char_ptr.size()))
						{
						}

						ImGui::Text(Str("Are you sure to remove: " + list_string[selected_com_type] + " ?").c_str());

						if (ImGui::Button("Yes", ImVec2(80, 0)))
						{
							ObjectFactory::s_instance->RemoveComponentByName(list_char_ptr[selected_com_type], EntityDecorator{ entity_0 , GameWorld::GetCurrent() });
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(80, 0)))
						{
							ImGui::CloseCurrentPopup();
							m_removeComPopup = []() {};
						}
						ImGui::EndPopup();
					}
				};
			}
		}
	}

	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		for (auto& com : GameWorld::GetCurrent()->GetAllComponent(entity_0))
		{
			com->ImGuiRender();
		}
	}

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
	ImGui::End();
}