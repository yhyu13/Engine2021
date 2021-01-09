#pragma once

#include "engine/core/utility/TypeHelper.h"
#include "engine/physics/Shape.h"
#include "engine/physics/RBTransform.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/Entity.h"

namespace AAAAgames
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

		f32 GetMass() const;
		f32 GetInvMass() const;

		const Vec3f& GetLinearVelocity() const;
		const Vec3f& GetWorldPosition() const;
		const Vec3f& GetPrevWorldPosition() const;

		f32 GetLinearDamping() const;
		f32 GetAngularDamping() const;

		const Vec3f GetLinearAcceleration() const;

		f32 GetRestitution() const;

		f32 GetGravityScale() const;
		f32 GetFriction() const;

		const Entity& GetEntity() const;
		void SetEntity(const Entity& entity);

		void SetMass(f32 mass);
		void SetLinearVelocity(const Vec3f& velocity);
		void SetAngularVelocity(const Vec3f& velocity);

		void SetLinearDamping(f32 damping);
		void SetAngularDamping(f32 damping);

		void SetRestitution(f32 restitution);

		void SetGravityScale(f32 gravityScale);
		void SetFriction(f32 friction);

		void SetWorldPosition(const Vec3f& pos);
		void SetPrevWorldPosition(const Vec3f& pos);

        void SetWorldRotation(const Quaternion& rot);
        void SetWorldScale(const Vec3f& scale);
        void SetWorldTransform(const Mat4& trans);

        void UpdateAABBShape();        
		std::shared_ptr<Shape>& GetShape();
		void SetAABBShape(const Vec3f& aabbMin, const Vec3f& aabbMax);
		const Vec3f GetAABBWidths() const;

		void SetColliderDisplacement(const Vec3f& displacement);
		const Vec3f& GetColliderDisplacement() const;


        void SetRBTrans(const Mat4& trans);
        const RBTransform& GetRBTrans() const;

		RBType GetRBType() const;
		void SetRBType(RBType type);

		void SetCollisionStatus(bool collided, f32 solveTimeLeft);
		bool IsCollided() const;
		float GetSolveTimeLeft() const;

		void ClearAllForces();

		// Render the bounding volume
		void Render();

	public:
		A4GAMES_Bitset<EntityType> m_entityTypeIngoreSet;

	private:
		// params still needed: flags (tbd)
		RBType m_rbType;

		Entity m_entity;

		// usage to be decided
		u32 m_islandIndex;

		f32 m_restitution;

		f32 m_mass;
		f32 m_invMass;

		f32 m_linearDamping;
		f32 m_angularDamping;

		f32 m_sleepTime;
		f32 m_gravityScale;

		f32 m_friction;

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