#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all components info
	 * It should act like a dockable menu
	 */
	class ComponentInspectorDock final : public BaseWidget
	{
	public:
		NONCOPYABLE(ComponentInspectorDock);
		ComponentInspectorDock();
		void Render() override;

	private:
		std::function<void()> m_addComPopup;
		std::function<void()> m_removeComPopup;

		ImVec2 m_Size;
	};
}