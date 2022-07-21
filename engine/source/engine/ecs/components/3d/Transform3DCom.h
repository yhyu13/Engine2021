#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/math/Geommath.h"
#include "engine/ecs/EntityDecorator.h"

namespace longmarch
{
    struct MS_ALIGN16 Transform3DCom final : BaseComponent<Transform3DCom>
    {
        Transform3DCom() = default;
        explicit Transform3DCom(const EntityDecorator& _this);

        void Update(double ts);

        //! Set Global Transformation
        void SetModelTr(const Mat4& m);
        //! Get Global Transformation
        Mat4 GetModelTr() const;
        //! Called by child
        Mat4 GetSuccessionModelTr(const Transform3DCom& childCom) const;
        //! Called by child
        Mat4 GetSuccessionModelTr(bool apply_trans, bool apply_rot, bool apply_scale) const;

        //! Get Global Transformation in the previous frame
        Mat4 GetPrevModelTr() const;
        //! Called by child
        Mat4 GetPrevSuccessionModelTr(const Transform3DCom& childCom) const;
        //! Called by child
        Mat4 GetPrevSuccessionModelTr(bool apply_trans, bool apply_rot, bool apply_scale) const;

        //! Set parent
        void SetParentModelTr(const Mat4& m);
        //! Remember to reset parent model transformation on removing a parent entity
        void ResetParentModelTr();

        //! Add scale relative to origin (root) 's frame
        void AddGlobalScale(const Vec3f& v);
        //! Set scale relative to origin(root) 's frame
        void SetGlobalScale(const Vec3f& v);
        //! Get scale relative to origin (root) 's frame
        Vec3f GetGlobalScale();

        //! Add scale relative to parent entity in parent 's frame
        void AddRelativeToParentScale(const Vec3f& v);
        //! Set scale relative to parent entity in parent 's frame
        void SetRelativeToParentScale(const Vec3f& v);
        //! Get scale relative to parent entity in parent 's frame
        Vec3f GetRelativeToParentScale();

        //! Add scale to the current scene model (local scale does not pass onto children scene objects)
        void AddLocalScale(const Vec3f& v);
        //! Set scale to the current scene model (local scale does not pass onto children scene objects)
        void SetLocalScale(const Vec3f& v);
        //! Get scale to the current scene model (local scale does not pass onto children scene objects)
        Vec3f GetLocalScale();

        /*******************************************************************
            There exist 3 types of tranlational coordniates
            Global - relative to origin (root must be (0,0,0))
            RelativeToParent - relative to parent
            Local - relative to local rotation
        ********************************************************************/
        //! Add position relative to origin (root) 's frame
        void AddGlobalPos(const Vec3f& v);
        //! Set position relative to origin (root) 's frame
        void SetGlobalPos(const Vec3f& v);
        //! Get position relative to origin (root) 's frame
        Vec3f GetGlobalPos();
        //! Get position relative to origin in the previous frame
        Vec3f GetPrevGlobalPos();

        //! Add position relative to parent entity in parent 's frame
        void AddRelativeToParentPos(const Vec3f& v);
        //! Set position relative to parent entity in parent 's frame
        void SetRelativeToParentPos(const Vec3f& v);
        //! Get position relative to parent entity in parent 's frame
        Vec3f GetRelativeToParentPos();
        //! Get prev position relative to parent entity in parent 's frame
        Vec3f GetPrevRelativeToParentPos();
        //! Add position relative to local frame
        void AddLocalPos(const Vec3f& v);

        //! Add velocity relative to origin (root) 's frame
        void AddGlobalVel(const Vec3f& v);
        //! Set velocity relative to origin (root) 's frame
        void SetGlobalVel(const Vec3f& v);
        //! Get velocity relative to origin (root) 's frame
        Vec3f GetGlobalVel();
        //! Add velocity relative to parent entity parent 's frame
        void AddRelativeToParentVel(const Vec3f& v);
        //! Set velocity relative to parent entity parent 's frame
        void SetRelativeToParentVel(const Vec3f& v);
        //! Get velocity relative to parent entity parent 's frame
        Vec3f GetRelativeToParentVel();
        //! Add velocity relative to local frame
        void AddLocalVel(const Vec3f& v);
        //! Set velocity relative to local frame
        void SetLocalVel(const Vec3f& v);
        //! Get velocity relative to local frame
        Vec3f GetLocalVel();

