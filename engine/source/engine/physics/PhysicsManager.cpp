#include "engine-precompiled-header.h"
#include "engine/Engine.h"

#include "PhysicsManager.h"

namespace longmarch
{

	PhysicsManager::PhysicsManager()
		: m_paused(true)
	{
		// No need to connect Update to engine update because we will use the body component system to update physics
		//{
		//	// Bind physics to late update so that it does not delay rendering, late update also allows to generate new entities from collision events
		//	Engine::GetInstance()->LateUpdate().Connect(std::bind(&PhysicsManager::Update, this, std::placeholders::_1));
		//}
		
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

	void PhysicsManager::Init()
	{
		LOCK_GUARD_NC();
	}

	void PhysicsManager::Update(double deltaTime)
	{
		ENGINE_EXCEPT(L"Logic Error. Should not use PhysicsManager::Update(double deltaTime)");
		//LOCK_GUARD_NC();
		//if (m_paused)
		//	return;

		//// use fixed timestep
		//deltaTime = 1.0 / 60.0;

		//// build islands(?) need more research

		//// do broad phase collision check (planned: BVH dynamic trees)

		//// do narrow phase collision check (GJK? or SAT)

		//// integrate physics
		//for (auto& elem : m_scenes)
		//{
		//	if(elem->IsUpdateEnabled())
		//		elem->Step(deltaTime);
		//}
	}

	void PhysicsManager::Shutdown()
	{
		LOCK_GUARD_NC();
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
