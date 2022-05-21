#pragma once
#include <string>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace longmarch
{
#define LongMarch_ImGuiHashTagName(name, hastagName, ...) ((std::string(name) + "###" + std::string(hastagName)).c_str())
	class ImGuiUtil
	{
	public:
		inline static ImVec4 ColGrayBG = { ImVec4(0.2f,0.2f,0.2f,0.75f) };
		inline static ImVec4 ColRed = { ImVec4(1.f,0.1f,0.1f,1.f) };
		inline static ImVec4 ColGreen = { ImVec4(0.1f,1.f,0.1f,1.f) };
		inline static ImVec4 ColBlue = { ImVec4(0.1f,0.1f,1.f,1.f) };
		inline static ImVec4 ColWhite = { ImVec4(1.f,1.f,1.f,1.f) };
		inline static ImVec4 ColGray = { ImVec4(0.2f,0.2f,0.2f,1.f) };
		inline static ImVec4 ColNavi = { ImVec4(0.26f, 0.59f, 0.98f, 1.00f) };

		inline static ImGuiWindowFlags popupFlag = { ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize };
		inline static ImGuiWindowFlags menuFlag = { ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse };

		inline static bool IgnoreMouseCaptured{ false }; //!< Custom boolean variable that need to be set every frame by the current widget manager. Ignore is usually set by the editor scene docker.
		inline static bool IgnoreKeyBoardCaptured{ false }; //!< Custom boolean variable that need to be set every frame by the current widget manager. Ignore is usually set by the editor scene docker.
		static bool IsMouseCaptured(bool considerIgnore);
		static bool IsKeyBoardCaptured(bool considerIgnore);

		//! Draw a question mark and display text on hover
		static void InlineHelpMarker(const char* desc);

		//! Zoom in texture on hover (used in editor panel)
		static void TextureViewerWithZoom(const std::shared_ptr<void>& texture); 

		//! Scale imgui all sizes (used on changing window resolution such as from 1080p to 4k)
		static void ScaleAllSizesFromBase(float scale = 1.0f);
	};
}