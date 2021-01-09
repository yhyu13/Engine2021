#include "engine-precompiled-header.h"
#include "BaseWidgetManager.h"

void AAAAgames::BaseWidgetManager::RenderUI()
{
	// Render widgets
	for (auto& [_, widget] : m_WidgetLUT)
	{
		if (widget->GetVisible())
		{
			widget->Render();
		}
	}
}

void AAAAgames::BaseWidgetManager::RegisterWidget(const std::string& name, const std::shared_ptr<BaseWidget>& w)
{
	m_WidgetLUT.emplace(name, w);
}

BaseWidget* AAAAgames::BaseWidgetManager::GetWidget(const std::string& name)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		return it->second.get();
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(name) + L" does not exist!");
	}
}

void AAAAgames::BaseWidgetManager::SetVisible(const std::string& name, bool visible)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		it->second->SetVisible(visible);
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(name) + L" does not exist!");
	}
}

bool AAAAgames::BaseWidgetManager::GetVisible(const std::string& name)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		return it->second->GetVisible();
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(name) + L" does not exist!");
	}
}