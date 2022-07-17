#pragma once

#include "engine/core/utility/TypeHelper.h"
#include "engine/physics/Shape.h"
#include "engine/physics/RBTransform.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/Entity.h"

namespace longmarch
{
    enum RBType
    {
        EMPTY = -1,
        noCollision = 0,
        staticBody,
        dynamicBody,
        kinematicBody,
        NUM
    };

    class RigidBody
    {
    public:
        RigidBody();

        void ApplyLinearForce(const Vec3f& force);
        void ApplyForceAtWorldPoint(const Vec3f& force, const Vec3f& point);

        void ApplyLinearImpulse(const Vec3f& impulse);
        void ApplyLinearImpulseAtWorldPoint(const Vec3f& impulse, const Vec3f& point);

        void ApplyTorque(const Vec3f& torque);

        void SetAwake();
        void Sleep();

        bool IsAwake() const;

        float GetMass() const;
        float GetInvMass() const;

        const Vec3f& GetLinearVelocity() const;
        const Vec3f& GetWorldPosition() const;
        const Vec3f& GetPrevWorldPosition() const;

        float GetLinearDamping() const;
        float GetAngularDamping() const;

        const Vec3f GetLinearAcceleration() const;

        float GetRestitution() const;

        float GetGravityScale() const;
        float GetFriction() const;

        const Entity& GetEntity() const;
        void SetEntity(const Entity& entity);

        void SetMass(float mass);
        void SetLinearVelocity(const Vec3f& velocity);
        void SetAngularVelocity(const Vec3f& velocity);

        void SetLinearDamping(float damping);
        void SetAngularDamping(float damping);

        void SetRestitution(float restitution);

        void SetGravityScale(float gravityScale);
        void SetFriction(float friction);

        void SetWorldPosition(const Vec3f& pos);
        void SetPrevWorldPosition(const Vec3f& pos);

        void SetWorldRotation(const Quaternion& rot);
        void SetWorldScale(const Vec3f& scale);
        void SetWorldTransform(const Mat4& trans);

        void UpdateAABBShape();
        void SetAABBShape(const Vec3f& aabbMin, const Vec3f& aabbMax);
        const std::shared_ptr<Shape> GetShape() const;

        Vec3f GetAABBWidths() const;

        void SetColliderDisplacement(const Vec3f& displacement);
        Vec3f GetColliderDisplacement() const;

        void SetRBTrans(const Mat4& trans);
        const RBTransform& GetRBTrans() const;

        RBType GetRBType() const;
        void SetRBType(RBType type);

        void SetCollisionStatus(bool collided, float solveTimeLeft);
        bool IsCollided() const;
        float GetSolveTimeLeft() const;

        void ClearAllForces();

        // Render the bounding volume
        void Render();

    public:
        LongMarch_Bitset256<EntityType> m_entityTypeIngoreSet;

    private:
        // params still needed: flags (tbd)
        RBType m_rbType;

        Entity m_entity;

        // usage to be decided
        uint32_t m_islandIndex;

        float m_restitution;

        float m_mass;
        float m_invMass;

        float m_linearDamping;
        float m_angularDamping;

        float m_sleepTime;
        float m_gravityScale;

        float m_friction;

        Vec3f m_prevPos;

        Vec3f m_linearVelocity;
        Vec3f m_angularVelocity;

        Vec3f m_force;
        Vec3f m_torque;

        Vec3f m_colliderDisplacement;

        Mat3 m_invInertiaModel;
        Mat3 m_invInertiaWorld;

        RBTransform m_transform;
        Quaternion m_quat;

        std::shared_ptr<Shape> m_shape;

        bool m_collided;
        float m_solveTimeLeft;

        bool m_awake;

        bool m_collidable = true;
    };
}
