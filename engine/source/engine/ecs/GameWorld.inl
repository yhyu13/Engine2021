#pragma once
#include "GameWorld.h"
#include "lgc.h"
#include "engine/ecs/components/ActiveCom.h"

namespace longmarch
{
    template <typename ComponentType>
    inline bool GameWorld::HasComponent(const Entity& entity) const
    {
        LOCK_GUARD_NC();
        if (const auto it = m_entityMaskMap.find(entity);
            it != m_entityMaskMap.end())
        {
            return it->second.IsAMatch(BitMaskSignature::Create<ComponentType>());
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
        LOCK_GUARD_NC();
        BitMaskSignature& newMask = m_entityMaskMap[entity];
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
                   "Fails to retrive proper bist mask for entity. OldManager is nullptr only if mask is empty.")
            if (!newManager)
            {
                newManager = MemoryManager::Make_shared<ArcheTypeManager>();
                newManager->AddComponentManger<ComponentType>();
            }
        }
        component.SetWorld(this);
        newManager->AddComponentToEntity(entity, component);
    }

    template <typename ComponentType>
    inline void GameWorld::RemoveComponent(const Entity& entity)
    {
        LOCK_GUARD_NC();
        BitMaskSignature& newMask = m_entityMaskMap[entity];
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
    }

    template <typename ComponentType>
    inline ComponentDecorator<ComponentType> GameWorld::GetComponent(const Entity& entity) const
    {
        LOCK_GUARD_NC();
        ComponentType* com = nullptr;
        if (const auto it = m_entityMaskMap.find(entity);
            it != m_entityMaskMap.end())
        {
            if (const auto& mask = it->second;
                mask.IsAMatch(BitMaskSignature::Create<ComponentType>()))
            {
                if (const auto iter_manager = m_maskArcheTypeMap.find(mask);
                    iter_manager != m_maskArcheTypeMap.end())
                {
                    if (const auto& manager = iter_manager->second;
                        manager)
                    {
                        com = manager->GetComponentByEntity<ComponentType>(entity);
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

    //! Unity ECS like for each function
    template <class ...Components>
    inline void GameWorld::ForEach(
        typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func) const
    {
        for (const auto& e : EntityView<Components...>())
        {
            if (auto activeCom = this->GetComponent<ActiveCom>(e); activeCom.Valid() && activeCom->IsActive())
            {
                auto ed = EntityDecorator(e, this);
                func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
            }
        }
    }

    //! Unity ECS like for each function (single worker thread), func is moved
    template <class ...Components>
    [[nodiscard]]
    inline auto GameWorld::BackEach(
        typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func) const
    {
        return StealThreadPool::GetInstance()->enqueue_task(
            [this, func = std::move(func)]()
            {
                this->_MultiThreadExceptionCatcher(
                    [this, &func]()
                    {
                        this->ForEach<Components...>(func);
                    }
                );
            }
        );
    }

    //! Helper method for pareach (defined in Gameworld.h)
    template <class... Components>
    void GameWorld::_ParEach(
        typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func,
        int min_split) const
    {
        try
        {
            auto& pool = s_parEach2Pool;
            auto es = EntityView<Components...>();
            if (es.empty())
            {
                // Early return on empty entities
                return;
            }
            int num_e = es.size();
            auto _begin = es.begin();
            auto _end = es.end();
            int split_size = num_e / pool.threads;
            // Minimum split size
            min_split = std::max(1, min_split);
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
                _jobs.emplace_back(std::move(pool.enqueue_task([this, &func, split_es = std::move(split_es)]()
                {
                    this->_MultiThreadExceptionCatcher(
                        [this, &func, &split_es]()
                        {
                            for (const auto& e : split_es)
                            {
                                if (auto activeCom = this->GetComponent<ActiveCom>(e); activeCom.Valid() && activeCom->
                                    IsActive())
                                {
                                    auto ed = EntityDecorator(e, this);
                                    func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
                                }
                            }
                        });
                })));
            }
            // Check any entities left
            if (num_e_left <= 0)
            {
                split_size += num_e_left;
                LongMarch_Vector<Entity> split_es(_begin, _begin + split_size);
                ENGINE_EXCEPT_IF((_begin + split_size) != _end, L"Reach end condition does not meet!");
                _jobs.emplace_back(std::move(pool.enqueue_task([this, &func, split_es = std::move(split_es)]()
                {
                    this->_MultiThreadExceptionCatcher(
                        [this, &func, &split_es]()
                        {
                            for (const auto& e : split_es)
                            {
                                if (auto activeCom = this->GetComponent<ActiveCom>(e); activeCom.Valid() && activeCom->
                                    IsActive())
                                {
                                    auto ed = EntityDecorator(e, this);
                                    func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
                                }
                            }
                        });
                })));
            }
            for (auto& job : _jobs)
            {
                job.wait();
            }
        }
        catch (EngineException& e) { EngineException::Push(std::move(e)); }
        catch (std::exception& e)
        {
            EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception"));
        }
        catch (...)
        {
            EngineException::Push(
                EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception"));
        }
    }
}
