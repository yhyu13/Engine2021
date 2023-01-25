#pragma once
#include "GameWorld.h"
#include "lgc.h"
#include "engine/ecs/components/ActiveCom.h"

namespace longmarch
{
    template <typename ComponentType>
    inline bool GameWorld::HasComponent(const Entity& entity) const
    {
        TRY_LOCK_READ();
        if (const auto it = m_entityMaskMap.find(entity);
            it != m_entityMaskMap.end())
        {
            return it->second.GetBitMask().IsAMatch(BitMaskSignature::Create<ComponentType>());
        }
        else
        {
            return false;
        }
    }

    // TODO @yuhang : implement Move component
    template <typename ComponentType>
    inline void GameWorld::AddComponent(const Entity& entity, const ComponentType& component)
    {
        TRY_LOCK_WRITE();
        auto& EntityMaskValue = m_entityMaskMap[entity];
        auto& newMask = EntityMaskValue.GetBitMask();
        auto& ComponentCache = EntityMaskValue.GetComponentCache();
        const auto oldMask = newMask;
        newMask.AddComponent<ComponentType>();

        if (oldMask == newMask)
        {
            ENGINE_EXCEPT(
                wStr(Str("Entity %s already has component type %s", Str(entity), typeid(ComponentType).name())));
            return;
        }

        const auto& oldManager = m_maskArcheTypeMap[oldMask];
        auto& newManager = m_maskArcheTypeMap[newMask];
        if (oldManager)
        {
            if (!newManager)
            {
                newManager = MemoryManager::Make_shared<ArcheTypeManager>();
                newManager->CopyComponentManagerFrom(*oldManager);
                newManager->AddComponentManger<ComponentType>();
            }
            // Transfer entity from old manager to new manager
            oldManager->MoveOutEntity(entity, *newManager);
        }
        else
        {
            ASSERT(oldMask == BitMaskSignature(),
                   "Fails to retrive proper bist mask for entity. OldManager is nullptr only if mask is empty.");
            if (!newManager)
            {
                newManager = MemoryManager::Make_shared<ArcheTypeManager>();
                newManager->AddComponentManger<ComponentType>();
            }
        }
        component.SetWorld(this);
        newManager->AddComponentToEntity(entity, component);
        ComponentCache.clear();
    }

    template <typename ComponentType>
    inline void GameWorld::RemoveComponent(const Entity& entity)
    {
        TRY_LOCK_WRITE();
        auto& EntityMaskValue = m_entityMaskMap[entity];
        auto& newMask = EntityMaskValue.GetBitMask();
        auto& ComponentCache = EntityMaskValue.GetComponentCache();
        const auto oldMask = newMask;
        newMask.RemoveComponent<ComponentType>();

        if (oldMask == newMask)
        {
            ENGINE_EXCEPT(
                wStr(Str("Entity %s already removed component type %s", Str(entity), typeid(ComponentType).name())));
            return;
        }

        const auto& oldManager = m_maskArcheTypeMap[oldMask];
        auto& newManager = m_maskArcheTypeMap[newMask];
        ASSERT(oldManager && newManager);
        // Transfer entity from old manager to new manager
        oldManager->MoveOutEntity(entity, *newManager);
        ComponentCache.clear();
    }

    template <typename ComponentType>
    inline ComponentDecorator<ComponentType> GameWorld::GetComponent(const Entity& entity) const
    {
        TRY_LOCK_READ();
        ComponentType* com = nullptr;
        if (const auto it = m_entityMaskMap.find(entity);
            it != m_entityMaskMap.end())
        {
            if (const auto& mask = it->second.GetBitMask();
                mask.IsAMatch(BitMaskSignature::Create<ComponentType>()))
            {
                auto& ComponentCache = it->second.GetComponentCache();
                auto ComTypeIndex = GetComponentTypeIndex<ComponentType>();
                if (const auto iter_com = ComponentCache.find(ComTypeIndex);
                    iter_com != ComponentCache.end())
                {
                    com = reinterpret_cast<ComponentType*>(iter_com->second);
                }
                else if (const auto iter_manager = m_maskArcheTypeMap.find(mask);
                    iter_manager != m_maskArcheTypeMap.end())
                {
                    if (const auto& manager = iter_manager->second;
                        manager)
                    {
                        com = manager->GetComponentByEntity<ComponentType>(entity);
                        ComponentCache[ComTypeIndex] = com;
                    }
                    else
                    {
                        ASSERT(mask == BitMaskSignature());
                    }
                }
            }
        }
        else
        {
            WARN_PRINT(
                Str("GameWorld::GetComponent: Fail to find Entity '%s' in world '%s'", Str(entity), this->GetName()));
        }
        return ComponentDecorator<ComponentType>(EntityDecorator{entity, this}, com);
    }

