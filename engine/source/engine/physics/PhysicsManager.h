#pragma once

#include <vector>
#include <map>

#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/EngineCore.h"
#include "engine/math/Geommath.h"
#include "engine/core/thread/Lock.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "engine/events/EventQueue.h"

#include "Scene.h"

namespace longmarch
{
	class ENGINE_API PhysicsManager final : private BaseAtomicClassNC, public BaseEventSubHandleClass
	{
	public:
		static PhysicsManager* GetInstance();
		void Init();
		void Update(double deltaTime);
		void Shutdown();

		//uint32_t CreateIsland();
		//void DeleteIsland(uint32_t id);

		std::shared_ptr<Scene> CreateScene();
		void AddScene(const std::shared_ptr<Scene>& scene);
		void DeleteScene(const std::shared_ptr<Scene>& scene);

		//Scene* GetScene(uint32_t sceneID);
		
	private:
		PhysicsManager();

		void _ON_PAUSE(EventQueue<EngineEventType>::EventPtr e);

	private:
		// map of Scene ID -> Scene, where each scene is an instance of the physics of a game world
		LongMarch_Vector<std::shared_ptr<Scene>> m_scenes;

		bool m_paused;
	};
}
