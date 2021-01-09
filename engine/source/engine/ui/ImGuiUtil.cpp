#include "engine-precompiled-header.h"
#include "ImGuiUtil.h"
#include "engine/renderer/Texture.h"
#include "engine/math/MathUtil.h"

void AAAAgames::ImGuiUtil::SetupEngineImGuiStyle()
{
	static bool _bStyleDark_ = !bStyleDark;
	static float _alpha_ = 1.0f - alpha;
	if (_bStyleDark_ == bStyleDark && _alpha_ == alpha)
	{
		return;
	}
	_bStyleDark_ = bStyleDark;
	_alpha_ = alpha;

	if (bStyleDark)
	{
		ImGui::StyleColorsDark();
	}
	else
	{
		ImGui::StyleColorsLight();
	}
	ImGuiStyle& style = ImGui::GetStyle();
	for (int i = 0; i <= ImGuiCol_COUNT; i++)
	{
		ImVec4& col = style.Colors[i];
		if (col.w < 1.00f)
		{
			col.w *= alpha;
		}
	}
}

void AAAAgames::ImGuiUtil::InlineHelpMarker(const char* desc)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void AAAAgames::ImGuiUtil::TextureViewerWithZoom(const std::shared_ptr<void>& texture)
{
	if (!texture)
	{
		return;
	}
	ImVec2 pos = ImGui::GetCursorScreenPos();
	auto tex = std::static_pointer_cast<Texture2D>(texture);
	ImTextureID my_tex_id = reinterpret_cast<ImTextureID>(tex->GetRendererID());
	float my_tex_w = MIN(256, tex->GetWidth());
	float my_tex_h = my_tex_w * tex->GetHeight() / tex->GetWidth();
	{
		ImGui::Text("%dx%d", (int)tex->GetWidth(), (int)tex->GetHeight());
		ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
		ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
		ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			float region_sz = 32.0f;
			auto& io = ImGui::GetIO();
			float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
			float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
			static float zoom = 4.0f;
			if (region_x < 0.0f) { region_x = 0.0f; }
			else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
			if (region_y < 0.0f) { region_y = 0.0f; }
			else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
			ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
			ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
			ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
			ImGui::EndTooltip();
		}
	}
}