    template <class ...Components>
    inline const LongMarch_Vector<Entity> GameWorld::EntityView() const
    {
        if constexpr (sizeof...(Components) == 0)
        {
            // Should not reach this statement, double check EntityView argument
            ENGINE_EXCEPT(
                L"GameWorld::EntityView should not receive a trivial bit mask. Double check EntityView argument.");
            return LongMarch_Vector<Entity>();
        }
        else
        {
            return EntityView(BitMaskSignature::Create<Components...>());
        }
    }

    template <class ...Components>
    inline const LongMarch_Vector<EntityChunkContext> GameWorld::EntityChunkView() const
    {
        if constexpr (sizeof...(Components) == 0)
        {
            // Should not reach this statement, double check EntityView argument
            ENGINE_EXCEPT(
                L"GameWorld::EntityView should not receive a trivial bit mask. Double check EntityView argument.");
            return LongMarch_Vector<EntityChunkContext>();
        }
        else
        {
            return EntityChunkView(BitMaskSignature::Create<Components...>());
        }
    }

    //! Unity DOTS ECS like for each function
    template <class ...Components>
    inline void GameWorld::ForEach(
        const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func) const
    {
        TRY_LOCK_READ();
        for (const auto& e : EntityView<Components...>())
        {
            auto ed = EntityDecorator(e, this);
            func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
        }
    }

    //! Unity DOTS ECS like for each function (single worker thread)
    template <class ...Components>
    [[nodiscard]]
    inline auto GameWorld::BackEach(
        const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func) const
    {
        return StealThreadPool::GetInstance()->enqueue_task(
            [this, func]()
            {
                ENGINE_TRY_CATCH(this->ForEach<Components...>(func););
            }
        );
    }

    //! Unity DOTS ECS like for each function (multi worker thread)
    template <class ... Components>
    auto GameWorld::ParEach(
        const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func,
        int min_split) const
    {
        return StealThreadPool::GetInstance()->enqueue_task([this, min_split, func]()
        {
            ENGINE_TRY_CATCH(ParEach_Internal<Components...>(func, min_split););
        });
    }

    //! Helper method for pareach (defined in Gameworld.h)
    template <class... Components>
    void GameWorld::ParEach_Internal(
        const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func,
        int min_split) const
    {
        auto es = EntityView<Components...>();
        if (es.empty())
        {
            // Early return on empty entities
            return;
        }
        
        TRY_LOCK_READ();
        auto& pool = s_parEachJobPool;

        const int num_e = es.size();
        auto _begin = es.begin();
        auto _end = es.end();
        int split_size = num_e / pool.threads;
        
        // Minimum split size
        min_split = std::max(s_parEachMinSplit, min_split);
        split_size = std::max(split_size, min_split);
        
        // Even number split size
        if (split_size % 2 != 0)
        {
            ++split_size;
        }
        
        int num_e_left = num_e;
        LongMarch_Vector<std::future<void>> _jobs;

        while ((num_e_left -= split_size) > 0)
        {
            LongMarch_Vector<Entity> split_es(_begin, _begin + split_size);
            _begin += split_size;
            _jobs.emplace_back(pool.enqueue_task([this, &func, split_es = std::move(split_es)]()
            {
                ENGINE_TRY_CATCH(
                    {
                        LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                        for (const auto& e : split_es)
                        {
                            auto ed = EntityDecorator(e, this);
                            func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
                        }
                    });
            }));
        }
        
        // Check any entities left
        if (num_e_left <= 0)
        {
            split_size += num_e_left;
            LongMarch_Vector<Entity> split_es(_begin, _begin + split_size);
            ENGINE_EXCEPT_IF((_begin + split_size) != _end, L"Reach end condition does not meet!");
            _jobs.emplace_back(pool.enqueue_task([this, &func, split_es = std::move(split_es)]()
            {
                ENGINE_TRY_CATCH(
                    {
                        LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                        for (const auto& e : split_es)
                        {
                            auto ed = EntityDecorator(e, this);
                            func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
                        }
                    });
            }));
        }

        // Wait on jobs to finish
        for (auto& job : _jobs)
        {
            job.wait();
        }
    }
}
