#pragma once
#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
    class GameWorld;

    template <typename ComponentType>
    class ComponentDecorator;

    struct MS_ALIGN8 EntityDecoratorVolatile
    {
        explicit EntityDecoratorVolatile(const Entity& entity, GameWorld* world)
            :
            m_entity(entity),
            m_world(world)
        {
        }

        template <typename ComponentType>
        void AddComponent(const ComponentType& component) const;

        template <typename ComponentType>
        void RemoveComponent() const;

        inline GameWorld* GetWorld() const
        {
            return m_world;
        }

    private:
        Entity m_entity;
        GameWorld* m_world{nullptr};
    };

    struct MS_ALIGN8 EntityDecorator
    {
        EntityDecorator() = default;

        explicit EntityDecorator(const Entity& entity, const GameWorld* world)
            :
            m_entity(entity),
            m_world(world)
        {
        }

        template <typename ComponentType>
        [[nodiscard]] ComponentDecorator<ComponentType> GetComponent() const;

        template <typename ComponentType>
        [[nodiscard]] bool HasComponent() const;

        __LongMarch_TRIVIAL_TEMPLATE__
        [[nodiscard]] LongMarch_Vector<BaseComponentInterface*> GetAllComponent() const;

        inline bool Valid() const
        {
            return m_entity != Entity() && m_world != nullptr;
        }

        //! Reset entity decorator (aka. invalidate it)
        inline void Reset()
        {
            m_entity = Entity();
            m_world = nullptr;
        }

        inline EntityDecoratorVolatile Volatile() const
        {
            return EntityDecoratorVolatile{GetEntity(), const_cast<GameWorld*>(GetWorld())};
        }

        inline const Entity GetEntity() const
        {
            return m_entity;
        }

        inline const EntityID GetID() const
        {
            return m_entity.m_id;
        }

        inline const EntityType GetType() const
        {
            return m_entity.m_type;
        }

        inline const GameWorld* GetWorld() const
        {
            return m_world;
        }

        inline operator Entity() const
        {
            return m_entity;
        }

        inline operator EntityID() const
        {
            return m_entity.m_id;
        }

        inline operator EntityType() const
        {
            return m_entity.m_type;
        }

    private:
        Entity m_entity;
        const GameWorld* m_world{nullptr};
    };
}

__LongMarch_TRIVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& os, const longmarch::EntityDecorator& ed);

__LongMarch_TRIVIAL_TEMPLATE__
bool operator==(const EntityDecorator& lhs, const EntityDecorator& rhs);

__LongMarch_TRIVIAL_TEMPLATE__
bool operator!=(const EntityDecorator& lhs, const EntityDecorator& rhs);

__LongMarch_TRIVIAL_TEMPLATE__
bool operator<(const EntityDecorator& lhs, const EntityDecorator& rhs);

/*
	Custum hash function for EntityDecorator
*/
namespace std
{
    template <>
    struct hash<longmarch::EntityDecorator>
    {
        std::size_t operator()(const longmarch::EntityDecorator& e) const noexcept
        {
            // TODO @yuhang : consider combine world hash into hash of entity decorator
            return hash<longmarch::Entity>()(e.GetEntity());
        }
    };
}
