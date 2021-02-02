/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: CS562
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 07/28/2020
- End Header ----------------------------*/

#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	class GameProfilerPage final : public BaseWidget
	{
	public:
		GameProfilerPage()
		{
			m_IsVisible = false;
		}
		void Render() override;
	};
}