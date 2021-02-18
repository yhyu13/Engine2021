#include "engine-precompiled-header.h"
#include "GameWorld.h"
#include "BaseComponentSystem.h"
#include "engine/ecs/components/ActiveCom.h"
#include "engine/ecs/components/ParentCom.h"
#include "engine/ecs/components/ChildrenCom.h"
#include "engine/ecs/components/IDNameCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/ecs/object-factory/ObjectFactory.h"

longmarch::GameWorld::GameWorld(bool setCurrent, const std::string& name, const fs::path& filePath)
	:
	m_entityManager(std::move(MemoryManager::Make_shared<longmarch::EntityManager>())),
	m_name((name.empty()) ? filePath.filename().string() : name)
{
	if (setCurrent)
	{
		SetCurrent(this);
	}
	InitSystemAndScene(FileSystem::ResolveProtocol(filePath));
}

GameWorld* longmarch::GameWorld::GetInstance(bool setCurrent, const std::string& name, const fs::path& filePath)
{
	auto worldName = (name.empty()) ? filePath.filename().string() : name;
	auto ptr = LongMarch_Unique_ptr<GameWorld>(new GameWorld(setCurrent, worldName, filePath));
	{
		LOCK_GUARD_S();
		auto& world = allManagedWorlds[worldName];
		world = std::move(ptr);
		return world.get();
	}
}

[[nodiscard]] std::future<GameWorld*> longmarch::GameWorld::GetInstanceAsync(bool setCurrent, const std::string& name, const fs::path& filePath)
{
	return StealThreadPool::GetInstance()->enqueue_task(
		[setCurrent, name, filePath]()->GameWorld* { return GetInstance(setCurrent, name, filePath); }
	);
}

void longmarch::GameWorld::SetCurrent(GameWorld* world)
{
	LOCK_GUARD_S();
	currentWorld = world;
}

GameWorld* longmarch::GameWorld::GetCurrent()
{
	LOCK_GUARD_S();
	return currentWorld;
}

GameWorld* longmarch::GameWorld::GetManagedWorldByName(const std::string& name)
{
	LOCK_GUARD_S();
	if (auto it = allManagedWorlds.find(name); it != allManagedWorlds.end())
	{
		return it->second.get();
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(name) + L" is not a managed world!");
		return nullptr;
	}
}

const LongMarch_Vector<std::string> longmarch::GameWorld::GetAllManagedWorldNames()
{
	LOCK_GUARD_S();
	LongMarch_Vector<std::string> ret;
	LongMarch_MapKeyToVec(allManagedWorlds, ret);
	return ret;
}

void longmarch::GameWorld::RemoveManagedWorld(const std::string& name)
{
	LOCK_GUARD_S();
	if (auto it = allManagedWorlds.find(name); it != allManagedWorlds.end())
	{
		if (it->second.get() == currentWorld)
		{
			currentWorld = nullptr;
		}
		allManagedWorlds.erase(it);
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(name) + L" is not a managed world!");
	}
}

void longmarch::GameWorld::RemoveManagedWorld(GameWorld* world)
{
	LOCK_GUARD_S();
	bool found = false;
	for (auto it = allManagedWorlds.begin(); it != allManagedWorlds.end(); ++it)
	{
		if (it->second.get() == world)
		{
			found = true;
			if (world == currentWorld)
			{
				currentWorld = nullptr;
			}
			allManagedWorlds.erase(it);
		}
	}
	if (!found)
	{
		ENGINE_EXCEPT(str2wstr(world->GetName()) + L" is not a managed world!");
	}
}

GameWorld* longmarch::GameWorld::Clone(const std::string& newName, const std::string& name)
{
	ASSERT(newName != name, "Clone world must have a different name!");
	auto newWorld = GetInstance(false, newName, "");
	auto from = GetManagedWorldByName(name);
	{
		from->LockNC();
		{
			// E
			newWorld->m_entityManager = from->m_entityManager->Copy();
			newWorld->m_entityMasks = from->m_entityMasks;

			// C
			newWorld->m_componentManagers.clear();
			newWorld->m_componentManagers.resize(from->m_componentManagers.size());
			for (auto i(0u); i < from->m_componentManagers.size(); ++i)
			{
				if (auto& manager = from->m_componentManagers[i]; manager) //!< For game world that does not contain any entity that use some components, the component manager might be nullptr
				{
					auto copy = manager->Copy();
					copy->SetWorld(newWorld);
					newWorld->m_componentManagers[i] = std::move(copy);
				}
			}

			// S
			newWorld->m_systemsName = from->m_systemsName;
			newWorld->m_systems.clear();
			for (auto& system : from->m_systems)
			{
				auto copy = system->Copy();
				copy->SetWorld(newWorld);
				newWorld->m_systems.emplace_back(std::move(copy));
			}
			ASSERT(newWorld->m_systems.size() == newWorld->m_systemsName.size(), "System pointer and name vector must have the same size!");

			newWorld->m_systemsMap.clear();
			for (auto i(0u); i < newWorld->m_systemsName.size(); ++i)
			{
				newWorld->m_systemsMap[newWorld->m_systemsName[i]] = newWorld->m_systems[i];
			}
		}
		from->UnlockNC();
	}
	newWorld->Init();
	return newWorld;
}

