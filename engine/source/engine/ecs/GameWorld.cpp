#include "engine-precompiled-header.h"
#include "GameWorld.h"
#include "BaseComponentSystem.h"
#include "engine/ecs/components/ActiveCom.h"
#include "engine/ecs/components/ParentCom.h"
#include "engine/ecs/components/ChildrenCom.h"
#include "engine/ecs/components/TypeNameCom.h"
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

void GameWorld::Init()
{
    auto queue = EventQueue<EngineEventType>::GetInstance();
    {
        queue->Subscribe(EngineEventType::ENG_WINDOW_QUIT, &GameWorld::_ON_ENG_WINDOW_QUIT);
    }
}

void GameWorld::CleanUp()
{
    // Join all jobs
    for (auto& [name, world] : s_allManagedWorlds)
    {
        world->MultiThreadJoin();
    }
    // Remove all worlds
    s_allManagedWorlds.clear();
}

GameWorld* longmarch::GameWorld::GetInstance(bool setCurrent, const std::string& name, const fs::path& filePath)
{
    auto worldName = (name.empty()) ? filePath.filename().string() : name;
    auto ptr = RefPtr<GameWorld>::Allocator::New(setCurrent, worldName, filePath);
    {
        LOCK_GUARD_S();
        auto& world = s_allManagedWorlds[worldName];
        world = std::move(RefPtr<GameWorld>(ptr));
        return world.get();
    }
}

[[nodiscard]] std::future<GameWorld*> longmarch::GameWorld::GetInstanceAsync(
    bool setCurrent, const std::string& name, const fs::path& filePath)
{
    return StealThreadPool::GetInstance()->enqueue_task(
        [setCurrent, name, filePath]()-> GameWorld*
        {
            return GetInstance(setCurrent, name, filePath);
        }
    );
}

void longmarch::GameWorld::SetCurrent(GameWorld* world)
{
    LOCK_GUARD_S();
    s_currentWorld = world;
}

GameWorld* longmarch::GameWorld::GetCurrent()
{
    LOCK_GUARD_S();
    return s_currentWorld;
}

GameWorld* longmarch::GameWorld::GetWorldByName(const std::string& name)
{
    LOCK_GUARD_S();
    if (auto it = s_allManagedWorlds.find(name); it != s_allManagedWorlds.end())
    {
        return it->second.get();
    }
    else
    {
        ENGINE_EXCEPT(wStr(name) + L" is not a managed world!");
        return nullptr;
    }
}

const LongMarch_Vector<std::string> longmarch::GameWorld::GetAllManagedWorldNames()
{
    LOCK_GUARD_S();
    LongMarch_Vector<std::string> ret;
    LongMarch_MapKeyToVec(s_allManagedWorlds, ret);
    return ret;
}

bool longmarch::GameWorld::RemoveWorld(const std::string& name)
{
    LOCK_GUARD_S();
    if (auto it = s_allManagedWorlds.find(name); it != s_allManagedWorlds.end())
    {
        if (it->second.get() == s_currentWorld)
        {
            s_currentWorld = nullptr;
        }
        s_allManagedWorlds.erase(it);
        return true;
    }
    else
    {
        ENGINE_EXCEPT(wStr(name) + L" is not a managed world!");
        return false;
    }
}

bool longmarch::GameWorld::RemoveWorld(GameWorld* world)
{
    LOCK_GUARD_S();
    bool found = false;
    for (auto it = s_allManagedWorlds.begin(); it != s_allManagedWorlds.end();)
    {
        if (it->second.get() == world)
        {
            found = true;
            if (world == s_currentWorld)
            {
                s_currentWorld = nullptr;
            }
            it = s_allManagedWorlds.erase(it);
        }
        ++it;
    }
    if (!found)
    {
        ENGINE_EXCEPT(wStr(world->GetName()) + L" is not a managed world!");
        return false;
    }
    return true;
}

GameWorld* longmarch::GameWorld::Clone(const std::string& newWorldName, const std::string& fromWorldName)
{
    ASSERT(newWorldName != fromWorldName, "Clone world must have a different name!");
    const auto fromWorld = GetWorldByName(fromWorldName);
    auto newWorld = Clone(newWorldName, fromWorld);
    return newWorld;
}

