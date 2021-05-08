#pragma once
#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
#define RESERVE_SIZE 2048

	class GameWorld;

	class BaseComponentManager : public BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(BaseComponentManager);
		BaseComponentManager() = default;
		virtual ~BaseComponentManager() = default;
	public:
		virtual BaseComponentInterface* GetBaseComponentByEntity(const Entity& entity) const = 0;
		virtual bool RemoveComponentFromEntity(const Entity& entity) = 0;
		virtual void ReorderComponentFromEntity(const Entity& entity) = 0;
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
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	template<typename ComponentType>
	class ComponentManager : public BaseComponentManager
	{
	public:
		NONCOPYABLE(ComponentManager);
		ComponentManager()
		{
			m_components.reserve(RESERVE_SIZE);
			m_entities.reserve(RESERVE_SIZE);
		}

		//! Never store a pointer to a component because it might subject to change upon resize
		ComponentType* GetComponentByEntity(const Entity& entity) const
		{
			LOCK_GUARD_NC();
			if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
			{
				return const_cast<ComponentType*>(&(m_components[it->second]));
			}
			else
			{
				return nullptr;
			}
		}

		bool AddComponentToEntity(const Entity& entity, const ComponentType& component)
		{
			if (!HasEntity(entity))
			{
				LOCK_GUARD_NC();
				uint32_t index = m_components.size();
				m_components.emplace_back(component);
				m_entities.emplace_back(entity);
				m_entitiesAndComponentIndexes.emplace(entity, index);
				return true;
			}
			else
			{
				return false;
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

				// First, make the last entity points the current index. And remove the current entity
				m_entitiesAndComponentIndexes[m_entities.back()] = index;
				m_entitiesAndComponentIndexes.erase(it);

				// Move the component data from last index to the index of the component data just removed
				std::swap(m_components[index], m_components.back());
				m_components.pop_back();

				// Swap the current entity with the last entity
				std::swap(m_entities[index], m_entities.back());
				m_entities.pop_back();

				return true;
			}
			return false;
		}

		virtual void ReorderComponentFromEntity(const Entity& entity) override
		{
			LOCK_GUARD_NC();
			if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
			{
				uint32_t index = it->second;

				// First, make the last entity points the current index. 
				std::swap(m_entitiesAndComponentIndexes[m_entities.back()], m_entitiesAndComponentIndexes[entity]);

				// Move the component data from last index to the index of the component data
				std::swap(m_components[index], m_components.back());

				// Swap the current entity with the last entity
				std::swap(m_entities[index], m_entities.back());
			}
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
			for (auto& com : m_components)
			{
				com.SetWorld(world);
			}
		}

	private:
		// Stores all the component instances in an array
		LongMarch_Vector<ComponentType> m_components;
		// Stores all entities indexed by the index of the component instance in m_components
		LongMarch_Vector<Entity> m_entities;
		// Maps the entity to the index of the component instance in the m_components
		LongMarch_UnorderedMap_flat<Entity, uint32_t> m_entitiesAndComponentIndexes;
	};

#undef RESERVE_SIZE
}

