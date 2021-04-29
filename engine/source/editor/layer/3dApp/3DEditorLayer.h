#pragma once
#include "engine/layer/Layer.h"
#include "engine/events/EventQueue.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "editor/events/EditorEventType.h"

namespace longmarch
{
	/**
	 * @brief 3D engine update/render pipeline
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
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

		void PreUpdate(double ts);
		void Update(double ts);
		void JoinAll();
		void PreRenderUpdate(double ts);
		void Render(double ts);
		void PostRenderUpdate(double ts);

		void BuildRenderPipeline();

	private:
		void _ON_EDITOR_SWITCH_TO_GAME_MODE(EventQueue<EditorEventType>::EventPtr e);
		void _ON_EDITOR_SWITCH_TO_EDITING_MODE(EventQueue<EditorEventType>::EventPtr e);

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