GameWorld* longmarch::GameWorld::Clone(const std::string& newName, const GameWorld* from)
{
    ASSERT(newName != from->GetName(), "Clone world must have a different name!");
    auto newWorld = GetInstance(false, newName, "");
    {
        from->LockNC();
        {
            // (E) Copy entities
            newWorld->m_entityManager = from->m_entityManager->Copy();
            newWorld->m_entityMaskMap = from->m_entityMaskMap;

            // (C) Copy components
            newWorld->m_maskArcheTypeMap.clear();
            newWorld->m_maskArcheTypeMap.reserve(from->m_maskArcheTypeMap.size());
            for (const auto& [mask, manager] : from->m_maskArcheTypeMap)
            {
                //!< For game world that does not contain any entity that use some components, the component manager might be nullptr
                if (manager)
                {
                    auto copy = manager->Copy();
                    copy->SetWorld(newWorld);
                    newWorld->m_maskArcheTypeMap[mask] = std::move(copy);
                }
            }

            // (S) Copy systems
            newWorld->m_systemsName = from->m_systemsName;
            newWorld->m_systems.clear();
            for (auto& system : from->m_systems)
            {
                auto copy = system->Copy();
                copy->SetWorld(newWorld);
                newWorld->m_systems.emplace_back(std::move(copy));
            }
            ASSERT(newWorld->m_systems.size() == newWorld->m_systemsName.size(),
                   "System pointer and fromWorldName vector must have the same size!");

            newWorld->m_systemsNameMap.clear();
            for (auto i(0u); i < newWorld->m_systemsName.size(); ++i)
            {
                newWorld->m_systemsNameMap[newWorld->m_systemsName[i]] = newWorld->m_systems[i];
            }
        }
        from->UnLockNC();
    }
    newWorld->InitECS();
    return newWorld;
}

void GameWorld::_ON_ENG_WINDOW_QUIT(EventQueue<EngineEventType>::EventPtr e)
{
    CleanUp();
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

void longmarch::GameWorld::InitSystemAndScene(const fs::path& _file)
{
    if (fs::exists(_file))
    {
        InitSystem(_file);
        InitScene(_file);
        InitECS();
    }
}

void longmarch::GameWorld::InitECS()
{
    ENGINE_EXCEPT_IF(m_systems.empty(), L"Calling Init() on an empty GameWorld!");
    for (auto& system : m_systems)
    {
        system->SetWorld(this);
        system->Init();
    }
}

void longmarch::GameWorld::Update(double frameTime)
{
    if (m_paused) frameTime = 0.0;
    for (auto& system : m_systems)
    {
        system->Update(frameTime);
    }
}

void longmarch::GameWorld::LateUpdate(double frameTime)
{
    if (m_paused) frameTime = 0.0;
    for (auto& system : m_systems)
    {
        system->LateUpdate(frameTime);
    }
}

void longmarch::GameWorld::MultiThreadUpdate(double frameTime)
{
    m_jobs.push(s_jobPool.enqueue_task([frameTime, this]()
    {
        ENGINE_TRY_CATCH(
            {
            Update(frameTime);
            LateUpdate(frameTime);
            }
        );
    }));
}

void longmarch::GameWorld::MultiThreadJoin()
{
    while (!m_jobs.empty())
    {
        m_jobs.front().wait();
        m_jobs.pop();
    }
}

void longmarch::GameWorld::PreRenderUpdate(double frameTime)
{
    if (m_paused) frameTime = 0.0;
    for (auto& system : m_systems)
    {
        system->PreRenderUpdate(frameTime);
    }
}

void longmarch::GameWorld::PreRenderPass(double frameTime)
{
    for (auto& system : m_systems)
    {
        system->PreRenderPass();
    }
}

void longmarch::GameWorld::PostRenderPass(double frameTime)
{
    for (auto& system : m_systems)
    {
        system->PostRenderPass();
    }
}

void longmarch::GameWorld::PostRenderUpdate(double frameTime)
{
    if (m_paused) frameTime = 0.0;
    for (auto& system : m_systems)
    {
        system->PostRenderUpdate(frameTime);
    }
}

void longmarch::GameWorld::RenderUI()
{
    for (auto& system : m_systems)
    {
        system->RenderUI();
    }
}

/**************************************************************
*	Remover
**************************************************************/

void longmarch::GameWorld::InactivateEntity_Helper(Entity e)
{
    if (auto com = GetComponent<ActiveCom>(e); com.Valid())
    {
        com->SetActive(false);
    }
}

void longmarch::GameWorld::RemoveFromParent_Helper(Entity e)
{
    if (auto parentCom = GetComponent<ParentCom>(e); parentCom.Valid())
    {
        if (auto com = GetComponent<ChildrenCom>(parentCom->GetParent()); com.Valid())
        {
            com->RemoveEntity(e);
        }
    }
}

EntityDecorator longmarch::GameWorld::GenerateEntity(EntityType type, bool active, bool add_to_root)
{
    EntityDecorator entity;
    {
        LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{1});
        LOCK_GUARD_NC();
        entity = EntityDecorator{m_entityManager->Create(type), this};
    }
    entity.Volatile().AddComponent(ActiveCom(active));
    entity.Volatile().AddComponent(IDNameCom(entity));
    entity.Volatile().AddComponent(ParentCom(entity));
    entity.Volatile().AddComponent(ChildrenCom(entity));
    if (add_to_root)
    {
        auto root = GetTheOnlyEntityWithType((EntityType)(EngineEntityType::SCENE_ROOT));
        AddChild_Helper(root, entity);
    }
    return entity;
}

