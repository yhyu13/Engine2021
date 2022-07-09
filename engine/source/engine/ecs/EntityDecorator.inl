#pragma once
#include "EntityDecorator.h"

namespace longmarch
{
    template <typename ComponentType>
    void EntityDecorator::AddComponent(const ComponentType& component)
    {
        GetVolatileWorld()->AddComponent<ComponentType>(m_entity, component);
    }

    template <typename ComponentType>
    void EntityDecorator::RemoveComponent()
    {
        GetVolatileWorld()->RemoveComponent<ComponentType>(m_entity);
    }

    template <typename ComponentType>
    ComponentDecorator<ComponentType> EntityDecorator::GetComponent() const
    {
        return GetWorld()->GetComponent<ComponentType>(m_entity);
    }

    template <typename ComponentType>
    bool EntityDecorator::HasComponent() const
    {
        return GetWorld()->HasComponent<ComponentType>(m_entity);
    }

    __LongMarch_TRIVIAL_TEMPLATE__
    LongMarch_Vector<BaseComponentInterface*> EntityDecorator::GetAllComponent() const
    {
        return GetWorld()->GetAllComponent(m_entity);
    }
}

__LongMarch_TRIVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& os, const longmarch::EntityDecorator& ed)
{
    os << ed.GetWorld()->GetName() << ":" << ed.GetEntity();
    return os;
}

__LongMarch_TRIVIAL_TEMPLATE__
bool operator==(const EntityDecorator& lhs, const EntityDecorator& rhs)
{
    ASSERT(lhs.GetWorld() == rhs.GetWorld(), Str(lhs) + " is in a differnet world of " + Str(rhs));
    return lhs.GetEntity() == rhs.GetEntity();
}

__LongMarch_TRIVIAL_TEMPLATE__
bool operator!=(const EntityDecorator& lhs, const EntityDecorator& rhs)
{
    ASSERT(lhs.GetWorld() == rhs.GetWorld(), Str(lhs) + " is in a differnet world of " + Str(rhs));
    return lhs.GetEntity() != rhs.GetEntity();
}

__LongMarch_TRIVIAL_TEMPLATE__
bool operator<(const EntityDecorator& lhs, const EntityDecorator& rhs)
{
    ASSERT(lhs.GetWorld() == rhs.GetWorld(), Str(lhs) + " is in a differnet world of " + Str(rhs));
    return lhs.GetEntity() < rhs.GetEntity();
}
