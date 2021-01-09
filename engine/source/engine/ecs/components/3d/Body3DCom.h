#pragma once
#include <memory>

#include "engine/ecs/ComponentDecorator.h"
#include "engine/core/exception/EngineException.h"
#include "Transform3DCom.h"
#include "engine/ecs/Entity.h"
#include "engine/physics/CollisionsManager.h"
#include "engine/physics/dynamics/RigidBody.h"
#include "engine/physics/Scene.h"

namespace longmarch
{
	struct RigidBodyInfo
	{
		LongMarch_Set<EntityType> entityTypeIngoreSet;
		Vec3f linearVelocity{ 0.f };
		RBType type{RBType::staticBody};
		float mass{1.0f};
		float restitution{1.0f};
		float linearDamping{0.f};
		float friction{0.f};
		float colliderDimensionExtent{ 0.75f };
	};

	struct CACHE_ALIGN32 Body3DCom final : BaseComponent<Body3DCom>
	{
	public:
		Body3DCom() = default;
		explicit Body3DCom(const Entity& e);

		//Choose Shape, fill in width and height for the body
		template<typename Shape, typename ...Args>
		void CreateBV(Args... args)
		{
			LOCK_GUARD2();
			m_boundingVolume = MemoryManager::Make_shared<Shape>(args...);
		}
		std::shared_ptr<Shape> GetBV();

		void ResetOtherEntity();
		void SetOtherEntity(const Entity& other);
		const Entity& GetOtherEntity();

		//void Integrate(float dt);
		bool HasRigidBody() const;
		RBType GetRigidBodyType() const;

		void AssignRigidBody(const std::shared_ptr<RigidBody>& rb);
		void UnassignRigidBody();

		// function to update the corresponding rigid body using Body3DCom data
		void UpdateRigidBody();

		// function to update the Body3DCom based on the rigid body
		void UpdateBody3DCom();

		const RBTransform& GetRBTrans() const;

		bool IsRBAwake() const;

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		//virtual void ImGuiRender() override;
		//virtual void Copy(BaseComponentInterface* other) override;

	public:
		// just store mass here for now, position and rotation should be taken care of by transform component
		float m_mass;
		float m_invMass;

		Entity m_this;
		Entity m_otherEntity;
		
		std::shared_ptr<Shape> m_boundingVolume{ nullptr }; // For view frustum culling

		// Physics body variable
		std::shared_ptr<RigidBody> m_body{ nullptr };
		RigidBodyInfo m_bodyInfo;
	};
}