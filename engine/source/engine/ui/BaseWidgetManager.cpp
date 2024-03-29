#include "engine-precompiled-header.h"
#include "BaseWidgetManager.h"

void longmarch::BaseWidgetManager::RenderUI()
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

void longmarch::BaseWidgetManager::RegisterWidget(const std::string& name, const std::shared_ptr<BaseWidget>& w)
{
	m_WidgetLUT.emplace(name, w);
}

BaseWidget* longmarch::BaseWidgetManager::GetWidget(const std::string& name)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		return it->second.get();
	}
	else
	{
		ENGINE_EXCEPT(wStr(name) + L" does not exist!");
		return nullptr;
	}
}

void longmarch::BaseWidgetManager::SetVisible(const std::string& name, bool visible)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		it->second->SetVisible(visible);
	}
	else
	{
		ENGINE_EXCEPT(wStr(name) + L" does not exist!");
	}
}

bool longmarch::BaseWidgetManager::GetVisible(const std::string& name)
{
	if (auto it = m_WidgetLUT.find(name); it != m_WidgetLUT.end())
	{
		return it->second->GetVisible();
	}
	else
	{
		ENGINE_EXCEPT(wStr(name) + L" does not exist!");
		return false;
	}
}

void longmarch::BaseWidgetManager::ResetCaptureMouseAndKeyboard()
{
	ImGuiUtil::IgnoreMouseCaptured = ImGuiUtil::IgnoreKeyBoardCaptured = true;
}

void longmarch::BaseWidgetManager::CaptureMouseAndKeyboardOnHover(bool resetIgnoreCapture)
{
	ImGuiIO& io = ImGui::GetIO();
	bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || ImGui::IsAnyItemHovered();
	// Set ImGui capture inputs on window focused
	if (isWindowHovered)
	{
		io.WantCaptureMouse = io.WantCaptureKeyboard = true;
		if (resetIgnoreCapture)
		{
			ImGuiUtil::IgnoreMouseCaptured = ImGuiUtil::IgnoreKeyBoardCaptured = false;
		}
	}
}
