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
		inline static ImGuiIO& GetIO() { return ImGui::GetIO(); }
		inline static bool IsMouseCaptured() { return GetIO().WantCaptureMouse; }
		inline static bool IsKeyBoardCaptured() { return GetIO().WantCaptureKeyboard; }

		inline static ImGuiWindowFlags popupFlag = { ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize };
		inline static ImGuiWindowFlags menuFlag = { ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse };

		inline static bool bStyleDark = true;
		inline static float alpha = 1.0f;
		static void SetupEngineImGuiStyle();
		static void InlineHelpMarker(const char* desc);
		static void TextureViewerWithZoom(const std::shared_ptr<void>& texture);
	};
}