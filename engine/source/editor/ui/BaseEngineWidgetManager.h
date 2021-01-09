#pragma once
#include "engine/ui/BaseWidgetManager.h"

namespace AAAAgames {
#define ENG_WIG_MAN_NAME "eng_wig_man"

	class EditorPickingComSys; //!< Component system side of picking system
	class EngineEditorDock; //!< Game world level tab system

	/**
	 * @brief Engine derived class of BaseWidgetManager
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class BaseEngineWidgetManager : public BaseWidgetManager
	{
	public:
		NONCOPYABLE(BaseEngineWidgetManager);
		BaseEngineWidgetManager() = default;
		virtual ~BaseEngineWidgetManager() = default;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		void CaptureMouseAndKeyboardOnMenu();	//<! Capture Mouse and Kyeboard on menu being clicked

		void PushBackSelectedEntity(const Entity& e); //!< UI side of picking system
		const A4GAMES_Vector<Entity> GetAllSelectedEntity(); //!< UI side of picking system

		void AddNewGameWorldLevel(const std::string& name);	//!< Game world level tab system

	private:
		void UpdateGameWorldTabs();	//!< Game world level tab system
		void UpdateSelectedEntity();	//!< UI side of picking system

		const A4GAMES_Vector<Entity> GetAllSelectedEntityBuffered(); // UI side of picking system

		friend EditorPickingComSys;
		friend EngineEditorDock;
	protected:
		A4GAMES_Vector<Entity> m_SelectedEntity;
		A4GAMES_Vector<Entity> m_SelectedEntityBuffer;
		//! name, isSelected, isVisible, shouldRemove
		A4GAMES_Vector<std::tuple<std::string, bool, bool, bool>> m_gameWorldLevels;
	};
}