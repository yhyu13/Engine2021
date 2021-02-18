#pragma once
#include "GameWorld.h"

namespace longmarch
{
	template<typename ComponentType>
	inline bool GameWorld::HasComponent(const Entity& entity)  const
	{
		ComponentManager<ComponentType>* manager = _GetComponentManager<ComponentType>();
		return manager->HasEntity(entity);
	}

	template<typename ComponentType>
	inline void GameWorld::AddComponent(const Entity& entity, const ComponentType& component)
	{
		ComponentManager<ComponentType>* manager = _GetComponentManager<ComponentType>();
		component.SetWorld(this);
		manager->AddComponentToEntity(entity, component);
		{
			LOCK_GUARD_NC();
			BitMaskSignature currentMask = m_entityMasks[entity];
			m_entityMasks[entity].AddComponent<ComponentType>(); // update the component-mask for the entity once a new component has been added
			_UpdateEntityForAllComponentSystems(entity, currentMask); // update the component-systems
		}
	}

	template<typename ComponentType>
	inline void GameWorld::RemoveComponent(const Entity& entity)
	{
		ComponentManager<ComponentType>* manager = _GetComponentManager<ComponentType>();
		if (manager->RemoveComponentFromEntity(entity))
		{
			LOCK_GUARD_NC();
			BitMaskSignature oldMask = m_entityMasks[entity];
			m_entityMasks[entity].RemoveComponent<ComponentType>();
			_UpdateEntityForAllComponentSystems(entity, oldMask);
		}
	}
	
	template<typename ComponentType>
	inline ComponentDecorator<ComponentType> GameWorld::GetComponent(const Entity& entity) const
	{
		ComponentManager<ComponentType>* manager = _GetComponentManager<ComponentType>();
		return ComponentDecorator<ComponentType>(EntityDecorator{ entity,this }, manager->GetComponentByEntity(entity));
	}

	template<class ...Components>
	inline const LongMarch_Vector<Entity> GameWorld::EntityView() const
	{
		LOCK_GUARD_NC();
		if constexpr (sizeof...(Components) == 0)
		{
			return LongMarch_Vector<Entity>();
		}
		else
		{
			LongMarch_Vector<Entity> ret;
			BitMaskSignature mask; mask.AddComponent<Components...>();
			for (const auto& [entity, entity_mask] : m_entityMasks)
			{
				if (entity_mask.IsAMatch(mask))
				{
					ret.emplace_back(entity);
				}
			}
			return ret;
		}
	}

	//! Unity ECS like for each function

	template<class ...Components>
	inline void GameWorld::ForEach(typename Identity<std::function<void(EntityDecorator e, Components&...)>>::Type func) const
	{
		for (const auto& e : EntityView<Components...>())
		{
			auto ed = EntityDecorator(e, this);
			func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
		}
	}

	//! Unity ECS like for each function (single worker thread), func is moved

	template<class ...Components>
	[[nodiscard]]
	inline auto GameWorld::BackEach(typename Identity<std::function<void(EntityDecorator e, Components&...)>>::Type func) const
	{
		return StealThreadPool::GetInstance()->enqueue_task([this, func = std::move(func)]() {
			_MultiThreadExceptionCatcher(
				[this, &func]() {
				for (const auto& e : EntityView<Components...>())
				{
					auto ed = EntityDecorator(e, this);
					func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
				}
			});
		});
	}

	//! Get the component-manager for a given component-type. Example usage: _GetComponentManager<ComponentType>();
	
	template<typename ComponentType>
	ComponentManager<ComponentType>* GameWorld::_GetComponentManager() const
	{
		const uint32_t family = GetComponentTypeIndex<ComponentType>();
		LockNC();
		if (family >= m_componentManagers.size())
		{
			m_componentManagers.resize(static_cast<size_t>(family) + 1);
		}
		auto& manager = m_componentManagers[family];
		if (!manager)
		{
			manager = std::move(MemoryManager::Make_shared<ComponentManager<ComponentType>>());
		}
		UnlockNC();
		return static_cast<ComponentManager<ComponentType>*>(manager.get());
	}

	//! Helper method for pareach

	template<class... Components>
	void GameWorld::_ParEach(typename Identity<std::function<void(EntityDecorator e, Components&...)>>::Type func, int min_split) const
	{
		try {
			auto es = EntityView<Components...>();
			if (es.empty())
			{
				return;
			}
			int num_e = es.size();
			auto& pool = s_pool_fine_grained;
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
				const LongMarch_Vector<Entity> split_es(_begin, _begin + split_size);
				_begin += split_size;
				_jobs.emplace_back(std::move(pool.enqueue_task([this, func, split_es = std::move(split_es)]() {
					_MultiThreadExceptionCatcher(
						[this, &func, &split_es]() {
						for (const auto& e : split_es)
						{
							auto ed = EntityDecorator(e, this);
							func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
						}
					});
				})));
			}
			if (num_e_left <= 0)
			{
				split_size += num_e_left;
				const LongMarch_Vector<Entity> split_es(_begin, _begin + split_size);
				_begin += split_size;
				ENGINE_EXCEPT_IF(_begin != _end, L"Reach end condition does not meet!");
				_jobs.emplace_back(std::move(pool.enqueue_task([this, func, split_es = std::move(split_es)]() {
					_MultiThreadExceptionCatcher(
						[this, &func, &split_es]() {
						for (const auto& e : split_es)
						{
							auto ed = EntityDecorator(e, this);
							func(ed, *(ed.template GetComponent<Components>().GetPtr())...);
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
		catch (std::exception& e) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception")); }
		catch (...) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception")); }
	}
}