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

    class BaseComponentManager
    {
    public:
        NONCOPYABLE(BaseComponentManager);
        BaseComponentManager() = default;
        virtual ~BaseComponentManager() = default;

    public:
        virtual void MoveBaseComponentByIndex(size_t index, BaseComponentInterface* component) = 0;
        virtual BaseComponentInterface* GetBaseComponentByIndex(size_t index) const = 0;
        virtual bool RemoveComponentByIndex(size_t index) = 0;
        virtual bool HasIndex(size_t index) const = 0;
        virtual size_t Size() const = 0;
        virtual void ExpandSize(size_t newSize) = 0;

        virtual void SwapBack(size_t index) = 0;
        virtual void PopBack() = 0;

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
        }

        //! ComponentType* is local and temporals because it might subject to change upon vector reallocation happens in the component manager internally
        ComponentType* GetComponentByIndex(size_t index) const
        {
            if (HasIndex(index))
            {
                return const_cast<ComponentType*>(&m_components[index]);
            }
            else
            {
                return nullptr;
            }
        }

        void AddComponentToIndex(size_t index, const ComponentType& component)
        {
            ASSERT(HasIndex(index));
            m_components[index] = component;
        }

        void MoveComponentToIndex(size_t index, ComponentType&& component)
        {
            ASSERT(HasIndex(index));
            m_components[index] = std::move(component);
        }

        virtual void MoveBaseComponentByIndex(size_t index, BaseComponentInterface* component) override
        {
            auto com = static_cast<ComponentType*>(component);
            ASSERT(com);
            MoveComponentToIndex(index, std::move(*com));
        }

        virtual BaseComponentInterface* GetBaseComponentByIndex(size_t index) const override
        {
            return static_cast<BaseComponentInterface*>(GetComponentByIndex(index));
        }

        virtual bool RemoveComponentByIndex(size_t index) override
        {
            if (HasIndex(index))
            {
                if (index != m_components.size() - 1)
                {
                    std::iter_swap(m_components.begin() + index, m_components.end() - 1);
                }
                m_components.pop_back();
                return true;
            }
            else
            {
                return false;
            }
        }

        virtual bool HasIndex(size_t index) const override
        {
            return index < m_components.size();
        }

        virtual size_t Size() const override
        {
            return m_components.size();
        }

        virtual void ExpandSize(size_t newSize) override
        {
            ASSERT(newSize >= m_components.size());
            m_components.resize(newSize);
        }

        virtual void SwapBack(size_t index) override
        {
            ASSERT(HasIndex(index));
            std::iter_swap(m_components.begin() + index, m_components.end() - 1);
        }

        virtual void PopBack() override
        {
            m_components.pop_back();
        }

        virtual std::shared_ptr<BaseComponentManager> Copy() const override
        {
            auto ret = MemoryManager::Make_shared<ComponentManager>();
            ret->m_components = m_components;
            return ret;
        }

        virtual std::shared_ptr<BaseComponentManager> Create() const override
        {
            return MemoryManager::Make_shared<ComponentManager>();
        }

        virtual void SetWorld(GameWorld* world) const override
        {
            for (auto& com : m_components)
            {
                com.SetWorld(world);
            }
        }

    private:
        friend class ArcheTypeManager;
        // Stores all the component instances in an array
        LongMarch_Vector<ComponentType> m_components;
    };

    /**
     * @brief Archetype is equivalent to a collection of entities with the same set of components.
     * The ArcheTypeManager manages push and pop of every entity of certain archetype on adding/removing components

     * @detail The ArcheTypeManager maintains the list of entities and their underlying indices 
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class ArcheTypeManager
    {
    public:
        NONCOPYABLE(ArcheTypeManager);

        ArcheTypeManager()
        {
            m_entities.reserve(RESERVE_SIZE);
            m_entitiesAndComponentIndexes.reserve(RESERVE_SIZE);
        }

        void CopyComponentManagerFrom(const ArcheTypeManager& other)
        {
            ASSERT(this->m_entities.empty());
            ASSERT(this->m_entitiesAndComponentIndexes.empty());
            ASSERT(this->m_componentManagers.empty());

            for (const auto& [comType, comManger] : other.m_componentManagers)
            {
                this->m_componentManagers[comType] = comManger->Create();
            }
        }

        template <typename ComponentType>
        void AddComponentManger()
        {
            ASSERT(this->m_entities.empty());
            ASSERT(this->m_entitiesAndComponentIndexes.empty());
            auto type = GetComponentTypeIndex<ComponentType>();
            ASSERT(!m_componentManagers.contains(type));
            m_componentManagers[type] = MemoryManager::Make_shared<ComponentManager<ComponentType>>();
        }

        [[nodiscard]] BaseComponentInterface* GetBaseComponentByEntity(const Entity& entity,
                                                                       ComponentTypeIndex_T comTypeIndex) const
        {
            size_t index;
            if (_HasEntity(entity, index))
            {
                ASSERT(m_componentManagers.contains(comTypeIndex));
                const auto& manager = m_componentManagers.at(comTypeIndex);
                ASSERT(manager, Str("ArcheType should already contain %ud", comTypeIndex));
                ASSERT(manager->HasIndex(index),
                       Str("ArcheType component manager %ud should already contain index %ull", comTypeIndex, index));
                return manager->GetBaseComponentByIndex(index);
            }
            else
            {
                ENGINE_EXCEPT(wStr(L"Entity %s is not registered", wStr(entity).c_str()));
                return nullptr;
            }
        }

        template <typename ComponentType>
        [[nodiscard]] ComponentType* GetComponentByEntity(const Entity& entity) const
        {
            size_t index;
            if (_HasEntity(entity, index))
            {
                ASSERT(m_componentManagers.contains(GetComponentTypeIndex<ComponentType>()));
                auto manager = static_cast<ComponentManager<ComponentType>*>(m_componentManagers.at(
                    GetComponentTypeIndex<ComponentType>()).get());
                ASSERT(manager, Str("ArcheType should already contain %s", typeid(ComponentType).name()));
                ASSERT(manager->HasIndex(index),
                       Str("ArcheType component manager %s should already contain index %ull", typeid(ComponentType).
                           name(), index));
                return manager->GetComponentByIndex(index);
            }
            else
            {
                ENGINE_EXCEPT(wStr(L"Entity %s is not registered", wStr(entity).c_str()));
                return nullptr;
            }
        }

        template <typename ComponentType>
        void AddComponentToEntity(const Entity& entity, const ComponentType& component)
        {
            auto manager = static_cast<ComponentManager<ComponentType>*>(m_componentManagers[GetComponentTypeIndex<
                ComponentType>()].get());
            ASSERT(manager, Str("ArcheType should already contain %s", typeid(ComponentType).name()));
            size_t index;
            if (!_HasEntity(entity, index))
            {
                index = m_entities.size();
                m_entities.push_back(entity);
                m_entitiesAndComponentIndexes[entity] = index;
            }
            manager->ExpandSize(index + 1);
            manager->AddComponentToIndex(index, component);
            ASSERT(m_entities.size() == manager->m_components.size());
        }

        template <typename ComponentType>
        void MoveComponentToEntity(const Entity& entity, ComponentType&& component)
        {
            auto manager = static_cast<ComponentManager<ComponentType>*>(m_componentManagers[GetComponentTypeIndex<
                ComponentType>()].get());
            ASSERT(manager, Str("ArcheType should already contain %s", typeid(ComponentType).name()));
            size_t index;
            if (!_HasEntity(entity, index))
            {
                index = m_entities.size();
                m_entities.push_back(entity);
                m_entitiesAndComponentIndexes[entity] = index;
            }
            manager->ExpandSize(index + 1);
            manager->MoveComponentToIndex(index, component);
            ASSERT(m_entities.size() == manager->m_components.size());
        }

        void MoveOutEntity(const Entity& entity, ArcheTypeManager& other)
        {
            // Sanity checks
            ASSERT(this->m_componentManagers.size() != other.m_componentManagers.size());
            size_t index;
            ASSERT(!other._HasEntity(entity, index), Str("Cannot move in a managed entity %s", Str(entity)));
            ASSERT(this->_HasEntity(entity, index), Str("Cannot move out a unmanaged entity %s", Str(entity)));

            // Emplace entity to the other manager
            const size_t emplaceIndex = other._AddEntityAndExpandComponentArray(entity);

            if (const bool updateBasedOnThis = m_componentManagers.size() < other.m_componentManagers.size();
                updateBasedOnThis)
            {
                for (const auto& [comType, comManger] : this->m_componentManagers)
                {
                    ASSERT(other.m_componentManagers.contains(comType),
                           Str("Component type %ud not contained by other", comType));
                    other.m_componentManagers[comType]->MoveBaseComponentByIndex(
                        emplaceIndex, comManger->GetBaseComponentByIndex(index));
                }
            }
            else
            {
                for (auto& [comType, comManger] : other.m_componentManagers)
                {
                    ASSERT(this->m_componentManagers.contains(comType),
                           Str("Component type %ud not contained by other", comType));
                    comManger->MoveBaseComponentByIndex(emplaceIndex,
                                                        this->m_componentManagers[comType]->GetBaseComponentByIndex(
                                                            index));
                }
            }
            // Destroy entity for this manager
            this->_RemoveEntityAtIndex(entity, index);
        }

        void RemoveEntity(const Entity& entity)
        {
            size_t index;
            ASSERT(this->_HasEntity(entity, index), Str("Cannot remove a unmanaged entity %s", Str(entity)));
            // Destroy entity for this manager
            this->_RemoveEntityAtIndex(entity, index);
        }

        [[nodiscard]] bool HasEntity(const Entity& entity) const
        {
            size_t _;
            return _HasEntity(entity, _);
        }

        [[nodiscard]] const LongMarch_Vector<Entity>& GetEntityView() const
        {
            return m_entities;
        }

        [[nodiscard]] std::shared_ptr<ArcheTypeManager> Copy() const
        {
            auto ret = MemoryManager::Make_shared<ArcheTypeManager>();

            ret->m_entities = m_entities;
            ret->m_entitiesAndComponentIndexes = m_entitiesAndComponentIndexes;
            for (const auto& [comType, comManager] : m_componentManagers)
            {
                ret->m_componentManagers[comType] = comManager->Copy();
            }
            return ret;
        }

        void SetWorld(GameWorld* world) const
        {
            for (const auto& [_, comManger] : m_componentManagers)
            {
                comManger->SetWorld(world);
            }
        }

    private:
        [[nodiscard]] size_t _AddEntityAndExpandComponentArray(const Entity& entity)
        {
            size_t index;
            ASSERT(!_HasEntity(entity, index));
            // Push back entity
            index = m_entities.size();
            m_entities.push_back(entity);
            m_entitiesAndComponentIndexes[entity] = index;
            for (auto& [_, comManager] : m_componentManagers)
            {
                // Asset entity length matches component list size
                ASSERT(index == comManager->Size());
                // Expand component array by 1 with uninitialized value
                comManager->ExpandSize(index + 1);
            }
            return index;
        }

        void _RemoveEntityAtIndex(const Entity& entity, size_t index)
        {
            ASSERT(index < m_entities.size());
            const bool shouldSwapBack = index != (m_entities.size() - 1);

            auto lastEntity = m_entities.back();
            if (shouldSwapBack)
            [[likely]]
            {
                std::iter_swap(m_entities.begin() + index, m_entities.end() - 1);
            }
            m_entities.pop_back();

            if (shouldSwapBack)
            [[likely]]
            {
                m_entitiesAndComponentIndexes[lastEntity] = index;
            }
            m_entitiesAndComponentIndexes.erase(entity);

            for (const auto& [comType, comManager] : m_componentManagers)
            {
                if (shouldSwapBack)
                [[likely]]
                {
                    comManager->SwapBack(index);
                }
                comManager->PopBack();
            }
        }

        [[nodiscard]] bool _HasEntity(const Entity& entity, size_t& index) const
        {
            if (const auto it = m_entitiesAndComponentIndexes.find(entity);
                it != m_entitiesAndComponentIndexes.end())
            {
                index = it->second;
                return true;
            }
            else
            {
                return false;
            }
        }

    private:
        // Stores all entities indexed by the index of the component instance in m_components
        LongMarch_Vector<Entity> m_entities;
        // Maps the entity to the index of the component instance in the m_components
        LongMarch_UnorderedMap_Par_node<Entity, size_t> m_entitiesAndComponentIndexes;
        // Contains all component managers which are indexed by component indices
        LongMarch_UnorderedMap_node<ComponentTypeIndex_T, std::shared_ptr<BaseComponentManager>> m_componentManagers;
    };
}

#undef RESERVE_SIZE
