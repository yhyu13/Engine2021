#pragma once
#include <memory>
#include <vector>
#include "engine/core/exception/EngineException.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/thread/ThreadPool.h"
#include "engine/core/thread/StealThreadPool.h"
#include "engine/core/utility/TypeHelper.h"
#include "engine/core/thread/RivalLock.h"
#include "engine/core/smart-pointer/RefPtr.h"

#include "engine/ecs/EntityManager.h"
#include "engine/ecs/BitMaskSignature.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs/ComponentManager.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/EntityType.h"

#include "engine/events/EventQueue.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

namespace longmarch
{
    class BaseComponentSystem;
    /**
     * @brief GameWorld is the abstraction for the internal workings of the ECS system.
     *
     * GameWorld provides communication among various parts of the ECS. In essence all the
     * game world does is provide users simplified methods for interacting
     * with the ECS system.
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    class GameWorld final : private BaseAtomicClassStatic, public BaseRefCountClassNC
    {
    public:
        struct GameWorldReadOnlyMode_Guard
        {
            GameWorldReadOnlyMode_Guard() = delete;
            GameWorldReadOnlyMode_Guard(const GameWorld* world)
                :
                m_world(world)
            {
                m_world->m_RWMode = GameWorldReadWriteMode::READ_ONLY;
            }
            ~GameWorldReadOnlyMode_Guard()
            {
                m_world->m_RWMode = GameWorldReadWriteMode::VOLATILE;
            }
            const GameWorld* m_world;
        };
        
    private:
        enum class GameWorldReadWriteMode : uint8_t
        {
            VOLATILE = 0, // Allow unordered read & write by applying rival locks
            READ_ONLY = 1, // Only allow read, will throw an exception on write
        };

        typedef LongMarch_UnorderedMap_flat<ComponentTypeIndex_T, void*> ComponentCacheMap_T;
        struct EntityMaskValue_T
        {
            EntityMaskValue_T() = default;
            EntityMaskValue_T(const EntityMaskValue_T& other)
            {
                GetBitMask() = std::get<0>(other.EntityMaskValue);
            }
            EntityMaskValue_T& operator=(const EntityMaskValue_T& other)
            {
                GetBitMask() = std::get<0>(other.EntityMaskValue);
                return *this;
            }
            EntityMaskValue_T(EntityMaskValue_T&& other) noexcept
            {
                GetBitMask() = std::move(std::get<0>(other.EntityMaskValue));
            }
            EntityMaskValue_T& operator=(EntityMaskValue_T&& other) noexcept
            {
                GetBitMask() = std::move(std::get<0>(other.EntityMaskValue));
                return *this;
            }
            
            BitMaskSignature& GetBitMask() const
            {
                return std::get<0>(EntityMaskValue);
            }
            ComponentCacheMap_T& GetComponentCache() const
            {
                return std::get<1>(EntityMaskValue);
            }
        private:
            mutable std::tuple<BitMaskSignature, ComponentCacheMap_T> EntityMaskValue;
        };
        
    private:
        NONCOPYABLE(GameWorld);
        GameWorld() = delete;
        explicit GameWorld(bool setCurrent, const std::string& name, const fs::path& filePath);
        friend TemplateMemoryManager<GameWorld>;

    public:
        /**
         * Init message binding and resources
         */
        static void Init();

        /**
         *  Clean Up all resources held by Gameworld
         */
        static void CleanUp();

        /**
         * @brief	Get a game world instance that is automatically managed.
         * @param	setCurrent		set the new instance to be the current world
         * @param	name			name of the new world
         * @param	filePath		deserialization the new world from a serialized file, if the file is empty, then you would need call Init() mannully
         */
        static GameWorld* GetInstance(bool setCurrent, const std::string& name, const fs::path& filePath);
        [[nodiscard]] static std::future<GameWorld*> GetInstanceAsync(bool setCurrent, const std::string& name,
                                                                      const fs::path& filePath);
        static void SetCurrent(GameWorld* world);
        [[nodiscard]] static GameWorld* GetCurrent();
        [[nodiscard]] static GameWorld* GetWorldByName(const std::string& name);
        static const LongMarch_Vector<std::string> GetAllManagedWorldNames();

        /**
         * @brief	Remove a managed world, throw exception if the world does not exist
         * @param	name		the name of the world
         */
        static bool RemoveWorld(const std::string& name);

        /**
         * @brief	Remove a managed world, throw exception if the world is not managed
         * @param	world		the pointer to the world
         */
        static bool RemoveWorld(GameWorld* world);

        /**
         * > Clone a world from an existing world
         * 
         * @param newWorldName The name of the new world to be created.
         * @param fromWorldName The name of the world to clone from.
         * 
         * @return A pointer to a GameWorld object.
         */
        static GameWorld* Clone(const std::string& newWorldName, const std::string& fromWorldName);

