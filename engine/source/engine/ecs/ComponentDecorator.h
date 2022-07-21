#pragma once
#include "EntityDecorator.h"
#include "ComponentManager.h"

namespace longmarch
{
    /**
     * @brief Component decorators are the wrapper constructs for components.
        These decorators help in creating useful abstractions for
        component specific operations.
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    template <typename ComponentType>
    class ComponentDecorator
    {
    public:
        ComponentDecorator() = default;

        explicit ComponentDecorator(const EntityDecorator& owner, ComponentType* component)
            :
            m_entity(owner), m_component(component)
        {
        }

        inline bool Update(bool throw_on_invalid = true)
        {
            m_component = m_entity.GetComponent<ComponentType>().GetPtr();
            if (Valid())
            {
                return true;
            }
            else
            {
                if (throw_on_invalid)
                {
                    ENGINE_EXCEPT(
                        wStr(Str(m_entity)) + L" fails to update component " + wStr(typeid(ComponentType).name()));
                }
                return false;
            }
        }

        inline bool Valid() const
        {
            return m_component != nullptr && m_entity.Valid();
        }

        inline ComponentType* operator->() const
        {
            return m_component;
        }

        inline ComponentType* GetPtr() const
        {
            return m_component;
        }

        inline ComponentType& GetRef() const
        {
            return *m_component;
        }

        inline EntityDecorator GetOwner() const
        {
            return m_entity;
        }

        friend inline void swap(ComponentDecorator<ComponentType>& x, ComponentDecorator<ComponentType>& y)
        {
            std::swap(*x.m_component, *y.m_component);
        }

        inline operator ComponentType*() const
        {
            return m_component;
        }

        inline operator ComponentType&() const
        {
            return *m_component;
        }

    private:
        EntityDecorator m_entity;
        ComponentType* m_component = {nullptr};
    };
}
