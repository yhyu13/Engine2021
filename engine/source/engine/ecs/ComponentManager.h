#pragma once

#pragma optimize("", off)

#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/allocator/TemplateMemoryManager.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

#define RESERVE_SIZE 64
#define NUM_COMPONENTS_PER_CHUNK 64
#define CHUNK_INDEX(x) ((x) / NUM_COMPONENTS_PER_CHUNK)
#define COM_INDEX(x) ((x) % NUM_COMPONENTS_PER_CHUNK)

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
        virtual bool HasIndex(size_t index, size_t& o_chunk_index, size_t& o_com_index) const = 0;
        virtual size_t Size() const = 0;
        virtual void ExpandSize(size_t newSize) = 0;

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
        struct ComponentChunk
        {
            // Component chunk storing a fixed number of elements, additionally, storing the index of the last available element
            ComponentChunk() = default;
            ComponentChunk(const ComponentChunk&) = default;

            std::array<ComponentType, NUM_COMPONENTS_PER_CHUNK> chunk;
            size_t last_index{NUM_COMPONENTS_PER_CHUNK - 1};
        };

        NONCOPYABLE(ComponentManager);
        ComponentManager() = default;

        //! ComponentType* is local and temporal because it might subject to change upon adding/removing elements happens in the component manager internally
        // So do not store ComponentType* across frames
        [[nodiscard]] ComponentType* GetComponentByIndex(size_t index) const
        {
            size_t chunk_index, com_index;
            if (HasIndex(index, chunk_index, com_index))
            {
                return const_cast<ComponentType*>(&_GetComponentInChunkByIndex(chunk_index, com_index));
            }
            else
            {
                return nullptr;
            }
        }

        void AddComponentToIndex(size_t index, const ComponentType& component)
        {
            size_t chunk_index, com_index;
            ASSERT(HasIndex(index, chunk_index, com_index));
            _GetComponentInChunkByIndex(chunk_index, com_index) = component;
        }

        void MoveComponentToIndex(size_t index, ComponentType&& component)
        {
            size_t chunk_index, com_index;
            ASSERT(HasIndex(index, chunk_index, com_index));
            _GetComponentInChunkByIndex(chunk_index, com_index) = std::move(component);
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
            size_t chunk_index, com_index;
            if (HasIndex(index, chunk_index, com_index))
            {
                // Swap between current chunk with last chunk
                auto& chunk = m_componentChunks[chunk_index];
                auto& last_chunk = m_componentChunks.back();
                std::iter_swap(chunk->chunk.begin() + com_index, last_chunk->chunk.begin() + last_chunk->last_index);
                if (last_chunk->last_index-- == 0)
                {
                    m_componentChunks.pop_back();
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        virtual bool HasIndex(size_t index, size_t& o_chunk_index, size_t& o_com_index) const override
        {
            if (o_chunk_index = CHUNK_INDEX(index);
                o_chunk_index < m_componentChunks.size())
            {
                o_com_index = COM_INDEX(index);
                return o_com_index <= m_componentChunks[o_chunk_index]->last_index;
            }
            else
            {
                return false;
            }
        }

        virtual size_t Size() const override
        {
            size_t ret = 0;
            for (const auto& chunk : m_componentChunks)
            {
                ret += (chunk->last_index + 1);
            }
            return ret;
        }

        virtual void ExpandSize(size_t newSize) override
        {
            auto chunk_index = CHUNK_INDEX(newSize - 1);
            auto com_index = COM_INDEX(newSize - 1);
            if (m_componentChunks.empty())
            [[unlikely]]
            {
                ASSERT(chunk_index == 0);
                auto chunk = TemplateMemoryManager<ComponentChunk>::Make_shared();
                chunk->last_index = com_index;
                m_componentChunks.emplace_back(std::move(chunk));
            }
            else
            [[likely]]
            {
                auto last_available_chunk_index = m_componentChunks.size() - 1;
                ASSERT(chunk_index >= last_available_chunk_index);
                for (int i = (chunk_index - last_available_chunk_index); i > 0; --i)
                {
                    auto chunk = TemplateMemoryManager<ComponentChunk>::Make_shared();
                    m_componentChunks.emplace_back(std::move(chunk));
                }
                m_componentChunks[chunk_index]->last_index = com_index;
            }
        }

        virtual std::shared_ptr<BaseComponentManager> Copy() const override
        {
            auto ret = MemoryManager::Make_shared<ComponentManager>();
            for (const auto& chunk : m_componentChunks)
            {
                ret->m_componentChunks.emplace_back(
                    std::move(TemplateMemoryManager<ComponentChunk>::Make_shared(*chunk)));
            }
            return ret;
        }

        virtual std::shared_ptr<BaseComponentManager> Create() const override
        {
            return MemoryManager::Make_shared<ComponentManager>();
        }

        virtual void SetWorld(GameWorld* world) const override
        {
            for (auto& chunk : m_componentChunks)
            {
                for (size_t i = 0; i <= chunk->last_index; ++i)
                {
                    chunk->chunk[i].SetWorld(world);
                }
            }
        }

    private:
        [[nodiscard]] ComponentType& _GetComponentInChunkByIndex(size_t chunk_index, size_t com_index) const
        {
            return m_componentChunks[chunk_index]->chunk[com_index];
        }


    private:
        friend class ArcheTypeManager;
        // Stores all the component instances in an array
        LongMarch_Vector<std::shared_ptr<ComponentChunk>> m_componentChunks;
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

                ASSERT([=](){size_t _1; size_t _2; return manager->HasIndex(index,_1,_2);}(),
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
                ASSERT([=](){size_t _1; size_t _2; return manager->HasIndex(index,_1,_2);}(),
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
            ASSERT(m_entities.size() == manager->Size());
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

            if (shouldSwapBack)
            {
                m_entitiesAndComponentIndexes[m_entities.back()] = index;
            }
            m_entitiesAndComponentIndexes.erase(entity);

            if (shouldSwapBack)
            {
                std::iter_swap(m_entities.begin() + index, m_entities.end() - 1);
            }
            m_entities.pop_back();

            for (auto& [_, comManager] : m_componentManagers)
            {
                comManager->RemoveComponentByIndex(index);
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
#undef NUM_COMPONENTS_PER_CHUNK 64
#undef CHUNK_INDEX
#undef COM_INDEX


#pragma optimize("", on)
