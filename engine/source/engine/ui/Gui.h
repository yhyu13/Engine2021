#pragma once
#include "ImGuiUtil.h"
#include "engine/core/EngineCore.h"

namespace longmarch
{
	namespace gui
	{
		using namespace ImGui;

		struct texture_info
		{
			std::shared_ptr<void> texture{ nullptr };
			bool is_rt{ false };
			bool is_origin_bl{ false };
		};

		//! Helper function for passing Texture to ImGui::Image.
		void Image(texture_info info, const ImVec2& _size, const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
			const ImVec2& _uv1 = ImVec2(1.0f, 1.0f), const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			const ImVec4& _borderCol = ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

		//! Helper function for passing Texture to ImGui::ImageButton.
		bool ImageButton(texture_info info, const ImVec2& _size, const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
			const ImVec2& _uv1 = ImVec2(1.0f, 1.0f), int _framePadding = -1,
			const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
			const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		bool ImageButtonEx(std::shared_ptr<void> texture, const ImVec2& size, const char* tooltip = nullptr,
			bool selected = false, bool enabled = true);

		void ImageWithAspect(texture_info info, const ImVec2& texture_size, const ImVec2& size,
			const ImVec2& _uv0 = ImVec2(0, 0), const ImVec2& _uv1 = ImVec2(1, 1),
			const ImVec4& tint_col = ImVec4(1, 1, 1, 1),
			const ImVec4& border_col = ImVec4(0, 0, 0, 0));

		int ImageButtonWithAspectAndLabel(texture_info info, const ImVec2& texture_size, const ImVec2& size,
			const ImVec2& uv0, const ImVec2& uv1, bool selected, bool* edit_label,
			const char* label, char* buf, size_t buf_size,
			ImGuiInputTextFlags flags = 0);

		bool ImageButtonWithAspectAndTextDOWN(texture_info info, const std::string& name, const ImVec2& texture_size,
			const ImVec2& imageSize, const ImVec2& uv0 = ImVec2(0, 0),
			const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1,
			const ImVec4& bg_col = ImVec4(0, 0, 0, 0),
			const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	}
}