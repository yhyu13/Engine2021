#pragma once
#include "engine/ui/BaseWidget.h"
#include <imgui/addons/imgui-filebrowser/imfilebrowser.h>

namespace longmarch
{
	/**
	 * @brief Dock space for engine eidtor widgets
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class EngineEditorHUD final : public BaseWidget
	{
	public:
		NONCOPYABLE(EngineEditorHUD);
		EngineEditorHUD();
		void Render() override;

	private:
		void SetupEngineImGuiStyle();
		void ShowEngineFPS();
		void ShowEngineMainMenuBar();
		void ShowEngineMenuFile();
		void ShowEngineMenuEdit();
		void ShowEngineMenuWindow();
		void ShowEngineMenuHelp();
		void ShowGameWorldLevelTab();
		void ShowPopUps();
		void HandleFileDialog();

	public:
		bool bStyleDark{ true };
		float fStyleAlpha{ 1.0f };

	private:
		std::function<void()> m_aboutEditorPopup;
		std::function<void()> m_saveBeforeLoadPopup;
		std::function<void()> m_saveBeforeQuitPopup;
		ImGui::FileBrowser m_JsonSaveSceneFileDialog;
		ImGui::FileBrowser m_JsonLoadSceneFileDialog;
	};
}