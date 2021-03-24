#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch
{
	/**
	 * @brief Manages all wigets winthin
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class BaseWidgetManager
	{
	public:
		NONCOPYABLE(BaseWidgetManager);
		BaseWidgetManager() = default;
		virtual ~BaseWidgetManager() = default;

		virtual void BeginFrame() { ResetCaptureMouseAndKeyboard(); };
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

		//! Should be called in BeginFrame()
		void ResetCaptureMouseAndKeyboard();

		//! Capture Mouse and Kyeboard on menu being hovered, should be called as per widget needs
		void CaptureMouseAndKeyboardOnHover();

	protected:
		LongMarch_Map<std::string, std::shared_ptr<BaseWidget>> m_WidgetLUT;
	};
}