#pragma once
#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

#ifndef _SHIPPING
#define RESERVE_SIZE 1 // w/o shipping, we need to test reallocation of component array
#else
#define RESERVE_SIZE 64
#endif

namespace longmarch
{
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
        virtual bool HasEntity(const Entity& entity) const = 0;

        //! Copy/Clone all data from component manager to a new one
        virtual std::shared_ptr<BaseComponentManager> Copy() const = 0;
        //! Create new component manager of the same component type with empty data
        virtual std::shared_ptr<BaseComponentManager> Create() const = 0;
        //! Set the gameworld for all managed components
        virtual void SetWorld(GameWorld* world) const = 0;
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
    template <typename ComponentType>
    class ComponentManager : public BaseComponentManager
    {
    public:
        NONCOPYABLE(ComponentManager);

        ComponentManager()
        {
            m_components.reserve(RESERVE_SIZE);
            m_entities.reserve(RESERVE_SIZE);
            m_entitiesAndComponentIndexes.reserve(RESERVE_SIZE);
        }

        //! ComponentType* is local and temporals because it might subject to change upon vector reallocation happens in the component manager internally
        ComponentType* GetComponentByEntity(const Entity& entity) const
        {
            LOCK_GUARD_NC();
            if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
            {
                return const_cast<ComponentType*>(&m_components[it->second]);
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
                int index = m_components.size();
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

        virtual bool RemoveComponentFromEntity(const Entity& entity) override
        {
            LOCK_GUARD_NC();
            if (auto it = m_entitiesAndComponentIndexes.find(entity); it != m_entitiesAndComponentIndexes.end())
            {
                auto index = it->second;
                auto lastEntity = m_entities.back();
                m_entitiesAndComponentIndexes[lastEntity] = index;
                m_entitiesAndComponentIndexes.erase(it);

                std::iter_swap(m_components.begin() + index, m_components.end() - 1);
                m_components.pop_back();
                std::iter_swap(m_entities.begin() + index, m_entities.end() - 1);
                m_entities.pop_back();

                return true;
            }
            else
            {
                return false;
            }
        }

        virtual std::shared_ptr<BaseComponentManager> Copy() const override
        {
            auto ret = MemoryManager::Make_shared<ComponentManager>();
            LOCK_GUARD_NC();
            ret->m_components = m_components;
            ret->m_entities = m_entities;
            ret->m_entitiesAndComponentIndexes = m_entitiesAndComponentIndexes;
            return ret;
        }

        virtual std::shared_ptr<BaseComponentManager> Create() const override
        {
            return MemoryManager::Make_shared<ComponentManager>();
        }

        virtual void SetWorld(GameWorld* world) const override
        {
            LOCK_GUARD_NC();
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
        LongMarch_UnorderedMap_Par_node<Entity, int> m_entitiesAndComponentIndexes;
    };

    /**
     * @brief Archetype is equivalent to a collection of entities with the same set of components.
     * The ArcheTypeManager manages push and pop of every entity of certain archetype on adding/removing components

     * @detail The ArcheTypeManager maintains the list of entities and their underlying indices 
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class ArcheTypeManager : public BaseAtomicClassNC
    {
    public:
        // yuhang TODO : Implement
        // TODO (1) adding entity
        // TODO (2) delete entity
        // TODO (3) move entity out of this manager to another manager
        // TODO (4) move entity from another manager to this manager
        // TODO (5) various sanity check

        bool AddEntity(const Entity& entity)
        {
            return false;
        }

        std::shared_ptr<ArcheTypeManager> Copy() const
        {
            return nullptr;
        }

        void SetWorld(GameWorld* world) const
        {
        }

    private:
        // Stores all entities indexed by the index of the component instance in m_components
        LongMarch_Vector<Entity> m_entities;
        // Maps the entity to the index of the component instance in the m_components
        LongMarch_UnorderedMap_flat<Entity, int> m_entitiesAndComponentIndexes;
        // Contains all component managers which are indexed by component indices
        LongMarch_UnorderedMap_flat<ComponentIndexType, std::shared_ptr<BaseComponentManager>> m_componentManagers;
    };
}

#undef RESERVE_SIZE
