#pragma once
#include <memory>
#include <vector>
#include <map>
#include "engine/core/exception/EngineException.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/thread/ThreadPool.h"
#include "engine/core/thread/StealThreadPool.h"
#include "engine/core/utility/TypeHelper.h"
#include "EntityType.h"
#include "EntityDecorator.h"
#include "EntityManager.h"
#include "BitMaskSignature.h"
#include "ComponentManager.h"
#include "ComponentDecorator.h"

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
    class GameWorld final : public BaseAtomicClassNC, public BaseAtomicClassStatic, public AdaptiveAtomicClassNC
    {
    private:
        NONCOPYABLE(GameWorld);
        GameWorld() = delete;
        explicit GameWorld(bool setCurrent, const std::string& name, const fs::path& filePath);

    public:
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
        static GameWorld* GetCurrent();
        static GameWorld* GetManagedWorldByName(const std::string& name);
        static const LongMarch_Vector<std::string> GetAllManagedWorldNames();

        /**
         * @brief	Remove a managed world, throw exception if the world does not exist
         * @param	name		the name of the world
         */
        static void RemoveManagedWorld(const std::string& name);

        /**
         * @brief	Remove a managed world, throw exception if the world is not managed
         * @param	world		the pointer to the world
         */
        static void RemoveManagedWorld(GameWorld* world);

        /**
         * @brief	Clone a given world with a new name, throw exception if the world does not exist
         * @param	name		the name of the world
         */
        static GameWorld* Clone(const std::string& newName, const std::string& name);

        /**
         * @brief	Clone a given world with a new name
         * @param	world		the pointer to the world
         */
        static GameWorld* Clone(const std::string& newName, GameWorld* world);

        inline const std::string& GetName() const { return m_name; }
        inline void SetPause(bool b) { m_paused = b; }
        inline bool IsPaused() const { return m_paused; }

        //! Initialize all systems
        void Init();
        void Update(double frameTime);
        void Update2(double frameTime);
        void Update3(double frameTime);

        void MultiThreadUpdate(double frameTime);
        void MultiThreadJoin();

        void PreRenderUpdate(double frameTime);
        void Render(double frameTime);
        void Render2(double frameTime);

        void PostRenderUpdate(double frameTime);
        void RenderUI();

        /**************************************************************
        *	Remover
        **************************************************************/
        //! Helper method that inactivates an entity
        void InactivateHelper(Entity e);

        //! Helper method that removes an entity from its parent
        void RemoveFromParentHelper(Entity e);

        //! Remove a specific entity only, its components still persists
        void RemoveEntity(const Entity& entity);

        //! Remove a specific entity and all its components
        void RemoveEntityAndComponents(const Entity& entity);

        /*
            Launch RemoveAllEntities() for each componenet system.
            This method does not remove components or entities by itself.
            You should create a GC component system that effectly remove all components and entities
        */
        void RemoveAllEntities();
        /*
            Remove all component systems
        */
        void RemoveAllComponentSystems();

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

        //! Check if entity is valid and exists in this gameworld
        bool HasEntity(const Entity& entity) const;

        //! Helper method that links an entity to a new parent, remove older parent as well.
        void AddChildHelper(Entity parent, Entity child);

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
        const LongMarch_Vector<std::string> GetAllComponentSystemName();

        //! In the order of execution of component systems
        const LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> GetAllComponentSystem();

        //! In the order of execution of component systems
        const LongMarch_Vector<std::pair<std::string, std::shared_ptr<BaseComponentSystem>>>
        GetAllComponentSystemNamePair();

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

        template <class... Components>
        const LongMarch_Vector<Entity> EntityView() const;
        
        const LongMarch_Vector<Entity> EntityView(const BitMaskSignature& bitMask) const;

        //! Unity ECS like for each function
        template <class... Components>
        void ForEach(typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func) const;

        //! Unity ECS like for each function (single worker thread), func is moved
        template <class... Components>
        [[nodiscard]] auto BackEach(
            typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func) const;

        //! Unity ECS like for each function (multi worker thread), func is moved
        template <class... Components>
        [[nodiscard]] auto ParEach(
            typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func,
            int min_split = -1) const
        {
            return StealThreadPool::GetInstance()->enqueue_task([this, min_split, func = std::move(func)]()
            {
                _ParEach<Components...>(func, min_split);
            });
        }

        //! Unity ECS like for each function
        void ForEach(const LongMarch_Vector<Entity>& es,
                     typename Identity<std::function<void(const EntityDecorator& e)>>::Type func) const;

        //! Unity ECS like for each function (single worker thread), func is moved
        [[nodiscard]] std::future<void> BackEach(const LongMarch_Vector<Entity>& es,
                                                 typename Identity<std::function<void(const EntityDecorator& e)>>::Type
                                                 func) const;

        //! Unity ECS like for each function (multi worker thread), func is moved
        [[nodiscard]] std::future<void> ParEach(const LongMarch_Vector<Entity>& es,
                                                typename Identity<std::function<void(const EntityDecorator& e)>>::Type
                                                func, int min_split = -1) const;

    private:
        //! Helper method that wraps exception handling for thread job
        inline void _MultiThreadExceptionCatcher(typename Identity<std::function<void()>>::Type func) const
        {
            ENGINE_TRY_CATCH({ func(); });
        };

        //! Init both system and scene from a single file
        void InitSystemAndScene(const fs::path& _file);
        //! Call InitSystem before InitScene
        void InitSystem(const fs::path& system_file);
        //! Call InitSystem before InitScene
        void InitScene(const fs::path& scene_file);

        //! Helper method for pareach
        void _ParEach2(const LongMarch_Vector<Entity>& es,
                       typename Identity<std::function<void(const EntityDecorator& e)>>::Type func,
                       int min_split = -1) const;

        //! Helper function for adding/removing entity from component system
        template <typename ComponentType>
        void _TryAddEntityForAllComponentSystems(const Entity& entity);

        template <typename ComponentType>
        void _TryRemoveEntityForAllComponentSystems(const Entity& entity);

        //! Get the component-manager for a given component-type. Example usage: _GetComponentManager<ComponentType>();
        template <typename ComponentType>
        ComponentManager<ComponentType>* _GetComponentManager() const;

        //! Helper method for pareach
        template <class... Components>
        void _ParEach(typename Identity<std::function<void(const EntityDecorator& e, Components&...)>>::Type func,
                      int min_split = -1) const;

    private:
        // Entity (ECS)
        //!< Contains all entities and their compoenent bit masks
        std::shared_ptr<EntityManager> m_entityManager;
        //!< Contains all entities and their compoenent bit masks
        LongMarch_UnorderedMap_Par_node<Entity, BitMaskSignature> m_entityMaskMap;
        //!< Contains all compoenent bit masks and their corresponding entities
        LongMarch_UnorderedMap_node<BitMaskSignature, LongMarch_Vector<Entity>> m_maskEntityVecMap;

        // Component (ECS)
        //!< Contains all component managers which are indexed by component indices
        mutable LongMarch_UnorderedMap_flat<ComponentIndexType, std::shared_ptr<BaseComponentManager>> m_componentManagers;

        // System (ECS)
        //!< In order array of all systems
        LongMarch_Vector<std::shared_ptr<BaseComponentSystem>> m_systems;
        //!< In order array of all names of systems
        LongMarch_Vector<std::string> m_systemsName;
        //!< System LUT based on names, iterating over this container does not gaurantee orderness
        LongMarch_UnorderedMap_flat<std::string, std::shared_ptr<BaseComponentSystem>> m_systemsNameMap;

        // Misc
        //! Holds multithreaded job that are created in a instance of gameworld
        AtomicQueueNC<std::shared_future<void>> m_jobs;
        //! Name of the game world
        std::string m_name;
        //! Is game world paused
        std::atomic_bool m_paused = {false};

    private:
        inline static LongMarch_UnorderedMap_flat<std::string, std::unique_ptr<GameWorld>> allManagedWorlds;
        inline static GameWorld* currentWorld = {nullptr};

        //! Multithreaded pool used in ParEach2 for inner function multithreading to avoid overflow stalling the default thread pool 
        inline static StealThreadPool s_parEach2Pool;
        //! GameWorld class level job pool, used in running game thread in the backgroud or any other async tasks
        inline static StealThreadPool s_JobPool{4};
    };
}

#include "GameWorld.inl"
#include "EntityDecorator.inl"
