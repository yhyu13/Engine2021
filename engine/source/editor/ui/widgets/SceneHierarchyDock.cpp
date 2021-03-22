#include "engine-precompiled-header.h"
#include "SceneHierarchyDock.h"
#include "../BaseEngineWidgetManager.h"
#include "engine/ecs/header/header.h"
#include "editor/events/EditorCustomEvent.h"

longmarch::SceneHierarchyDock::SceneHierarchyDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 300, 600 });

	m_addEntityPopup = []() {};
	m_removeEntityPopup = []() {};

	{
		auto queue = EventQueue<EngineIOEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<SceneHierarchyDock>(this, EngineIOEventType::LOAD_SCENE_BEGIN, &SceneHierarchyDock::_ON_LOAD_SCENE_BEGIN));
	}
}

void longmarch::SceneHierarchyDock::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);
	if (!ImGui::Begin("Scene Node Menu", &m_IsVisible, ImGuiWindowFlags_HorizontalScrollbar))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::End();
		return;
	}

	constexpr int yoffset_item = 5;
	{
		m_addEntityPopup();
		if (ImGui::Button("Add Entity"))
		{
			// Add entity is valid when only one entity is selected
			auto es = manager->GetAllSelectedEntity();
			bool bShouldShow = (es.size() == 1);
			if (bShouldShow)
			{
				auto entity_0 = *es.begin();
				m_addEntityPopup = [this, entity_0]()
				{
					if (!ImGui::IsPopupOpen("AddEntityTypePopup"))
					{
						ImGui::OpenPopup("AddEntityTypePopup");
					}

					if (ImGui::BeginPopupModal("AddEntityTypePopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
					{
						auto list_string = ObjectFactory::s_instance->GetAllEntityTypeName();
						// Erase reserved options
						std::erase(list_string, "SCENE_ROOT");
						std::sort(list_string.begin(), list_string.end());
						const auto list_char_ptr = LongMarch_StrVec2ConstChar(list_string);

						static int selected_entity_type = 0;
						if (ImGui::Combo("Entity Type", &selected_entity_type, &list_char_ptr[0], list_char_ptr.size()))
						{
						}
						if (ImGui::Button("Add", ImVec2(80, 0)))
						{
							auto e_type = ObjectFactory::s_instance->GetEntityTypeFromName(list_char_ptr[selected_entity_type]);
							auto e = GameWorld::GetCurrent()->GenerateEntity3DNoCollision(e_type, true, false);
							GameWorld::GetCurrent()->AddChildHelper(entity_0, e);
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(80, 0)))
						{
							ImGui::CloseCurrentPopup();
							m_addEntityPopup = []() {};
						}
						ImGui::EndPopup();
					}
				};
			}
		}
	}

	ImGui::SameLine();
	{
		m_removeEntityPopup();
		if (ImGui::Button("Remove Entity"))
		{
			// Add entity is valid when only one entity is selected
			auto es = manager->GetAllSelectedEntity();
			bool bShouldShow = (es.size() == 1);
			if (bShouldShow)
			{
				auto entity_0 = *es.begin();
				m_addEntityPopup = [this, entity_0]()
				{
					if (!ImGui::IsPopupOpen("RemoveEntityTypePopup"))
					{
						ImGui::OpenPopup("RemoveEntityTypePopup");
					}

					if (ImGui::BeginPopupModal("RemoveEntityTypePopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
					{
						std::string name(Str(entity_0));
						if (auto nameCom = GameWorld::GetCurrent()->GetComponent<IDNameCom>(entity_0); nameCom.Valid())
						{
							name = nameCom->GetUniqueName();
						}
						ImGui::Text(Str("Are you sure to remove: " + name + " ?").c_str());
						if (ImGui::Button("Yes", ImVec2(80, 0)))
						{
							auto queue = EventQueue<EngineEventType>::GetInstance();
							auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(EntityDecorator{ entity_0 , GameWorld::GetCurrent() });
							queue->Publish(e);
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(80, 0)))
						{
							ImGui::CloseCurrentPopup();
							m_addEntityPopup = []() {};
						}
						ImGui::EndPopup();
					}
				};
			}
		}
	}

	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		// Get scene roots
		auto world = GameWorld::GetCurrent();
		auto root = world->GetTheOnlyEntityWithType((EntityType)(EngineEntityType::SCENE_ROOT));
		{
			if (ImGui::TreeNode(("Scene Nodes " + Str(root)).c_str()))
			{
				// Increase spacing to differentiate leaves from expanded contents.
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1);
				// Recursively display tree nodes
				GenerateTreeNode(root);
				ImGui::PopStyleVar();
				ImGui::TreePop();
			}
		}
	}
	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
	ImGui::End();
}