        /*******************************************************************
            Rotation is always local
            global - relative to right handed coord (x-right, y-front, z-up) at origin (root must be unit Quaternion)
            local - relative to right handed coord (x-right, y-front, z-up) at local
        ********************************************************************/

        //! Add rotation global coordinate
        void AddGlobalRot(const Quaternion& r);
        //! Set rotation local coordinate
        void SetGlobalRot(const Quaternion& r);
        //! Get rotation local coordinate
        Quaternion GetGlobalRot();
        //! Get rotation local coordinate of the previous frame
        Quaternion GetPrevGlobalRot();

        //! Add velocity relative to parent entity in parent 's frame
        void AddRelativeToParentRot(const Quaternion& v);
        //! Set velocity relative to parent entity in parent 's frame
        void SetRelativeToParentRot(const Quaternion& v);
        //! Get velocity relative to parent entity in parent 's frame
        Quaternion GetRelativeToParentRot();

        //! Add rotation local coordinate
        void AddLocalRot(const Quaternion& r);

        /*******************************************************************
            Storing Quaternion for velocity not only take more memory
            But also require using slerp to update rotation
        ********************************************************************/
        //! Add rotational velocity in origin (root) 's frame
        void AddGlobalRotVel(const Vec3f& r);
        //! Set rotational velocity in origin (root) 's frame
        void SetGlobalRotVel(const Vec3f& r);
        //! Get rotational velocity in origin (root) 's frame
        Vec3f GetGlobalRotVel();

        //! Add rotational velocity relative to parent entity in parent 's frame
        void AddRelativeToParentRotVel(const Vec3f& v);
        //! Set velocity relative to parent entity in parent 's frame
        void SetRelativeToParentRotVel(const Vec3f& v);
        //! Get velocity relative to parent entity in parent 's frame
        Vec3f GetRelativeToParentRotVel();

        //! In Euler rad
        void AddLocalRotVel(const Vec3f& r);
        //! In Euler rad
        void SetLocalRotVel(const Vec3f& r);
        //! In Euler rad
        Vec3f GetLocalRotVel();

        virtual void JsonSerialize(Json::Value& value) const override;
        virtual void JsonDeserialize(const Json::Value& value) override;
        virtual void ImGuiRender() override;

        //! This Copy function is just an example
        virtual void Copy(BaseComponentInterface* other) override;

    private:
        Mat4 prev_parentTr{Mat4(1.0f)};
        Mat4 parentTr{Mat4(1.0f)};
        Quaternion prev_rtp_rotation{Geommath::UnitQuat}; // Prev frame Global rotation
        Quaternion rtp_rotation{Geommath::UnitQuat}; // Global rotation
        Vec3f g_rotational_velocity{Vec3f(0.0f)}; // Parent Global rotational velocity
        Vec3f rtp_rotational_velocity{Vec3f(0.0f)}; // Relative to parent rotational velocity
        Vec3f l_rotational_velocity{Vec3f(0.0f)}; // Local rotational velocity
        Vec3f prev_rtp_pos{Vec3f(0.0f)}; // Prev frame Location relative to parent parent 's frame
        Vec3f rtp_pos{Vec3f(0.0f)}; // Location relative to parent in parent 's frame
        Vec3f rtp_velocity{Vec3f(0.0f)}; // Velocity relative to parent in parent 's frame
        Vec3f l_velocity{Vec3f(0.0f)}; // Velocity relative to local frame
        Vec3f g_total_velocity{Vec3f(0.0f)}; // Velocity relative to origin (root) in origin (root) 's frame
        Vec3f rtp_scale{Vec3f(1.0f)}; // Relative to parent frame scale, passed onto children scene objects
        Vec3f l_scale{Vec3f(1.0f)}; // Local scale does not pass onto children scene objects but apply on the current scene object only

    public:
        Entity m_this;
        bool m_apply_parent_trans{true};
        bool m_apply_parent_rot{true};
        bool m_apply_parent_scale{false};

#ifdef DEBUG_DRAW
    public:
        struct debug
        {
            EntityDecorator rotationNode;
            EntityDecorator velocityNode;
            bool showRotation{false};
            bool initRotation{false};
            bool showVelocity{false};
            bool initVelocity{false};
        } debug;
#endif
    };
}
