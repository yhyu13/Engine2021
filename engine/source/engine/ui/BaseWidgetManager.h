#pragma once
#include "engine/ui/BaseWidget.h"

namespace AAAAgames
{
	/**
	 * @brief Manages all wigets winthin
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class BaseWidgetManager
	{
	public:
		NONCOPYABLE(BaseWidgetManager);
		BaseWidgetManager() = default;
		virtual ~BaseWidgetManager() = default;

		virtual void BeginFrame() {};
		virtual void RenderUI();
		virtual void EndFrame() {};

		//! Unified wiget style of all wigets winthin
		virtual void PushWidgetStyle() {};
		//! Unified wiget style of all wigets winthin
		virtual void PopWidgetStyle() {};

		void RegisterWidget(const std::string& name, const std::shared_ptr<BaseWidget>& w);
		BaseWidget* GetWidget(const std::string& name);
		void SetVisible(const std::string& name, bool visible);
		bool GetVisible(const std::string& name);

	protected:
		A4GAMES_Map<std::string, std::shared_ptr<BaseWidget>> m_WidgetLUT;
	};
}