        /**
         * It copies the entities, components and systems from the `from` world to the `newWorld` world
         * 
         * @param newName The name of the new world.
         * @param from The world to copy from.
         * 
         * @return A pointer to a GameWorld object.
         */
        static GameWorld* Clone(const std::string& newName, const GameWorld* from);

    private:
        static void _ON_ENG_WINDOW_QUIT(EventQueue<EngineEventType>::EventPtr e);

    public:
        inline const std::string& GetName() const { return m_name; }
        inline void SetPause(bool b) { m_paused = b; }
        inline bool IsPaused() const { return m_paused; }

        void Update(double frameTime);

#if MULTITHREAD_UPDATE
        void MultiThreadUpdate(double frameTime);
        void MultiThreadJoin();
#endif

        void PreRenderUpdate(double frameTime);
        
        void PreRenderPass(double frameTime);
        void PostRenderPass(double frameTime);

        void PostRenderUpdate(double frameTime);
        void RenderUI();

        /**************************************************************
        *	Helper
        **************************************************************/
        //! Helper method that inactivates an entity
        void InactivateEntity_Helper(Entity e);

        //! Helper method that removes an entity from its parent
        void RemoveFromParent_Helper(Entity e);

        /**************************************************************
        *	Entity
        **************************************************************/
        /*
            Generate an entity with basic components:
            ActiveCom,
            ParentCom,
            ChildrenCom
        */
        EntityDecorator GenerateEntity(EntityType type, bool active, bool add_to_root);
        /*
            Generate an entity with basic components:
            ActiveCom,
            ParentCom,
            ChildrenCom,
            Transform3DCom,
            Scene3DCom,
            Body3DCom
        */
        EntityDecorator GenerateEntity3D(EntityType type, bool active, bool add_to_root);
        /*
            Generate an entity with basic components:
            ActiveCom,
            ParentCom,
            ChildrenCom,
            Transform3DCom,
            Scene3DCom,
        */
        EntityDecorator GenerateEntity3DNoCollision(EntityType type, bool active, bool add_to_root);

        //! Remove a specific entity and all its components
        void RemoveEntity(const Entity& entity);

        //! Check if entity is valid and exists in this gameworld
        bool HasEntity(const Entity& entity) const;

        //! Helper method that links an entity to a new parent, remove older parent as well.
        void AddChild_Helper(Entity parent, Entity child);

        //! Type must have one entity. If that entity does not exist or there exists more than one, throw an exception
        const Entity GetTheOnlyEntityWithType(EntityType type) const;
        const LongMarch_Vector<Entity> GetAllEntityWithType(EntityType type) const;
        const LongMarch_Vector<Entity> GetAllEntityWithType(const std::initializer_list<EntityType>& types) const;
        const LongMarch_Vector<Entity> GetAllEntityWithType(const LongMarch_Vector<EntityType>& types) const;
        const Entity GetEntityFromID(EntityID ID) const;

        /**************************************************************
        *	Component System
        **************************************************************/
        void RegisterSystem(const std::shared_ptr<BaseComponentSystem>& system, const std::string& name);

        //! Return a pointer to a specific component system, return nullptr if it does not exist
        BaseComponentSystem* GetComponentSystem(const std::string& name);

        //! In the order of execution of component systems
        const LongMarch_Vector<std::string> GetAllComponentSystemName() const;

        //! In the order of execution of component systems
        const LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> GetAllComponentSystem() const;

        //! In the order of execution of component systems
        const LongMarch_Vector<std::pair<std::string, std::shared_ptr<BaseComponentSystem>>>
        GetAllComponentSystemNamePair() const;

        /**************************************************************
        *	Component
        **************************************************************/

        //! Use this method to get all components for an entity.
        const LongMarch_Vector<BaseComponentInterface*> GetAllComponent(const Entity& entity) const;

        //! Use this method to remove all components for an entity.
        void RemoveAllComponent(const Entity& entity);

        template <typename ComponentType>
        bool HasComponent(const Entity& entity) const;

        template <typename ComponentType>
        void AddComponent(const Entity& entity, const ComponentType& component);

        template <typename ComponentType>
        void RemoveComponent(const Entity& entity);

        template <typename ComponentType>
        ComponentDecorator<ComponentType> GetComponent(const Entity& entity) const;

        // Unity DOTS ECS like for each function ------------------------------------------------------------------
        template <class... Components>
        const LongMarch_Vector<Entity> EntityView() const;

        const LongMarch_Vector<Entity> EntityView(const BitMaskSignature& bitMask) const;
        
        template <class... Components>
        void ForEach(
            const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func) const;

        template <class... Components>
        [[nodiscard]] auto BackEach(
            const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func) const;

        template <class... Components>
        [[nodiscard]] auto ParEach(
            const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func,
            int min_batch = -1) const;

        void ForEach(const LongMarch_Vector<Entity>& es,
                     const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func) const;

        // Running job in a single backgroud thread
        [[nodiscard]] std::future<void> BackEach(const LongMarch_Vector<Entity>& es,
                                                 const std::type_identity_t<std::function<void
                                                     (const EntityDecorator& e)>>&
                                                 func) const;

