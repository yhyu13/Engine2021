#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/Engine.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs//GameWorld.h"
#include "engine/events/EventQueue.h"
#include "editor/events/EditorEventType.h"

namespace longmarch
{
	/**
	 * @brief Editor's camera controller
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class EditorCameraControllerComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(EditorCameraControllerComSys);

		EditorCameraControllerComSys() = default;
		virtual void Init() override;
		virtual void Update(double dt) override;

		virtual std::shared_ptr<BaseComponentSystem> Copy() const override;

	private:
		void _ON_CAM_TELEPORT_TO_ENTITY(EventQueue<EditorEventType>::EventPtr e);

	private:
		GameWorld* m_editingWorld{ nullptr };
	};
}