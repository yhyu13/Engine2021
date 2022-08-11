#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
    struct ParentCom;
    /*
    Data class that stores references to children entities
    */
    struct MS_ALIGN8 ChildrenCom final : public BaseComponent<ChildrenCom>
    {
        ChildrenCom() = default;
        explicit ChildrenCom(const EntityDecorator& _this);
        void AddEntity(const Entity& child);
        const LongMarch_Vector<Entity> GetChildren();
        bool HasEntity(const Entity& child);
        bool RemoveEntity(const Entity& child);
        void RemoveAll();
        bool IsLeaf();

    private:
        friend ParentCom;
        void AddEntityWORecursion(const Entity& child); //!< Set child without recursion

    private:
        LongMarch_Vector<Entity> m_children;
        Entity m_this;
    };
}
