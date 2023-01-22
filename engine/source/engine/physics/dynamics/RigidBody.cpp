#include "engine-precompiled-header.h"
#include "RigidBody.h"
#include "engine/physics/Circle.h"
#include "engine/physics/AABB.h"
#include "engine/physics/OOBB.h"
#include "engine/core/allocator/MemoryManager.h"

namespace longmarch
{
    RigidBody::RigidBody()
        : m_rbType(RBType::staticBody),
          m_restitution(1.0f),
          m_mass(1.0f),
          m_invMass(1.0f),
          m_linearDamping(0.0f),
          m_angularDamping(0.0f),
          m_sleepTime(0.0f),
          m_gravityScale(1.0f),
          m_friction(0.0f),
          m_shape(nullptr),
          m_awake(false)
    {
    }

    void RigidBody::ApplyLinearForce(const Vec3f& force)
    {
        m_force += force * m_mass;

        SetAwake();
    }

    void RigidBody::ApplyForceAtWorldPoint(const Vec3f& force, const Vec3f& point)
    {
        m_force += force * m_mass;

        m_torque += glm::cross(point - m_transform.m_pos, force);

        SetAwake();
    }

    void RigidBody::ApplyLinearImpulse(const Vec3f& impulse)
    {
        m_linearVelocity += impulse * m_invMass;

        SetAwake();
    }

    void RigidBody::ApplyLinearImpulseAtWorldPoint(const Vec3f& impulse, const Vec3f& point)
    {
        // TODO
        m_linearVelocity += impulse * m_invMass;

        m_angularVelocity += m_invInertiaWorld * glm::cross(point - m_transform.m_pos, impulse);

        SetAwake();
    }

    void RigidBody::ApplyTorque(const Vec3f& torque)
    {
        m_torque += torque;
    }

    void RigidBody::SetAwake()
    {
        m_awake = true;
    }

    void RigidBody::Sleep()
    {
        m_awake = false;
    }

    bool RigidBody::IsAwake() const
    {
        return m_awake;
    }

    float RigidBody::GetMass() const
    {
        return m_mass;
    }

    float RigidBody::GetInvMass() const
    {
        return m_invMass;
    }

    const Vec3f& RigidBody::GetLinearVelocity() const
    {
        return m_linearVelocity;
    }

    const Vec3f& RigidBody::GetWorldPosition() const
    {
        return m_transform.m_pos;
    }

    const Vec3f& RigidBody::GetPrevWorldPosition() const
    {
        return m_prevPos;
    }

    float RigidBody::GetLinearDamping() const
    {
        return m_linearDamping;
    }

    float RigidBody::GetAngularDamping() const
    {
        return m_angularDamping;
    }

    const Vec3f RigidBody::GetLinearAcceleration() const
    {
        return m_force * m_invMass;
    }

    float RigidBody::GetRestitution() const
    {
        return m_restitution;
    }

    float RigidBody::GetGravityScale() const
    {
        return m_gravityScale;
    }

    float RigidBody::GetFriction() const
    {
        return m_friction;
    }

    Vec3f RigidBody::GetColliderDisplacement() const
    {
        return m_colliderDisplacement;
    }

    Vec3f RigidBody::GetAABBWidths() const
    {
        if (m_shape == nullptr)
        {
            Vec3f aabbExtents;
            switch (m_shape->GetType())
            {
            case Shape::SHAPE_TYPE::AABB:
                {
                    auto aabbShape = std::static_pointer_cast<AABB>(m_shape);
                    aabbExtents = aabbShape->GetMax() - aabbShape->GetMin();
                }
                break;
            default:
                ENGINE_EXCEPT(L"Logic error!");
                break;
            }
            return aabbExtents;
        }
        else
        {
            return Vec3f();
        }
    }

    const Entity& RigidBody::GetEntity() const
    {
        return m_entity;
    }

    void RigidBody::SetMass(float mass)
    {
        m_mass = mass;

        // epsilon introduced to handle the case where mass is 0)
        m_invMass = 1.0f / (mass + FLT_EPSILON);
    }

