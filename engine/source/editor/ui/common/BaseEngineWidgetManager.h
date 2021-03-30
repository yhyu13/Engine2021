#pragma once
#include "engine/ui/BaseWidgetManager.h"

namespace longmarch {
#define ENG_WIG_MAN_NAME "eng_wig_man"

	class EditorPickingComSys; //!< Component system side of picking system
	class EngineEditorHUD; //!< Game world level tab system

	/**
	 * @brief Engine derived class of BaseWidgetManager
	 */
	class BaseEngineWidgetManager : public BaseWidgetManager
	{
	public:
		NONCOPYABLE(BaseEngineWidgetManager);
		BaseEngineWidgetManager() = default;
		virtual ~BaseEngineWidgetManager() = default;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		//! UI side of picking system (add entity to picked entity buffer)
		void PushBackSelectedEntityBuffered(const Entity& e); 
		//! UI side of picking system (use this inside any ImGui's Render() function)
		const LongMarch_Vector<Entity> GetAllSelectedEntity(); 
		//! UI side of picking system (use this anywhere outside ImGui's Render() function)
		const LongMarch_Vector<Entity> GetAllSelectedEntityBuffered(); 
		//! Game world level tab system
		void AddNewGameWorldLevel(const std::string& name);	

	private:
		//! Game world level tab system
		void UpdateGameWorldTabs();	
		//! UI side of picking system
		void UpdateSelectedEntity();

		friend EngineEditorHUD;
	protected:
		LongMarch_Vector<Entity> m_SelectedEntity;
		LongMarch_Vector<Entity> m_SelectedEntityBuffer;
		//! Key : name | Value : isSelected, isVisible, shouldRemove
		LongMarch_Vector<std::tuple<std::string, bool, bool, bool>> m_gameWorldLevels;
	};
}