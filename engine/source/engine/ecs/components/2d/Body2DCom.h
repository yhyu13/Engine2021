#pragma once
#include <memory>
#include "engine/ecs/BaseComponent.h"
#include "engine/core/exception/EngineException.h"
#include "Transform2DCom.h"
#include "engine/ecs/Entity.h"
#include "engine/physics/CollisionsManager.h"

namespace longmarch
{
	
	struct CACHE_ALIGN Body2DCom final : BaseComponent<Body2DCom>
	{
	public:
		Body2DCom() = default;
		Body2DCom(float posx, float posy)
			:m_PosX(posx), m_PosY(posy), m_Mass(0), m_AccX(0), m_AccY(0),
			 m_VelX(0), m_VelY(0), m_PrevPosX(0), m_PrevPosY(0), m_InvMass(0),
			 m_TotalForceX(0), m_TotalForceY(0),m_Restitution(0), m_overrideFriction(false)
		{
		};

		Body2DCom& CopyBody2DCom(const Body2DCom& Body2DCom)
		{
			if (this != &Body2DCom)
			{
				m_PosX = Body2DCom.m_PosX;
				m_PosY = Body2DCom.m_PosY;
				m_Mass = Body2DCom.m_Mass;
				m_AccX = Body2DCom.m_AccX;
				m_AccY = Body2DCom.m_AccY;
				m_VelX = Body2DCom.m_VelX;
				m_VelY = Body2DCom.m_VelY;
				m_InvMass = Body2DCom.m_InvMass;
				m_PrevPosX = Body2DCom.m_PrevPosX;
				m_PrevPosY = Body2DCom.m_PrevPosY;
				m_TotalForceX = Body2DCom.m_TotalForceX;
				m_TotalForceY = Body2DCom.m_TotalForceY;
				m_Restitution = Body2DCom.m_Restitution;
				shape = Body2DCom.shape;
				m_overrideFriction = Body2DCom.m_overrideFriction;
			}
			return *this;
		}

	public:
		float m_PosX;
		float m_PosY;
		float m_PosZ;
		float m_Mass;
		float m_AccX;
		float m_AccY;
		float m_VelX;
		float m_VelY;
		float m_PrevPosX;
		float m_PrevPosY;
		float m_InvMass;
		float m_TotalForceX;
		float m_TotalForceY;
		float m_Restitution;
		std::shared_ptr<Shape> shape;
		Entity m_otherEntity;
		bool m_overrideFriction;

	public:

		void SetAcceleration(float x, float y)
		{
			m_AccX = x;
			m_AccY = y;
		}

		void SetTotalForce(float x, float y)
		{
			m_TotalForceX = x;
			m_TotalForceY = y;
		}

		void SetTotalForceX(float x)
		{
			m_TotalForceX = x;
		}

		void SetTotalForceY(float y)
		{
			m_TotalForceY = y;
		}

		void AddTotalForce(float x, float y)
		{
			m_TotalForceX += x;
			m_TotalForceY += y;
		}

		void AddTotalForceX(float x)
		{
			m_TotalForceX += x;
		}

		void AddTotalForceY(float y)
		{
			m_TotalForceX += y;
		}


		void ResetOtherEntity()
		{
			m_otherEntity.m_type = 0;
		}

		void SetOtherEntity(const Entity& other)
		{
			m_otherEntity = other;
		}

		const Entity& GetOtherEntity()
		{
			return m_otherEntity;
		}

		//Choose Shape, fill in width and height for the body
		template<typename Shape, typename ...Args>
		void ChooseShape(Shape s, Args... args)
		{
			LOCK_GUARD()
			shape = MemoryManager::Make_shared<s>(args);
		}

		void SetVelocity(float VelX, float VelY)
		{
			m_VelX = VelX;
			m_VelY = VelY;
		}

		void SetVelocity(Vec2f& velocity)
		{
			m_VelX = velocity.x;
			m_VelY = velocity.y;
		}

		void SetPos3D(const Vec3f& pos)
		{
			m_PosX = pos.x;
			m_PosY = pos.y;
			m_PosZ = pos.z;
		}

		const Vec3f& GetPos3D()
		{
			return Vec3f(m_PosX, m_PosY, m_PosZ);
		}

		void SetRestitution(float rest)
		{
			m_Restitution = rest;
		}

		void SetMass(float mass)
		{
			m_Mass = mass;
		}

		void SetPos(const Vec2f& v)
		{
			m_PosX = v.x;
			m_PosY = v.y;
		}

		const Vec2f GetPos() const
		{
			return Vec2f(m_PosX, m_PosY);
		}

		const Vec2f GetVelocity() const
		{
			return Vec2f(m_VelX, m_VelY);
		}

		void Integrate(float dt)
		{
			//m_AccX = m_AccY = 0.0f;

			//dt = dt / 1000.0f;

			m_PrevPosX = m_PosX;
			m_PrevPosY = m_PosY;

			if (m_Mass == 0)
				m_InvMass = 0;
			else
				m_InvMass = 1 / m_Mass;

			//Applying Gravity
			//float g = m_Mass * Gravity;
			//m_TotalForceY = /* m_AddedForceY * */g;

			//Apply Acceleration
			//m_AccX = m_TotalForceX * m_InvMass;
			//m_AccY = m_TotalForceY * m_InvMass;

			//Change in Position
			m_PosX = m_VelX * dt + m_PrevPosX;
			m_PosY = m_VelY * dt + m_PrevPosY;

			//Simulating Friction
			if (!m_overrideFriction)
			{
				m_VelX *= 0.99f;
				m_VelY *= 0.99f;
			}

			//Nullifying All Forces To Activate with Press of a button
			//m_TotalForceX = m_TotalForceY = 0.0f;
		}
	};
}
