#pragma once
#include "engine/Engine.h"
#include "engine/EngineEssential.h"

namespace longmarch
{
	/**
	 * @brief Base widget class
	 *
	 * @author Kyle Wang (kyle.wang@digipen.edu | 60000719)
	 */
	class BaseWidget
	{
	public:
		NONCOPYABLE(BaseWidget);
		BaseWidget() = default;
		virtual ~BaseWidget() = default;
		virtual void Init() {};
		virtual void Update() {};
		virtual void Render() = 0;
		inline void SetVisible(bool vis) { m_IsVisible = vis; }
		inline bool GetVisible() const { return m_IsVisible; }
		inline ImVec4 GetStyle() const { return m_styleColor; }
		inline void SetStyle(const ImVec4& c) { m_styleColor = c; }
		inline void SetStyle(int r, int g, int b, int a) { m_styleColor = (ImVec4)ImColor::ImColor(r, g, b, a); }

		ImVec2 PosScaleBySize(const ImVec2& vec, const ImVec2& windowSize);
		ImVec2 ScaleSize(const ImVec2& itemSize);
		unsigned int GetWindowSize_X();
		unsigned int GetWindowSize_Y();

#define WIDGET_EARLY_QUIT() if (!m_ShouldDraw) { return; }
#define WIDGET_TOGGLE(KEY) 	if (auto input = InputManager::GetInstance(); input->IsKeyTriggered(KEY)) { m_ShouldDraw = !m_ShouldDraw; }

	protected:
		bool m_ShouldDraw = { true };
		bool m_IsVisible = { true };
		ImVec4 m_styleColor = { ImGuiUtil::ColGrayBG };
	};
}