void longmarch::SceneHierarchyDock::GenerateTreeNode(const Entity& e)
{
	auto world = GameWorld::GetCurrent();
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	auto selection_mask = m_PerEntitySelectionMask[e];
	auto childrenCom = world->GetComponent<ChildrenCom>(e);
	auto idNameCom = world->GetComponent<IDNameCom>(e);
	if (childrenCom.Valid())
	{
		// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (selection_mask)
		{
			/*
				Update seleted items to widget manager which communicate with ECS
			*/
			node_flags |= ImGuiTreeNodeFlags_Selected;
			manager->PushBackSelectedEntityBuffered(e);
		}

		// Handle entity with children
		if (!childrenCom->IsLeaf())
		{
			bool node_open = ImGui::TreeNodeEx(idNameCom->GetUniqueName().c_str(), node_flags, idNameCom->GetName().c_str());
			if (ImGui::IsItemClicked())
			{
				SetSelectionMaskExclusion(e, true, !ImGui::GetIO().KeyCtrl);
				if (ImGui::IsMouseDoubleClicked(0))
				{
					/*
						Teleport the Editor camera to selected item
					*/
					auto engineEventQueue = EventQueue<EditorEventType>::GetInstance();
					auto event = MemoryManager::Make_shared<EditorCameraTeleportToEntityEvent>(EntityDecorator {e, GameWorld::GetCurrent()});
					engineEventQueue->Publish(event);
				}
			}
			if (node_open)
			{
				for (auto& child : childrenCom->GetChildren())
				{
					GenerateTreeNode(child);
				}
				ImGui::TreePop();
			}
		}
		// Handle entity that is leaf
		else
		{
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx(idNameCom->GetUniqueName().c_str(), node_flags, idNameCom->GetName().c_str());
			if (ImGui::IsItemClicked())
			{
				// TODO : implement multi shift multi-selection
				SetSelectionMaskExclusion(e, true, !ImGui::GetIO().KeyCtrl);
				if (ImGui::IsMouseDoubleClicked(0))
				{
					auto engineEventQueue = EventQueue<EditorEventType>::GetInstance();
					auto event = MemoryManager::Make_shared<EditorCameraTeleportToEntityEvent>(EntityDecorator {e, GameWorld::GetCurrent()});
					engineEventQueue->Publish(event);
				}
			}
		}
	}
	else
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"All scene node should have a valid ChildrenCom");
	}
}

void longmarch::SceneHierarchyDock::SetSelectionMaskExclusion(const Entity& e, bool mask, bool exclusive_set_others)
{
	// Set selected item to be same status as mask
	{
		m_PerEntitySelectionMask[e] = mask;
	}
	// Exclusive all other item to be opposite status as mask
	if (exclusive_set_others)
	{
		for (auto& item : m_PerEntitySelectionMask)
		{
			if (item.first != e)
			{
				item.second = !mask;
			}
		}
	}
}

void longmarch::SceneHierarchyDock::SetSelectionMaskExclusion(const std::set<Entity>& es, bool mask, bool exclusive_set_others)
{
	// Set selected items to be same status as mask
	for (auto& e : es)
	{
		m_PerEntitySelectionMask[e] = mask;
	}
	// Exclusive all other item to be opposite status as mask
	if (exclusive_set_others)
	{
		for (auto& item : m_PerEntitySelectionMask)
		{
			if (es.find(item.first) == es.end())
			{
				item.second = !mask;
			}
		}
	}
}

void longmarch::SceneHierarchyDock::_ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e)
{
	m_PerEntitySelectionMask.clear();
}