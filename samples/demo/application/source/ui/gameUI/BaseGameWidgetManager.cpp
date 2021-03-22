#include "application-precompiled-header.h"
#include "BaseGameWidgetManager.h"

namespace longmarch
{
	void BaseGameWidgetManager::RenderUI()
	{
		// Render widgets
		BaseWidgetManager::RenderUI();
	}

	void BaseGameWidgetManager::LoadWidget(const fs::path& filepath)
	{
		const auto& json = FileSystem::GetCachedJsonCPP(filepath);
		auto styles = json["bg-Color"];
		{
			for (auto& [name, widget] : m_WidgetLUT)
			{
				auto& style = styles[name];
				if (!style.isNull())
				{
					widget->SetStyle(style[0].asInt(), style[1].asInt(), style[2].asInt(), style[3].asInt());
				}
				else
				{
					widget->SetStyle(51, 51, 51, 127); // Default background color
				}
			}
		}
	}
}