EntityDecorator longmarch::GameWorld::GenerateEntity3D(EntityType type, bool active, bool add_to_root)
{
    auto entity = GenerateEntity(type, active, add_to_root);
    entity.Volatile().AddComponent(Transform3DCom(entity));
    entity.Volatile().AddComponent(Scene3DCom(entity));
    entity.Volatile().AddComponent(Body3DCom(entity));
    return entity;
}

EntityDecorator longmarch::GameWorld::GenerateEntity3DNoCollision(EntityType type, bool active, bool add_to_root)
{
    auto entity = GenerateEntity(type, active, add_to_root);
    entity.Volatile().AddComponent(Transform3DCom(entity));
    entity.Volatile().AddComponent(Scene3DCom(entity));
    return entity;
}

bool longmarch::GameWorld::HasEntity(const Entity& entity) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});
    return LongMarch_contains(m_entityMaskMap, entity);
}

void longmarch::GameWorld::AddChild_Helper(Entity parent, Entity child)
{
    RemoveFromParent_Helper(child);
    if (auto com = GetComponent<ChildrenCom>(parent); com.Valid())
    {
        com->AddEntity(child);
    }
}

void longmarch::GameWorld::RegisterSystem(const std::shared_ptr<BaseComponentSystem>& system, const std::string& name)
{
    system->SetWorld(this);
    m_systems.emplace_back(system);
    m_systemsName.emplace_back(name);
    m_systemsNameMap[name] = system;
}

BaseComponentSystem* longmarch::GameWorld::GetComponentSystem(const std::string& name)
{
    if (auto it = m_systemsNameMap.find(name); it != m_systemsNameMap.end())
    {
        return it->second.get();
    }
    else
    {
        return nullptr;
    }
}

const LongMarch_Vector<std::string> longmarch::GameWorld::GetAllComponentSystemName() const
{
    return m_systemsName;
}

const LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> longmarch::GameWorld::GetAllComponentSystem() const
{
    return m_systems;
}

const LongMarch_Vector<std::pair<std::string, std::shared_ptr<BaseComponentSystem>>>
longmarch::GameWorld::GetAllComponentSystemNamePair() const
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
    {
        // Remove components first
        RemoveAllComponent(entity);
    }

    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{1});
    // Then remove the entity
    LOCK_GUARD_NC();
    m_entityManager->Destroy(entity);
    const auto it = m_entityMaskMap.find(entity);
    ASSERT(it != m_entityMaskMap.end());
    m_entityMaskMap.erase(it);
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
        throw EngineException(
            _CRT_WIDE(__FILE__), __LINE__, wStr(type) + L" should have one entity but there is " + wStr(es.size()));
    }
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(EntityType type) const
{
    return GetAllEntityWithType(LongMarch_Vector<EntityType>({type}));
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(
    const std::initializer_list<EntityType>& types) const
{
    return GetAllEntityWithType(LongMarch_Vector<EntityType>(types));
}

const LongMarch_Vector<Entity> longmarch::GameWorld::GetAllEntityWithType(
    const LongMarch_Vector<EntityType>& types) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});

    LongMarch_Vector<Entity> result;
    for (auto& type : types)
    {
        auto entities = m_entityManager->GetAllEntitiesWithType(type);
        std::ranges::copy(entities.begin(), entities.end(),
                          std::back_inserter(result));
    }
    return result;
}

