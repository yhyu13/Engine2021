#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/scene-graph/Scene3DNode.h"
#include "engine/core/asset-manager/ResourceManager.h"

namespace longmarch
{
    //! Data class that stores references to the current animation and maintain an animation state map
    struct CACHE_ALIGN Animation3DCom final : BaseComponent<Animation3DCom>
    {
    public:
        struct AnimationSetting
        {
            std::string name;
            float playBackSpeed{1.0f};
            bool refresh{true};
            bool looping{true};
            bool pause{false};
        };

    public:
        Animation3DCom() = default;
        explicit Animation3DCom(const EntityDecorator& _this);

        //! Set a different collection of 3D animation (and skeleton) from resource manager
        void SetAnimationCollection(const std::string& name);

        //! Set the current animation to play in the next frame in the animation hierarachy
        void SetCurrentAnimation(const AnimationSetting& anima);

        //! The animation could play at a different frame rate which is useful for distanant objects
        void SetAnimationTickTimer(float period);

        //! Update the current animation
        void UpdateAnimation(double dt, const std::shared_ptr<Scene3DNode>& sceneNode);

        virtual void JsonSerialize(Json::Value& value) const override;
        virtual void JsonDeserialize(const Json::Value& value) override;
        virtual void ImGuiRender() override;

    public:
        std::string currentAnimName{"None"};
        std::shared_ptr<Animation3D> m_animaRef{nullptr};
        std::shared_ptr<FABRIKResolver> m_IKResolverRef{nullptr};
        Timer animationTickTimer{1.0 / 60.0};
        Entity m_this;
        float currentTime{0.0f}; //!< current time of animation in seconds
        float playBackSpeed{1.0f};
        bool pause{false};
        bool looping{true};

#ifdef DEBUG_DRAW
    public:
        struct debug
        {
            std::string ee_bone_name;
            std::string ik_root_bone_name;
            EntityDecorator rootNode;
            EntityDecorator ikTargetNode;
            EntityDecorator ikPoleNode;
            uint32_t numBones{2u};
            int showBoneType{0};
            bool showBone{false};
            bool showBoneInit{false};
            bool hideMesh{false};
            bool enableIKTarget{false};
            bool showIKTarget{false};
            bool showIKTargetInit{false};
            bool enablePole{false};
        } debug;
#endif
    };
}
