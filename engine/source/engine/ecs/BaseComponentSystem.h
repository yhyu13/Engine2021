#pragma once
#include "engine/EngineEssential.h"
#include "EntityDecorator.h"
#include "BitMaskSignature.h"
#include "ComponentDecorator.h"
#include "GameWorld.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

namespace longmarch
{
#ifndef EARLY_RETURN
#define  EARLY_RETURN(dt) {if (!dt) return; }
#endif // !EARLY_RETURN

#define RESERVE_SIZE 1

    /**
     * @brief Systems are the places where the actual game-code goes. The entities and
              components are simply data-storage units, systems actually work on that
              data.
     *
     * Each system can specify which components it wishes to pay attention to.
        This information is stored in the form of a bit-mask signature. Entities
        which have the necessary components (entity's signatures matches the
        system's signature) will be registered with the system.

        Usage example:

        // declaring a component
        struct Position : longmarch::BaseComponent<Position> {
            Position() = default;
            Position(float x) : x(x) {};
            float x;
        };

        // declaring a system
        class Wind : public longmarch::BaseComponentSystem {
        public:
            Wind() {
                // wind system wants to pay attention to the Position component, thus creating the
                // signature that contains that information.
                m_systemSignature.AddComponent<Position>();
            }

            // actual game logic goes here
            virtual void Update(double frameTime) override {
            }
        };
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    class BaseComponentSystem : protected BaseAtomicClassNC, public BaseEventSubHandleClass
    {
    public:
        NONCOPYABLE(BaseComponentSystem);

        BaseComponentSystem()
        {
            m_UserRegisteredEntities.reserve(RESERVE_SIZE);
        }

        virtual ~BaseComponentSystem() = default;

        // yuhang : every component system should have a explicit copy constructor named Copy() in order to be cloned between gameworlds
        virtual std::shared_ptr<BaseComponentSystem> Copy() const = 0;
#define COMSYS_DEFAULT_COPY(SystemType) \
        virtual std::shared_ptr<BaseComponentSystem> Copy() const override \
        { \
            auto ret = MemoryManager::Make_shared<SystemType>(); \
            ret->m_UserRegisteredEntities = m_UserRegisteredEntities; \
            ret->m_systemSignature = m_systemSignature; \
            return ret; \
        }

        /*
            Override Init to register components and event call backs
        */
        virtual void Init()
        {
        }

        virtual void Update(double frameTime)
        {
        }

        virtual void Update2(double frameTime)
        {
        }

        virtual void PreRenderUpdate(double dt)
        {
        }

        //! Can be used for debug rendering, it is called just before the main render pipeline
        virtual void Render()
        {
        }

        //! Can be used for debug rendering, it is called just after the main render pipeline
        virtual void Render2()
        {
        }

        virtual void PostRenderUpdate(double dt)
        {
        }

        virtual void RenderUI()
        {
        }

        inline virtual void RemoveAllRegisteredEntities()
        {
            LOCK_GUARD_NC();
            m_UserRegisteredEntities.clear();
        }

        inline void SetWorld(GameWorld* world) const
        {
            m_parentWorld = world;
        }

        //! Dispatcher to parent world
        template <class ComponentType>
        ComponentDecorator<ComponentType> GetComponent(const Entity& entity)
        {
            return m_parentWorld->GetComponent<ComponentType>(entity);
        }

        //! Dispatcher to parent world
        template <class ComponentType>
        bool HasComponent(const Entity& entity) const
        {
            return m_parentWorld->HasComponent<ComponentType>(entity);
        }

        //! Dispatcher to parent world
        __LongMarch_TRIVIAL_TEMPLATE__
        inline void ForEach(const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func) const
        {
            m_parentWorld->ForEach(GetRegisteredEntities(), func);
        }

        //! Dispatcher to parent world
        __LongMarch_TRIVIAL_TEMPLATE__
        [[nodiscard]] inline std::future<void> BackEach(
            const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func) const
        {
            return m_parentWorld->BackEach(GetRegisteredEntities(), func);
        }

