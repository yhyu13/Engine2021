#include "engine-precompiled-header.h"
#include "Gui.h"
#include "engine/core/exception/EngineException.h"

//! Helper function for passing Texture to ImGui::Image.
void longmarch::gui::Image(texture_info info, const ImVec2& _size, const ImVec2& _uv0, const ImVec2& _uv1, const ImVec4& _tintCol, const ImVec4& _borderCol)
{
	throw NotImplementedException();
}

//! Helper function for passing Texture to ImGui::ImageButton.
bool longmarch::gui::ImageButton(texture_info info, const ImVec2& _size, const ImVec2& _uv0, const ImVec2& _uv1, int _framePadding, const ImVec4& _bgCol, const ImVec4& _tintCol)
{
	throw NotImplementedException();
	return false;
}

bool longmarch::gui::ImageButtonEx(std::shared_ptr<void> texture, const ImVec2& size, const char* tooltip, bool selected, bool enabled)
{
	throw NotImplementedException();
	return false;
}

void longmarch::gui::ImageWithAspect(texture_info info, const ImVec2& texture_size, const ImVec2& size, const ImVec2& _uv0, const ImVec2& _uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	throw NotImplementedException();
}

int longmarch::gui::ImageButtonWithAspectAndLabel(texture_info info, const ImVec2& texture_size, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, bool selected, bool* edit_label, const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags)
{
	throw NotImplementedException();
	return -1;
}

bool longmarch::gui::ImageButtonWithAspectAndTextDOWN(texture_info info, const std::string& name, const ImVec2& texture_size, const ImVec2& imageSize, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	throw NotImplementedException();
	return false;
}