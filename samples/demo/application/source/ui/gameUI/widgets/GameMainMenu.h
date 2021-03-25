/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: CS562
Author: Hang Yu (yohan680919@gmail.com)
Creation date: 07/28/2020
- End Header ----------------------------*/

#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	class GameMainMenu final : public BaseWidget
	{
	public:
		GameMainMenu()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 535 });
		}
		void Render() override;

		void RenderGameSettingMenu();
	private:
		ImVec2 m_Size;
	};
}