const Entity longmarch::GameWorld::GetEntityFromID(EntityID ID) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});
    return m_entityManager->GetEntityFromID(ID);
}

const LongMarch_Vector<BaseComponentInterface*> longmarch::GameWorld::GetAllComponent(const Entity& entity) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});
    LongMarch_Vector<BaseComponentInterface*> ret;
    if (auto it = m_entityMaskMap.find(entity);
        it != m_entityMaskMap.end())
    {
        const auto& mask = it->second.GetBitMask();
        if (const auto iter_manager = m_maskArcheTypeMap.find(mask);
            iter_manager != m_maskArcheTypeMap.end())
        {
            if (const auto& manager = iter_manager->second;
                manager)
            {
                const auto& allComType = mask.GetAllComponentTypeIndex();
                ret.reserve(allComType.size());
                for (auto comType : allComType)
                {
                    ret.push_back(manager->GetBaseComponentByEntity(entity, comType));
                }
            }
            else
            {
                ASSERT(mask == BitMaskSignature());
            }
        }
    }
    else
    {
        WARN_PRINT(
            Str("GameWorld::GetComponent: Fail to find Entity '%s' in world '%s'", Str(entity), this->GetName()));
    }
    return ret;
}

void longmarch::GameWorld::RemoveAllComponent(const Entity& entity)
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{1});
    LOCK_GUARD_NC();
    if (auto it = m_entityMaskMap.find(entity);
        it != m_entityMaskMap.end())
    {
        auto& mask = it->second.GetBitMask();
        if (auto& manager = m_maskArcheTypeMap[mask];
            manager)
        {
            manager->RemoveEntity(entity);
        }
        mask.Reset();
    }
}

const LongMarch_Vector<Entity> GameWorld::EntityView(const BitMaskSignature& mask) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});

    ENGINE_EXCEPT_IF(mask == BitMaskSignature(),
                     L"GameWorld::EntityView should not receive a trivial bit mask. Double check EntityView argument.");
    LongMarch_Vector<Entity> ret;
    ret.reserve(256);
    for (const auto& [comTypes, manager] : m_maskArcheTypeMap)
    {
        if (manager && comTypes.IsAMatch(mask))
        {
            const auto& entities = manager->GetEntityView();
            std::ranges::copy(entities.begin(), entities.end(), std::back_inserter(ret));
        }
    }
    return ret;
}

void longmarch::GameWorld::ForEach(const LongMarch_Vector<Entity>& es,
                                   const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func)
const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
    for (const auto& e : es)
    {
        func(EntityDecorator(e, this));
    }
}

[[nodiscard]]
std::future<void> longmarch::GameWorld::BackEach(const LongMarch_Vector<Entity>& es,
                                                 const std::type_identity_t<std::function<void
                                                     (const EntityDecorator& e)>>&
                                                 func) const
{
    return StealThreadPool::GetInstance()->enqueue_task(
        [this, es, func]()
        {
            ENGINE_TRY_CATCH(this->ForEach(es, func););
        }
    );
}

[[nodiscard]]
std::future<void> longmarch::GameWorld::ParEach(const LongMarch_Vector<Entity>& es,
                                                const std::type_identity_t<std::function<void
                                                    (const EntityDecorator& e)>>&
                                                func, int min_split) const
{
    return StealThreadPool::GetInstance()->enqueue_task([this, es, min_split, func]()
    {
        ENGINE_TRY_CATCH(ParEach_Internal(es, func, min_split););
    });
}