GameWorld* longmarch::GameWorld::Clone(const std::string& newName, GameWorld* from)
{
	ASSERT(newName != from->GetName(), "Clone world must have a different name!");
	auto newWorld = GetInstance(false, newName, "");
	{
		from->LockNC();
		{
			// E
			newWorld->m_entityManager = from->m_entityManager->Copy();
			newWorld->m_entityMasks = from->m_entityMasks;

			// C
			newWorld->m_componentManagers.clear();
			newWorld->m_componentManagers.resize(from->m_componentManagers.size());
			for (auto i(0u); i < from->m_componentManagers.size(); ++i)
			{
				if (auto& manager = from->m_componentManagers[i]; manager) //!< For game world that does not contain any entity that use some components, the component manager might be nullptr
				{
					auto copy = manager->Copy();
					copy->SetWorld(newWorld);
					newWorld->m_componentManagers[i] = std::move(copy);
				}
			}

			// S

			newWorld->m_systemsName = from->m_systemsName;
			newWorld->m_systems.clear();
			for (auto& system : from->m_systems)
			{
				auto copy = system->Copy();
				copy->SetWorld(newWorld);
				newWorld->m_systems.emplace_back(std::move(copy));
			}
			ASSERT(newWorld->m_systems.size() == newWorld->m_systemsName.size(), "System pointer and name vector must have the same size!");

			newWorld->m_systemsMap.clear();
			for (auto i(0u); i < newWorld->m_systemsName.size(); ++i)
			{
				newWorld->m_systemsMap[newWorld->m_systemsName[i]] = newWorld->m_systems[i];
			}
		}
		from->UnlockNC();
	}
	newWorld->Init();
	return newWorld;
}

void longmarch::GameWorld::InitSystem(const fs::path& system_file)
{
	ASSERT(m_systems.empty(), "GameWorld component system is not empty before init!");
	ObjectFactory::s_instance->LoadSystems(system_file, this);
}

void longmarch::GameWorld::InitScene(const fs::path& scene_file)
{
	ASSERT(!m_systems.empty(), "GameWorld component system is empty after init!");
	ObjectFactory::s_instance->LoadGameWorldScene(scene_file, this);
}

//! Helper function for adding/removing entity from component system

void longmarch::GameWorld::_UpdateEntityForAllComponentSystems(const Entity& entity, BitMaskSignature& oldMask)
{
	// Do not place thread lock here!
	BitMaskSignature updatedMask = m_entityMasks[entity];
	for (auto&& system : m_systems) {
		BitMaskSignature systemSignature = system->GetSystemSignature();
		if (updatedMask.IsNewMatch(oldMask, systemSignature))
		{
			system->AddEntity(entity);
		}
		else if (updatedMask.IsNoLongerMatched(oldMask, systemSignature))
		{
			system->RemoveEntity(entity);
		}
	}
}

void longmarch::GameWorld::InitSystemAndScene(const fs::path& _file)
{
	if (fs::exists(_file))
	{
		InitSystem(_file);
		InitScene(_file);
		Init();
	}
}

void longmarch::GameWorld::Init() {
	ENGINE_EXCEPT_IF(m_systems.empty(), L"Calling Init() on an empty GameWorld!");
	for (auto&& system : m_systems)
	{
		system->SetWorld(this);
		system->Init();
	}
}

void longmarch::GameWorld::Update(double frameTime)
{
	if (m_paused) frameTime = 0.0;
	for (auto&& system : m_systems)
	{
		system->Update(frameTime);
	}
}

void longmarch::GameWorld::Update2(double frameTime)
{
	if (m_paused) frameTime = 0.0;
	for (auto&& system : m_systems)
	{
		system->Update2(frameTime);
	}
}

void longmarch::GameWorld::Update3(double frameTime)
{
	if (m_paused) frameTime = 0.0;
	for (auto&& system : m_systems)
	{
		system->Update3(frameTime);
	}
}

void longmarch::GameWorld::MultiThreadUpdate(double frameTime)
{
	m_jobs.emplace_back(
		std::move(StealThreadPool::GetInstance()->enqueue_task([frameTime, this]()
			{
				_MultiThreadExceptionCatcher([frameTime, this]()
					{
						Update(frameTime);
						Update2(frameTime);
						Update3(frameTime);
					});
			})
		));
}

