#pragma once
#include "engine/ui/BaseWidget.h"
#include <imgui/addons/imgui-filebrowser/imfilebrowser.h>

namespace AAAAgames
{
	/**
	 * @brief Dock space for engine eidtor widgets
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class EngineEditorDock final : public BaseWidget
	{
	public:
		NONCOPYABLE(EngineEditorDock);
		EngineEditorDock();
		void Render() override;

	private:
		void ShowEngineFPS();
		void ShowEngineMainMenuBar();
		void ShowEngineMenuFile();
		void ShowEngineMenuEdit();
		void ShowEngineMenuWindow();
		void ShowEngineMenuHelp();
		void ShowGameWorldLevelTab();
		void ShowPopUps();
		void HandleFileDialog();
		void ShowEnginePerformanceMonitor();

	private:
		std::function<void()> m_aboutEditorPopup;
		std::function<void()> m_saveBeforeLoadPopup;
		std::function<void()> m_saveBeforeQuitPopup;
		ImGui::FileBrowser m_JsonSaveSceneFileDialog;
		ImGui::FileBrowser m_JsonLoadSceneFileDialog;
	};

	// utility structure for realtime plot
	struct ScrollingBuffer {
		int MaxSize;
		int Offset;
		ImVector<ImVec2> Data;
		ScrollingBuffer() {
			MaxSize = 2000;
			Offset = 0;
			Data.reserve(MaxSize);
		}
		void AddPoint(float x, float y) {
			if (Data.size() < MaxSize)
				Data.push_back(ImVec2(x, y));
			else {
				Data[Offset] = ImVec2(x, y);
				Offset = (Offset + 1) % MaxSize;
			}
		}
		void Erase() {
			if (Data.size() > 0) {
				Data.shrink(0);
				Offset = 0;
			}
		}
	};

	// utility structure for realtime plot
	struct RollingBuffer {
		float Span;
		ImVector<ImVec2> Data;
		RollingBuffer() {
			Span = 10.0f;
			Data.reserve(2000);
		}
		void AddPoint(float x, float y) {
			float xmod = fmodf(x, Span);
			if (!Data.empty() && xmod < Data.back().x)
				Data.shrink(0);
			Data.push_back(ImVec2(xmod, y));
		}
	};
}