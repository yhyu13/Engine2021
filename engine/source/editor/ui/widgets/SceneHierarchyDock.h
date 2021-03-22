#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all scene nodes in the hierarchical order. Enables single selection and multi-selection
	 * It should act like a dockable menu
	 */
	class SceneHierarchyDock final : public BaseWidget, public BaseEventSubHandleClass
	{
	public:
		NONCOPYABLE(SceneHierarchyDock);
		SceneHierarchyDock();
		void Render() override;

		void SetSelectionMaskExclusion(const Entity& e, bool mask, bool exclusive_set_others);
		void SetSelectionMaskExclusion(const std::set<Entity>& es, bool mask, bool exclusive_set_others);

	private:
		void GenerateTreeNode(const Entity& e);

		void _ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);

	private:
		std::function<void()> m_addEntityPopup;
		std::function<void()> m_removeEntityPopup;

		ImVec2 m_Size;
		LongMarch_UnorderedMap<Entity, bool> m_PerEntitySelectionMask;
	};
}