void longmarch::GameWorld::MultiThreadJoin()
{
	while (!m_jobs.empty())
	{
		m_jobs.back().wait();
		m_jobs.pop_back();
	}
}

void longmarch::GameWorld::PreRenderUpdate(double frameTime)
{
	if (m_paused) frameTime = 0.0;
	for (auto&& system : m_systems)
	{
		system->PreRenderUpdate(frameTime);
	}
}

void longmarch::GameWorld::Render(double frameTime)
{
	for (auto&& system : m_systems)
	{
		system->Render();
	}
}

void longmarch::GameWorld::Render2(double frameTime)
{
	for (auto&& system : m_systems)
	{
		system->Render2();
	}
}

void longmarch::GameWorld::PostRenderUpdate(double frameTime)
{
	if (m_paused) frameTime = 0.0;
	for (auto&& system : m_systems)
	{
		system->PostRenderUpdate(frameTime);
	}
}

void longmarch::GameWorld::RenderUI()
{
	for (auto&& system : m_systems)
	{
		system->RenderUI();
	}
}

/**************************************************************
*	Remover
**************************************************************/

void longmarch::GameWorld::InactivateHelper(Entity e)
{
	if (auto com = GetComponent<ActiveCom>(e); com.Valid())
	{
		com->SetActive(false);
	}
}

void longmarch::GameWorld::RemoveFromParentHelper(Entity e)
{
	if (auto parentCom = GetComponent<ParentCom>(e); parentCom.Valid())
	{
		if (auto com = GetComponent<ChildrenCom>(parentCom->GetParent()); com.Valid())
		{
			com->RemoveEntity(e);
		}
	}
}

void longmarch::GameWorld::RemoveAllEntities()
{
	for (auto&& system : m_systems)
	{
		system->RemoveAllEntities();
	}
	m_entityMasks.clear();
}

void longmarch::GameWorld::RemoveAllComponentSystems()
{
	m_systems.clear();
	m_systemsName.clear();
	m_systemsMap.clear();
}

EntityDecorator longmarch::GameWorld::GenerateEntity(EntityType type, bool active, bool add_to_root)
{
	auto entity = EntityDecorator{ m_entityManager->Create(type), this };
	entity.AddComponent(ActiveCom(active));
	entity.AddComponent(IDNameCom(entity));
	entity.AddComponent(ParentCom(entity));
	entity.AddComponent(ChildrenCom(entity));
	if (add_to_root)
	{
		auto root = GetTheOnlyEntityWithType((EntityType)(EngineEntityType::SCENE_ROOT));
		AddChildHelper(root, entity);
	}
	return entity;
}

EntityDecorator longmarch::GameWorld::GenerateEntity3D(EntityType type, bool active, bool add_to_root)
{
	auto entity = GenerateEntity(type, active, add_to_root);
	entity.AddComponent(Transform3DCom(entity));
	entity.AddComponent(Scene3DCom(entity));
	entity.AddComponent(Body3DCom(entity));
	return entity;
}

EntityDecorator longmarch::GameWorld::GenerateEntity3DNoCollision(EntityType type, bool active, bool add_to_root)
{
	auto entity = GenerateEntity(type, active, add_to_root);
	entity.AddComponent(Transform3DCom(entity));
	entity.AddComponent(Scene3DCom(entity));
	return entity;
}

void longmarch::GameWorld::AddChildHelper(Entity parent, Entity child)
{
	RemoveFromParentHelper(child);
	if (auto com = GetComponent<ChildrenCom>(parent); com.Valid())
	{
		com->AddEntity(child);
	}
}

void longmarch::GameWorld::RegisterSystem(const std::shared_ptr<BaseComponentSystem>& system, const std::string& name) {
	LOCK_GUARD_NC();
	system->SetWorld(this);
	m_systems.emplace_back(system);
	m_systemsName.emplace_back(name);
	m_systemsMap[name] = system;
}

BaseComponentSystem* longmarch::GameWorld::GetComponentSystem(const std::string& name)
{
	if (auto it = m_systemsMap.find(name); it != m_systemsMap.end())
	{
		return it->second.get();
	}
	else
	{
		return nullptr;
	}
}

const LongMarch_Vector<std::string> longmarch::GameWorld::GetAllComponentSystemName()
{
	return m_systemsName;
}

const LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> longmarch::GameWorld::GetAllComponentSystem()
{
	return m_systems;
}

