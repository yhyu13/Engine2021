#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "../../ai/BehaviorTree.h"
#include "../../ai/Blackboard.h"

namespace longmarch
{
    struct CACHE_ALIGN AIControllerCom final : public BaseComponent<AIControllerCom>
    {
    public:
        // Member Functions
        AIControllerCom() = default;
        explicit AIControllerCom(const EntityDecorator& _this);

        void Update(double dt);

        BehaviorTree& GetBT();
        void SetBT(const fs::path& filepath);

        Blackboard* GetBlackboard();
        void SetBlackboard(Blackboard* blackboard);

    public:
        Entity m_this;

    private:
        BehaviorTree BT;
        Blackboard* bb{nullptr};
    };
}