        // Running jobs in a thread pool
        [[nodiscard]] std::future<void> ParEach(const LongMarch_Vector<Entity>& es,
                                                const std::type_identity_t<std::function<void
                                                    (const EntityDecorator& e)>>&
                                                func, int min_batch = -1) const;

        // UE5 Mass ECS like for each chunk function ------------------------------------------------------------------
        template <class... Components>
        const LongMarch_Vector<EntityChunkContext> EntityChunkView() const;

        const LongMarch_Vector<EntityChunkContext> EntityChunkView(const BitMaskSignature& bitMask) const;

        void ForEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
             const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>& func) const;

        // Running job in a single background thread
        [[nodiscard]] std::future<void> BackEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
                                                 const std::type_identity_t<std::function<void
                                                     (const EntityChunkContext& e)>>&
                                                 func) const;

        // Running jobs in a thread pool
        [[nodiscard]] std::future<void> ParEachChunk(const LongMarch_Vector<EntityChunkContext>& es,
                                                const std::type_identity_t<std::function<void
                                                    (const EntityChunkContext& e)>>&
                                                func, int min_batch = -1) const;

    private:
        //! Init ECS systems
        void InitECS();
        //! Init both system and scene from a single file
        void InitSystemAndScene(const fs::path& _file);
        //! Call InitSystem before InitScene
        void InitSystem(const fs::path& system_file);
        //! Call InitSystem before InitScene
        void InitScene(const fs::path& scene_file);

        //! Helper method for pareach
        template <class... Components>
        void ParEach_Internal(const std::type_identity_t<std::function<void(const EntityDecorator& e, Components&...)>>& func,
                      int min_batch = -1) const;

        //! Helper method for pareach
        void ParEach_Internal(const LongMarch_Vector<Entity>& es,
                       const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func,
                       int min_batch = -1) const;

        void ParEachChunk_Internal(const LongMarch_Vector<EntityChunkContext>& es,
                       const std::type_identity_t<std::function<void(const EntityChunkContext& e)>>& func,
                       int min_batch = -1) const;

        bool ShouldApplyRivalLock() const
        {
            return m_RWMode != GameWorldReadWriteMode::READ_ONLY;
        }

    private:
        inline static LongMarch_UnorderedMap_flat<std::string, RefPtr<GameWorld>> s_allManagedWorlds;
        inline static GameWorld* s_currentWorld {nullptr};

        //! Multithreaded pool used in ParEach2 for inner function multithreading to avoid thread overflow stalling the default thread pool and the entire program
        inline static StealThreadPool s_parEachJobPool;
        constexpr inline static int s_parEachMinBatch{64};
        
    private:
        // Entity (E)
        //!< Contains all entities
        std::shared_ptr<EntityManager> m_entityManager;
        //!< Contains all entities and their component bit masks and component pointer location cache (do not copy this component cache map when cloning a gameworld)
        LongMarch_UnorderedMap_Par_node<Entity, EntityMaskValue_T> m_entityMaskMap;

        // Component (C)
        //!< Contains all components bit masks and their corresponding entities
        LongMarch_UnorderedMap_node<BitMaskSignature, std::shared_ptr<ArcheTypeManager>> m_maskArcheTypeMap;

        // System (S)
        //!< In order array of all systems
        LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> m_systems;
        //!< In order array of all names of systems
        LongMarch_Vector<std::string> m_systemsName;
        //!< System LUT based on names, iterating over this container does not gaurantee orderness
        LongMarch_UnorderedMap_node<std::string, std::shared_ptr<BaseComponentSystem>> m_systemsNameMap;

        // Misc
        //! Read & Write access mode
        mutable GameWorldReadWriteMode m_RWMode {GameWorldReadWriteMode::VOLATILE};
        //! Rival group lock
        mutable RivalLock<2> m_rivalLock;
        //! Holds multi-threaded job that are created in a instance of Gameworld
        std::future<void> m_asyncUpdatejob;
        //! Name of the game world
        std::string m_name;
        //! Is game world paused
        std::atomic_bool m_paused {false};

#define TRY_LOCK_READ() LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{0}, ShouldApplyRivalLock())
#define TRY_LOCK_WRITE() LOCK_GUARD_RIVAL(m_rivalLock, RivalGroup{1}, ShouldApplyRivalLock()); \
do {ASSERT(m_RWMode != GameWorldReadWriteMode::READ_ONLY);} while(0)

#if GAMEWORLD_UPDATE_READ_ONLY
        #define LOCK_GUARD_GAMEWORLD_READYONLY(world) GameWorld::GameWorldReadOnlyMode_Guard _gw_read_only_guard(world);ATOMIC_THREAD_FENCE
#else
        #define LOCK_GUARD_GAMEWORLD_READYONLY(...) 
#endif
    };
}

#include "GameWorld.inl"
#include "EntityDecorator.inl"
