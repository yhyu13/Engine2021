#include "engine-precompiled-header.h"
#include "BaseWidget.h"

ImVec2 AAAAgames::BaseWidget::PosScaleBySize(const ImVec2& vec, const ImVec2& windowSize)
{
	auto engine = Engine::GetInstance();
	const auto& prop = engine->GetWindow()->GetWindowProperties();
	auto width = prop.m_monitorWidth;
	auto height = prop.m_monitorHeight;
	return ImVec2(vec.x / width * windowSize.x, vec.y / height * windowSize.y);
}

ImVec2 AAAAgames::BaseWidget::ScaleSize(const ImVec2& itemSize) {
	auto engine = Engine::GetInstance();
	const auto& prop = engine->GetWindow()->GetWindowProperties();
	auto monitorWidth = prop.m_monitorWidth;
	auto monitorHeight = prop.m_monitorHeight;
	auto winWidth = prop.m_width;
	auto winHeight = prop.m_height;
	return ImVec2(itemSize.x * winWidth / monitorWidth, itemSize.y * winHeight / monitorHeight);
}

unsigned int AAAAgames::BaseWidget::GetWindowSize_X()
{
	auto engine = Engine::GetInstance();
	return engine->GetWindow()->GetWidth();
}

unsigned int AAAAgames::BaseWidget::GetWindowSize_Y()
{
	auto engine = Engine::GetInstance();
	return engine->GetWindow()->GetHeight();
}