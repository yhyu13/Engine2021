/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: CS562
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 09/07/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   3DGameWidgetManager.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#pragma once
#include "engine/ui/BaseWidget.h"
#include "../BaseGameWidgetManager.h"

namespace longmarch {

	/**
	 * @brief BaseWidget manager for 3D application editor
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class _3DGameWidgetManager final : public BaseGameWidgetManager
	{
	public:
		NONCOPYABLE(_3DGameWidgetManager);
		_3DGameWidgetManager();

		virtual void PushWidgetStyle() override {};
		virtual void PopWidgetStyle() override {};
	};
}