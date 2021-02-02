/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/13/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   MainGameLayer.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      05/13/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#pragma once
#include "Import.h"

namespace longmarch
{
	/**
	 * @brief Main editor layer
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class MainGameLayer final : public Layer
	{
	public:
		NONCOPYABLE(MainGameLayer);
		MainGameLayer();
		virtual void Init() override;
		virtual void OnUpdate(double ts) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void LoadResources();
		void InitFramework();
		void InitGameWorld();

		void BuildTestScene();

	private:
		void _ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_LOAD_SCENE(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_LOAD_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e);

		void _ON_SAVE_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_SAVE_SCENE(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_SAVE_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e);

		void _ON_WINDOW_INTERRUPT(EventQueue<EngineEventType>::EventPtr e);

	private:
		ImFont* m_font;
	};
}