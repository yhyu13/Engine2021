#pragma once
#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

namespace AAAAgames
{
#define RESERVE_SIZE 8

	class GameWorld;

	template<typename ComponentType>
	struct ComponentData
	{
		ComponentData()
		{
			m_data.reserve(RESERVE_SIZE);
		}
		A4GAMES_Vector<ComponentType> m_data;
	};

	struct EntityData
	{
		EntityData()
		{
			m_data.reserve(RESERVE_SIZE);
		}
		A4GAMES_Vector<Entity> m_data;
	};

	class BaseComponentManager : public BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(BaseComponentManager);
		BaseComponentManager() = default;
		virtual ~BaseComponentManager() = default;
	public:
		virtual BaseComponentInterface* GetBaseComponentByEntity(const Entity& entity) const = 0;
		virtual bool RemoveComponentFromEntity(const Entity& entity) = 0;
		virtual bool HasEntity(const Entity& entity) const = 0;
		virtual std::shared_ptr<BaseComponentManager> Copy() const = 0;
		virtual void SetWorld(GameWorld* world) const = 0; //!< Set the gameworld for all managed components
	};

	/**
	 * @brief Component manager brings the entities and their corresponding components together.
		Each component-type gets a component-manager.

	 * @detail Instead of making entities own their components, it is much more cache efficient
		to store all components of a type at a single location. This ensures that when
		systems run their update methods, they access all required components of a type
		from a contiguous memory.
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	template<typename ComponentType>
	class ComponentManager : public BaseComponentManager
	{
	public:
		NONCOPYABLE(ComponentManager);
		ComponentManager() = default;

		//! Never store a pointer to a component because it might subject to change upon resize
		ComponentType* GetComponentByEntity(const Entity& entity) const
		{
			LOCK_GUARD_NC();
			if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
			{
				return const_cast<ComponentType*>(&(m_components.m_data[it->second]));
			}
			else
			{
				return nullptr;
			}
		}

		void AddComponentToEntity(const Entity& entity, const ComponentType& component)
		{
			if (!HasEntity(entity))
			{
				LOCK_GUARD_NC();
				uint32_t index = m_components.m_data.size();
				m_components.m_data.emplace_back(component);
				m_entities.m_data.emplace_back(entity);
				m_entitiesAndComponentIndexes.emplace(entity, index);
			}
		}

		virtual bool HasEntity(const Entity& entity) const override
		{
			LOCK_GUARD_NC();
			return m_entitiesAndComponentIndexes.contains(entity);
		}

		virtual BaseComponentInterface* GetBaseComponentByEntity(const Entity& entity) const override
		{
			return static_cast<BaseComponentInterface*>(GetComponentByEntity(entity));
		}

		/*
			Remove component from an entity.
			Every time we remove an entity, we update the component list by moving
			the last component-instance in the list to take the place of the removed
			component instance.
			Return true on removing an existing entity.
			Return false on that entity does not exist
		*/
		virtual bool RemoveComponentFromEntity(const Entity& entity) override
		{
			LOCK_GUARD_NC();
			if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
			{
				uint32_t index = it->second;
				uint32_t lastIndex = m_components.m_data.size() - 1;

				// First, make the last entity points the current index. And remove the current entity
				m_entitiesAndComponentIndexes[m_entities.m_data[lastIndex]] = index;
				m_entitiesAndComponentIndexes.erase(entity);

				// Move the component data from last index to the index of the component data just removed
				std::swap(m_components.m_data[index], m_components.m_data[lastIndex]);
				m_components.m_data.pop_back();

				// Swap the current entity with the last entity
				std::swap(m_entities.m_data[index], m_entities.m_data[lastIndex]);
				m_entities.m_data.pop_back();

				return true;
			}
			return false;
		}

		virtual std::shared_ptr<BaseComponentManager> Copy() const
		{
			LOCK_GUARD_NC();
			auto ret = MemoryManager::Make_shared<ComponentManager>();
			ret->m_components = m_components;
			ret->m_entities = m_entities;
			ret->m_entitiesAndComponentIndexes = m_entitiesAndComponentIndexes;
			return ret;
		}

		virtual void SetWorld(GameWorld* world) const
		{
			for (auto& com : m_components.m_data)
			{
				com.SetWorld(world);
			}
		}

	private:
		// Stores all the component instances in an array
		ComponentData<ComponentType> m_components;
		// Stores all entities indexed by the index of the component instance in m_components
		EntityData m_entities;
		// Maps the entity to the index of the component instance in the m_components
		A4GAMES_UnorderedMap_Par_flat<Entity, uint32_t> m_entitiesAndComponentIndexes;
	};
#undef RESERVE_SIZE
}