        //! Dispatcher to parent world
        __LongMarch_TRIVIAL_TEMPLATE__
        [[nodiscard]] inline std::future<void> ParEach(
            const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func, int min_split = -1) const
        {
            return m_parentWorld->ParEach(GetRegisteredEntities(), func, min_split);
        }

        //! Dispatcher to parent world
        inline void ForEachUser(const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func) const
        {
            m_parentWorld->ForEach(GetUserRegisteredEntities(), func);
        }

        //! Dispatcher to parent world
        __LongMarch_TRIVIAL_TEMPLATE__
        [[nodiscard]] inline std::future<void> BackEachUser(
            const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func) const
        {
            return m_parentWorld->BackEach(GetUserRegisteredEntities(), func);
        }

        //! Dispatcher to parent world
        __LongMarch_TRIVIAL_TEMPLATE__
        [[nodiscard]] inline std::future<void> ParEachUser(
            const std::type_identity_t<std::function<void(const EntityDecorator& e)>>& func, int min_split = -1) const
        {
            return m_parentWorld->ParEach(GetUserRegisteredEntities(), func, min_split);
        }

        /*
         *	@brief Get a copy of all gamework registered entities because registered entities are subjected to change
         **/
        inline const LongMarch_Vector<Entity> GetRegisteredEntities() const
        {
            if (m_systemSignature != BitMaskSignature())
            [[likely]]
            {
                return m_parentWorld->EntityView(m_systemSignature);
            }
            else
            [[unlikely]]
            {
                ENGINE_EXCEPT(
                    L"GetRegisteredEntities() called on a trivial bit mask. This should not happen because system with trivial bit mask should always use User Registered Entities.");
                return LongMarch_Vector<Entity>();
            }
        }

        /*
        *	@brief Get a copy of all user registered entities because registered entities are subjected to change
        *	User registered entities allow iteration on a set of entities that are different
        *	from what this ComponentSystem signature mask has registered from GameWorld
        *
        **/
        inline const LongMarch_Vector<Entity> GetUserRegisteredEntities() const
        {
            LOCK_GUARD_NC();
            return m_UserRegisteredEntities;
        }

        /*
         *	@brief Check if an entity is already registered.
         *	User registered entities allow iteration on a set of entities that are different
         *	from what this ComponentSystem signature mask has registered from GameWorld
         *
         *	@return return true if exists
         **/
        inline bool HasUserEntity(const Entity& entity) const
        {
            LOCK_GUARD_NC();
            return LongMarch_Contains(m_UserRegisteredEntities, entity);
        }

        /*
         *	@brief Add a user registered entity
         *	User registered entities allow iteration on a set of entities that are different
         *	from what this ComponentSystem signature mask has registered from GameWorld
         *
         *	@detail simply emplace back an entity, would not check if entity exists.
         *	Use HasUserEntity() to check existence.
         **/
        inline void AddUserEntity(const Entity& entity)
        {
            LOCK_GUARD_NC();
            m_UserRegisteredEntities.emplace_back(entity);
        }

        /*
         *	@brief Remove a user registered entity
         *	User registered entities allow iteration on a set of entities that are different
         *	from what this ComponentSystem signature mask has registered from GameWorld
         *
         *	@return return true if entity can be found and removed
         **/
        inline bool RemoveUserEntity(const Entity& entity)
        {
            LOCK_GUARD_NC();
            return LongMarch_EraseFirst(m_UserRegisteredEntities, entity) != -1;
        }

        inline BitMaskSignature& GetSystemSignature()
        {
            return m_systemSignature;
        }

    protected:
        LongMarch_Vector<Entity> m_UserRegisteredEntities;
        BitMaskSignature m_systemSignature;
        mutable GameWorld* m_parentWorld{nullptr};
    };

#undef RESERVE_SIZE
}
