#pragma once
#include "engine/EngineEssential.h"
#include "EntityDecorator.h"
#include "BitMaskSignature.h"
#include "ComponentDecorator.h"
#include "GameWorld.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

namespace longmarch
{
#ifndef EARLY_RETURN
#define  EARLY_RETURN(dt) {if (!dt) return; }
#endif // !EARLY_RETURN

#define RESERVE_SIZE 8

	/**
	 * @brief Systems are the places where the actual game-code goes. The entities and
			  components are simply data-storage units, systems actually work on that
			  data.
	 *
	 * Each system can specify which components it wishes to pay attention to.
		This information is stored in the form of a bit-mask signature. Entities
		which have the necessary components (entity's signatures matches the
		system's signature) will be registered with the system.

		Usage example:

		// declaring a component
		struct Position : longmarch::BaseComponent<Position> {
			Position() = default;
			Position(float x) : x(x) {};
			float x;
		};

		// declaring a system
		class Wind : public longmarch::BaseComponentSystem {
		public:
			Wind() {
				// wind system wants to pay attention to the Position component, thus creating the
				// signature that contains that information.
				m_systemSignature.AddComponent<Position>();
			}

			// actual game logic goes here
			virtual void Update(double frameTime) override {
			}
		};
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class BaseComponentSystem : public BaseAtomicClassNC, public BaseEventSubHandleClass
	{
	public:
		NONCOPYABLE(BaseComponentSystem);
		BaseComponentSystem()
		{
			m_bufferedRegisteredEntities.reserve(RESERVE_SIZE);
		}
		virtual ~BaseComponentSystem() = default;

		/*
			Override Init to register components and event call backs
		*/
		virtual void Init() {}

		virtual void Update(double frameTime) {}

		virtual void Update2(double frameTime) {}

		virtual void Update3(double frameTime) {}

		virtual void PreRenderUpdate(double dt) {}

		//! Can be used for debug rendering, it is called just before the main render pipeline
		virtual void Render() {}

		//! Can be used for debug rendering, it is called just after the main render pipeline
		virtual void Render2() {}

		virtual void PostRenderUpdate(double dt) {}

		virtual void RenderUI() {}

		virtual std::shared_ptr<BaseComponentSystem> Copy() const = 0;

#define COMSYS_DEFAULT_COPY(SystemType) \
		virtual std::shared_ptr<BaseComponentSystem> Copy() const override \
		{ \
		LOCK_GUARD_NC(); \
		auto ret = MemoryManager::Make_shared<SystemType>(); \
		ret->m_bufferedRegisteredEntities = m_bufferedRegisteredEntities; \
		ret->m_systemSignature = m_systemSignature; \
		return ret; \
		} \

		inline virtual void RemoveAllEntities()
		{
			LOCK_GUARD_NC();
			m_bufferedRegisteredEntities.clear();
		}

		inline void SetWorld(GameWorld* world) const
		{
			LOCK_GUARD_NC();
			m_parentWorld = world;
		}

		//! Dispatcher to parent world
		template<class ComponentType>
		ComponentDecorator<ComponentType> GetComponent(const Entity& entity)
		{
			return m_parentWorld->GetComponent<ComponentType>(entity);
		}

		//! Dispatcher to parent world
		template<class ComponentType>
		bool HasComponent(const Entity& entity)
		{
			return m_parentWorld->HasComponent<ComponentType>(entity);
		}

		//! Dispatcher to parent world
		__LongMarch_TRVIAL_TEMPLATE__
			inline void ForEach(typename Identity<std::function<void(EntityDecorator e)>>::Type func)
		{
			m_parentWorld->ForEach(GetRegisteredEntities(), func);
		}
		//! Dispatcher to parent world
		__LongMarch_TRVIAL_TEMPLATE__
			[[nodiscard]] inline std::future<void> BackEach(typename Identity<std::function<void(EntityDecorator e)>>::Type func)
		{
			return m_parentWorld->BackEach(GetRegisteredEntities(), func);
		}

		//! Dispatcher to parent world
		__LongMarch_TRVIAL_TEMPLATE__
			[[nodiscard]] inline std::future<void> ParEach(typename Identity<std::function<void(EntityDecorator e)>>::Type func, int min_split = -1)
		{
			return m_parentWorld->ParEach(GetRegisteredEntities(), func, min_split);
		}

		//! Get a copy of all registered engities because registered entities are subjected to change
		inline const LongMarch_Vector<Entity> GetRegisteredEntities() const
		{
			LOCK_GUARD_NC();
			return m_bufferedRegisteredEntities;
		}

		inline void AddEntity(const Entity& entity)
		{
			LOCK_GUARD_NC();
			m_bufferedRegisteredEntities.emplace_back(entity);
		}

		inline void RemoveEntity(const Entity& entity)
		{
			LOCK_GUARD_NC();
			// c++ 20 Uniform container ensured erase!
			std::erase(m_bufferedRegisteredEntities, entity);
			// Erase remove idiom
			/*if (!m_bufferedRegisteredEntities.empty())
			{
				m_bufferedRegisteredEntities.erase(std::remove(m_bufferedRegisteredEntities.begin(), m_bufferedRegisteredEntities.end(), entity), m_bufferedRegisteredEntities.end());
			}*/
		}

		inline BitMaskSignature& GetSystemSignature()
		{
			return m_systemSignature;
		}

	protected:
		LongMarch_Vector<Entity> m_bufferedRegisteredEntities;
		BitMaskSignature m_systemSignature;
		mutable GameWorld* m_parentWorld{ nullptr };
	};
#undef RESERVE_SIZE
}