void longmarch::GameWorld::ParEach_Internal(const LongMarch_Vector<Entity>& es,
                                    const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func,
                                    int min_split) const
{
    if (es.empty())
    {
        // Early return on empty entities
        return;
    }

    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
    auto& pool = s_parEachJobPool;

    // Init splitting parameters
    const int num_e = es.size();
    auto _begin = es.begin();
    auto _end = es.end();
    int split_size = num_e / pool.threads;

    // Adjust minimum split size & set actual split size
    min_split = std::max(s_parEachMinSplit, min_split);
    split_size = std::max(split_size, min_split);

    // Even number split size
    if (split_size % 2 != 0)
    {
        ++split_size;
    }

    // Ready for splitting jobs
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
                    this->ForEach(split_es, func);
                    }
                );
            }
        ));
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
                    this->ForEach(split_es, func);
                    }
                );
            }
        ));
    }

    // Wait for each job to finish
    for (auto& job : _jobs)
    {
        job.wait();
    }
}

const LongMarch_Vector<EntityChunkContext> GameWorld::EntityChunkView(const BitMaskSignature& mask) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0});

    ENGINE_EXCEPT_IF(mask == BitMaskSignature(),
                     L"GameWorld::EntityChunkView should not receive a trivial bit mask. Double check EntityChunkView argument.");
    LongMarch_Vector<EntityChunkContext> ret;
    for (const auto& [comTypes, manager] : m_maskArcheTypeMap)
    {
        if (manager && comTypes.IsAMatch(mask))
        {
            for (size_t i = 0; i < manager->NumOfChunks(); ++i)
            {
                ret.emplace_back(manager.get(), i);
            }
        }
    }
    return ret;
}

void GameWorld::ForEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
                             const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>& func) const
{
    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
    for (const auto& e : es)
    {
        func(e);
    }
}

std::future<void> GameWorld::BackEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
                                           const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>&
                                           func) const
{
    return StealThreadPool::GetInstance()->enqueue_task(
        [this, es, func]()
        {
            ENGINE_TRY_CATCH(this->ForEachChunk(es, func););
        }
    );
}

std::future<void> GameWorld::ParEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
                                          const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>&
                                          func, int min_split) const
{
    return StealThreadPool::GetInstance()->enqueue_task([this, es, min_split, func]()
    {
        ENGINE_TRY_CATCH(ParEachChunk_Internal(es, func, min_split););
    });
}

void GameWorld::ParEachChunk_Internal(const LongMarch_Vector<EntityChunkContext>& es,
                              const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>& func,
                              int min_split) const
{
    if (es.empty())
    {
        // Early return on empty entities
        return;
    }

    LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
    auto& pool = s_parEachJobPool;

    // Init splitting parameters
    const int num_e = es.size();
    const bool should_split = num_e / pool.threads < 1;

    // Ready for splitting jobs
    LongMarch_Vector<std::future<void>> _jobs;

    if (should_split)
    [[unlikely]]
    {
        int splits = pool.threads - num_e;
        for (auto& e : es)
        {
            if (splits > 0)
            [[unlikely]]
            {
                // Try split entity chunks into two parts
                --splits;
                auto e1 = e;
                e1.m_iterEndIndex /= 2;
                if (e1.m_iterBeginIndex <= e1.m_iterEndIndex)
                {
                    _jobs.emplace_back(pool.enqueue_task([this, &func, &e1]()
                        {
                            ENGINE_TRY_CATCH(
                                LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                                func(e1);
                            );
                        }
                    ));
                }

                auto e2 = e;
                e2.m_iterBeginIndex = e1.m_iterEndIndex + 1;
                if (e2.m_iterBeginIndex <= e2.m_iterEndIndex)
                {
                    _jobs.emplace_back(pool.enqueue_task([this, &func, &e2]()
                        {
                            ENGINE_TRY_CATCH(
                                LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                                func(e2);
                            );
                        }
                    ));
                }
            }
            else
            [[likely]]
            {
                _jobs.emplace_back(pool.enqueue_task([this, &func, &e]()
                    {
                        ENGINE_TRY_CATCH(
                            LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                            func(e);
                        );
                    }
                ));
            }
        }
    }
    else
    [[likely]]
    {
        for (auto& e : es)
        {
            _jobs.emplace_back(pool.enqueue_task([this, &func, &e]()
                {
                    ENGINE_TRY_CATCH(
                        LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{ 0 });
                        func(e);
                    );
                }
            ));
        }
    }
    
    // Wait for each job to finish
    for (auto& job : _jobs)
    {
        job.wait();
    }
}
