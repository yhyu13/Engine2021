#pragma once
#include "Import.h"

namespace longmarch
{
	/**
	 * @brief Main editor layer
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
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

		void PreUpdate(double ts);
		void Update(double ts);
		void JoinAll();
		void PreRenderUpdate(double ts);
		void Render(double ts);
		void PostRenderUpdate(double ts);

		void BuildRenderPipeline();
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
		struct
		{
			std::function<void(double)> mainRenderPipeline;

			ImFont* m_font_1k;
			ImFont* m_font_2k;
			ImFont* m_font_4k;
		}m_Data;
	};
}