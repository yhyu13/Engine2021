#pragma once
#include "engine/layer/Layer.h"
#include "engine/events/EventQueue.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "editor/events/EditorEventType.h"

namespace AAAAgames
{
	/**
	 * @brief 3D engine update/render pipeline
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class _3DEditorLayer final : public Layer
	{
	public:
		NONCOPYABLE(_3DEditorLayer);
		_3DEditorLayer();
		virtual void Init() override;
		virtual void OnUpdate(double ts) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void BuildRenderPipeline();
		void PreUpdate(double ts);
		void Update(double ts);
		void JoinAll();
		void PreRenderUpdate(double ts);
		void Render(double ts);
		void PostRenderUpdate(double ts);

	private:
		void _ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e);
		void _ON_LOAD_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e);

		void _ON_EDITOR_SWITCH_TO_GAME_MODE(EventQueue<EditorEventType>::EventPtr e);
		void _ON_EDITOR_SWITCH_TO_EDITING_MODE(EventQueue<EditorEventType>::EventPtr e);
	private:
		struct
		{
			std::function<void(double)> mainRenderPipeline;
		}m_Data;
		// TODO : stop calling render pipeline on unfocused
		bool m_enable_pause_on_unfocused = { true };
		ImFont* m_font;
	};
}