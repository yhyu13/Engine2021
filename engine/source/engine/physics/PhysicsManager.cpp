#include "engine-precompiled-header.h"
#include "engine/Engine.h"

#include "PhysicsManager.h"

namespace longmarch
{

	PhysicsManager::PhysicsManager()
		: m_paused(true)
	{
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			ManageEventSubHandle(queue->Subscribe<PhysicsManager>(this, EngineEventType::ENG_WINDOW_INTERRUTPTION, &PhysicsManager::_ON_PAUSE));
		}
	}

	PhysicsManager* PhysicsManager::GetInstance()
	{
		static PhysicsManager instance = PhysicsManager();
		return &instance;
	}

	std::shared_ptr<Scene> PhysicsManager::CreateScene()
	{
		LOCK_GUARD_NC();
		std::shared_ptr<Scene> newScene = MemoryManager::Make_shared<Scene>();
		m_scenes.push_back(newScene);

		return newScene;
	}

	void PhysicsManager::AddScene(const std::shared_ptr<Scene>& scene)
	{
		LOCK_GUARD_NC();
		if (LongMarch_Contains(m_scenes, scene))
		{
			ENGINE_EXCEPT(L"Scene already exists!");
		}
		else
		{
			m_scenes.push_back(scene);
		}
	}

	void PhysicsManager::DeleteScene(const std::shared_ptr<Scene>& scene)
	{
		LOCK_GUARD_NC();
		std::erase(m_scenes, scene);
	}

	void PhysicsManager::_ON_PAUSE(EventQueue<EngineEventType>::EventPtr e)
	{
		LOCK_GUARD_NC();
		if (auto event = std::dynamic_pointer_cast<EngineWindowInterruptionEvent>(e); event)
		{
			m_paused = !event->m_isFocused && Engine::GetPaused();
		}
	}
}
