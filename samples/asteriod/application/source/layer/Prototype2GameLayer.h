/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/13/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   Prototype2GameLayer.h
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
	class Prototype2GameLayer final : public Layer
	{
	public:
		NONCOPYABLE(Prototype2GameLayer);
		Prototype2GameLayer();
		virtual void Init() override;
		virtual void OnUpdate(double ts) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void LoadResources();
		void InitFramework();
		void InitGameWorld();

		void PreUpdate(double ts);
		void Update(double ts);
		void JoinAll();
		void PreRenderUpdate(double ts);
		void Render(double ts);
		void PostRenderUpdate(double ts);

		void BuildRenderPipeline();

	private:
		void _ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_LOAD_SCENE(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_LOAD_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e);

		void _ON_SAVE_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_SAVE_SCENE(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_SAVE_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e);

		void _ON_WINDOW_INTERRUPT(EventQueue<EngineEventType>::EventPtr e);

	private:		
		struct
		{
			std::function<void(double)> mainRenderPipeline;
		}m_Data;
		ImFont* m_font;
	};
}