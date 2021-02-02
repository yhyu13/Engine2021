/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 10/25/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   PathMoverDemoMenu.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Main menu for 3D application editor
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class PathMoverDemoMenu : public BaseWidget
	{
	public:
		NONCOPYABLE(PathMoverDemoMenu);
		PathMoverDemoMenu()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 535 });
		}
		virtual void Render() override;

		void RenderPathMotionMenu();
	private:
		ImVec2 m_Size;
	};
}