    void RigidBody::SetLinearVelocity(const Vec3f& velocity)
    {
        //ENGINE_EXCEPT_IF(m_rbType == RBType::staticBody, L"Cannot change linear velocity of static rigid bodies!");

        m_linearVelocity = velocity;
    }

    void RigidBody::SetAngularVelocity(const Vec3f& velocity)
    {
        //ENGINE_EXCEPT_IF(m_rbType == RBType::staticBody, L"Cannot change angular velocity of static rigid bodies!");

        m_angularVelocity = velocity;
    }

    void RigidBody::SetLinearDamping(float damping)
    {
        m_linearDamping = damping;
    }

    void RigidBody::SetAngularDamping(float damping)
    {
        m_angularDamping = damping;
    }

    void RigidBody::SetRestitution(float restitution)
    {
        m_restitution = restitution;
    }

    void RigidBody::SetWorldPosition(const Vec3f& pos)
    {
        m_transform.m_pos = pos;
    }

    void RigidBody::SetPrevWorldPosition(const Vec3f& pos)
    {
        m_prevPos = pos;
    }

    void RigidBody::SetWorldRotation(const Quaternion& rot)
    {
        m_transform.m_rot = rot;
    }

    void RigidBody::SetWorldScale(const Vec3f& scale)
    {
        m_transform.m_scale = scale;
    }

    void RigidBody::SetGravityScale(float gravityScale)
    {
        m_gravityScale = gravityScale;
    }

    void RigidBody::SetFriction(float friction)
    {
        m_friction = friction;
    }

    void RigidBody::UpdateAABBShape()
    {
        m_shape->SetModelTrAndUpdate(
                        Geommath::ToTransformMatrix(m_transform.m_pos, m_transform.m_rot, m_transform.m_scale));
    }

    void RigidBody::SetAABBShape(const Vec3f& aabbMin, const Vec3f& aabbMax)
    {
        // only set shape if it hasn't been set already
        ENGINE_EXCEPT_IF(m_shape != nullptr, L"Rigid Body already has shape assigned!");

        std::shared_ptr<AABB> tempPtr = MemoryManager::Make_shared<AABB>();

        tempPtr->SetOriginalMin(aabbMin);
        tempPtr->SetOriginalMax(aabbMax);

        //SetColliderDisplacement(tempPtr->GetCenter() - GetWorldPosition());
        m_colliderDisplacement = tempPtr->GetCenter() - m_transform.m_pos;

        //SetColliderDisplacement(tempPtr->GetCenter() - GetWorldPosition());
        m_colliderDisplacement = tempPtr->GetCenter() - m_transform.m_pos;

        m_shape = tempPtr;
    }

    const std::shared_ptr<Shape> RigidBody::GetShape() const
    {
        return m_shape;
    }

    void RigidBody::SetColliderDisplacement(const Vec3f& displacement)
    {
        m_colliderDisplacement = displacement;
    }

    void RigidBody::SetCollisionStatus(bool collided, float solveTimeLeft)
    {
        m_collided = collided;
        m_solveTimeLeft = solveTimeLeft;
    }

    void RigidBody::SetRBTrans(const Mat4& trans)
    {
        m_transform.m_pos = Geommath::GetTranslation(trans);
        m_transform.m_rot = Geommath::GetRotation(trans);
        m_transform.m_scale = Geommath::GetScale(trans);
    }

    const RBTransform& RigidBody::GetRBTrans() const
    {
        return m_transform;
    }

    bool RigidBody::IsCollided() const
    {
        return m_collided;
    }

    float RigidBody::GetSolveTimeLeft() const
    {
        return m_solveTimeLeft;
    }

    void RigidBody::ClearAllForces()
    {
        m_force = Vec3f(0.0f, 0.0f, 0.0f);
        m_torque = m_force;
    }

    void RigidBody::SetEntity(const Entity& entity)
    {
        m_entity = entity;
    }

    void RigidBody::Render()
    {
        if (m_shape)
        {
            m_shape->RenderShape();
        }
    }

    RBType RigidBody::GetRBType() const
    {
        return m_rbType;
    }

    void RigidBody::SetRBType(RBType type)
    {
        m_rbType = type;
    }
}