const LongMarch_Vector<std::pair<std::string, std::shared_ptr<BaseComponentSystem>>> longmarch::GameWorld::GetAllComponentSystemNamePair()
{
	ASSERT(m_systemsName.size() == m_systems.size(), "Component system sizes do not match!");
	LongMarch_Vector<std::pair<std::string, std::shared_ptr<BaseComponentSystem>>> ret;
	for (int i(0); i < m_systemsName.size(); ++i)
	{
		ret.emplace_back(m_systemsName[i], m_systems[i]);
	}
	return ret;
}

void longmarch::GameWorld::RemoveEntity(const Entity& entity)
{
	m_entityManager->Destroy(entity);
}

void longmarch::GameWorld::RemoveEntityAndComponents(const Entity& entity)
{
	RemoveAllComponent(entity);
	RemoveEntity(entity);
}

const Entity longmarch::GameWorld::GetTheOnlyEntityWithType(EntityType type) const
{
	auto es = GetAllEntityWithType(type);
	if (es.size() == 1)
	{
		return es[0];
	}
	else
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(type) + L" should have one entity but there is " + wStr(es.size()));
	}
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(EntityType type) const
{
	return GetAllEntityWithType(LongMarch_Vector<EntityType>({ type }));
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(const std::initializer_list<EntityType>& types) const
{
	return GetAllEntityWithType(LongMarch_Vector<EntityType>(types));
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(const LongMarch_Vector<EntityType>& types) const
{
	LOCK_GUARD_NC();
	LongMarch_Vector<Entity> result;
	for (auto& type : types)
	{
		for (auto& id : m_entityManager->GetAllEntityIDWithType(type))
		{
			result.emplace_back(id, type);
		}
	}
	return result;
}

const Entity longmarch::GameWorld::GetEntityFromID(EntityID ID) const
{
	return m_entityManager->GetEntityFromID(ID);
}

const LongMarch_Vector<BaseComponentInterface*> longmarch::GameWorld::GetAllComponent(const Entity& entity) const
{
	LOCK_GUARD_NC();
	LongMarch_Vector<BaseComponentInterface*> ret;
	for (auto& family : m_entityMasks.at(entity).GetAllComponentIndex())
	{
		ENGINE_EXCEPT_IF(family >= m_componentManagers.size(), L"Entity " + str2wstr(Str(entity)) + L"is requesting all components before some component managers are initilaized!");
		ret.emplace_back(m_componentManagers[family]->GetBaseComponentByEntity(entity));
	}
	return ret;
}

void longmarch::GameWorld::RemoveAllComponent(const Entity& entity)
{
	LOCK_GUARD_NC();
	BitMaskSignature oldMask = m_entityMasks[entity];
	for (auto& family : m_entityMasks[entity].GetAllComponentIndex())
	{
		ENGINE_EXCEPT_IF(family >= m_componentManagers.size(), L"Entity " + str2wstr(Str(entity)) + L"is requesting all components before some component managers are initilaized!");
		m_componentManagers[family]->RemoveComponentFromEntity(entity);
	}
	// Clear bit mask and remove entity from component systems
	m_entityMasks[entity].Reset();
	_UpdateEntityForAllComponentSystems(entity, oldMask);
}

void longmarch::GameWorld::ForEach(const LongMarch_Vector<Entity>& es, typename Identity<std::function<void(EntityDecorator e)>>::Type func) const
{
	for (const auto& e : es)
	{
		func(EntityDecorator(e, this));
	}
}

[[nodiscard]]
std::future<void> longmarch::GameWorld::BackEach(const LongMarch_Vector<Entity>& es, typename Identity<std::function<void(EntityDecorator e)>>::Type func) const
{
	return StealThreadPool::GetInstance()->enqueue_task([this, func = std::move(func), es]() {
		_MultiThreadExceptionCatcher(
			[this, &func, &es]() {
				for (const auto& e : es)
				{
					func(EntityDecorator(e, this));
				}
			});
	});
}

[[nodiscard]]
std::future<void> longmarch::GameWorld::ParEach(const LongMarch_Vector<Entity>& es, typename Identity<std::function<void(EntityDecorator e)>>::Type func, int min_split) const
{
	return StealThreadPool::GetInstance()->enqueue_task([this, es, min_split, func = std::move(func)]() { _ParEach2(es, func, min_split); });
}

void longmarch::GameWorld::_ParEach2(const LongMarch_Vector<Entity>& es, typename Identity<std::function<void(EntityDecorator e)>>::Type func, int min_split) const
{
	try {
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
					[this, &func, &split_es]()
					{
						for (const auto& e : split_es)
						{
							func(EntityDecorator(e, this));
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
					[this, &func, &split_es]()
					{
						for (const auto& e : split_es)
						{
							func(EntityDecorator(e, this));
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