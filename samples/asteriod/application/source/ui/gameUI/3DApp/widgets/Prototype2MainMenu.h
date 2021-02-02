/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 12/02/2020
- End Header ----------------------------*/

#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch 
{
	class Prototype2MainMenu final : public BaseWidget
	{
	public:
		Prototype2MainMenu()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 535 });
		}
		void Render() override;

	private:
		ImVec2 m_Size;
	};
}