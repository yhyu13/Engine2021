/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 07/28/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   BaseGameWidgetManager.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#pragma once
#include "engine/ui/BaseWidgetManager.h"

namespace longmarch {
#define APP_WIG_MAN_NAME "app_wig_man"

	class BaseGameWidgetManager : public BaseWidgetManager
	{
	public:
		virtual void RenderUI() override;
		void CaptureMouseAndKeyboardOnMenu();

		void LoadWidget(const fs::path& filepath);
	};
}
