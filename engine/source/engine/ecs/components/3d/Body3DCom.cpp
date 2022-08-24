#include "engine-precompiled-header.h"
#include "Body3DCom.h"
#include "engine/ecs/object-factory/ObjectFactory.h"

longmarch::Body3DCom::Body3DCom(const Entity& e)
	: 
	m_this(e)
{
}

const std::shared_ptr<Shape>& longmarch::Body3DCom::GetBoundingVolume() const
{
	LOCK_GUARD();
	return m_boundingVolume;
}

bool longmarch::Body3DCom::HasRigidBody() const
{
	LOCK_GUARD();
	return m_rigidBody != nullptr;
}

RBType longmarch::Body3DCom::GetRigidBodyType() const
{
	LOCK_GUARD();
	if (m_rigidBody)
	{
		return m_rigidBody->GetRBType();
	}
	return RBType::EMPTY;
}

void longmarch::Body3DCom::AssignRigidBody(const std::shared_ptr<RigidBody>& rb)
{
	LOCK_GUARD();
	m_rigidBody = rb;
}

void longmarch::Body3DCom::UnassignRigidBody()
{
	LOCK_GUARD();
	m_rigidBody = nullptr;
}

void longmarch::Body3DCom::UpdateRigidBody()
{
	LOCK_GUARD();
	if (m_rigidBody)
	{
		m_rigidBody->SetMass(m_mass);
		m_rigidBody->UpdateAABBShape();
	}
}

void longmarch::Body3DCom::UpdateBody3DCom()
{
	LOCK_GUARD();
	if (m_rigidBody)
	{
		m_mass = m_rigidBody->GetMass();
		m_invMass = m_rigidBody->GetInvMass();
	}
}

const longmarch::RBTransform& longmarch::Body3DCom::GetRBTrans() const
{
	LOCK_GUARD();
	ENGINE_EXCEPT_IF(m_rigidBody == nullptr, L"Trying to access Rigid Body Transform but Rigid Body does not exist!");

	return m_rigidBody->GetRBTrans();
}

bool longmarch::Body3DCom::IsRBAwake() const
{
	LOCK_GUARD();
	if (!m_rigidBody)
	{
		return false;
	}
	return m_rigidBody->IsAwake();
}

void longmarch::Body3DCom::JsonSerialize(Json::Value& value) const
{
	LOCK_GUARD();
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	{
		Json::Value output;
		output["id"] = "Body3DCom";
		auto& val = output["value"];

		static const auto& _default = Body3DCom();
		{
			auto& rigigBodyData = val["rigid-body"];
			{
				if (m_rigidBodyInfo.type != _default.m_rigidBodyInfo.type)
				{
					switch (m_rigidBodyInfo.type)
					{
					case RBType::noCollision:
						rigigBodyData["type"] = "no_collision";
						break;
					case RBType::staticBody:
						rigigBodyData["type"] = "static";
						break;
					case RBType::dynamicBody:
						rigigBodyData["type"] = "dynamic";
						break;
					default:
						ENGINE_EXCEPT(L"Logic Error!");
						break;
					}
				}
				if (m_rigidBodyInfo.mass != _default.m_rigidBodyInfo.mass)
				{
					rigigBodyData["mass"] = m_rigidBodyInfo.mass;
				}
				if (m_rigidBodyInfo.restitution != _default.m_rigidBodyInfo.restitution)
				{
					rigigBodyData["restitution"] = m_rigidBodyInfo.restitution;
				}
				if (m_rigidBodyInfo.linearDamping != _default.m_rigidBodyInfo.linearDamping)
				{
					rigigBodyData["linear-damping"] = m_rigidBodyInfo.linearDamping;
				}
				if (m_rigidBodyInfo.friction != _default.m_rigidBodyInfo.friction)
				{
					rigigBodyData["friction"] = m_rigidBodyInfo.friction;
				}
				if (m_rigidBodyInfo.linearVelocity != _default.m_rigidBodyInfo.linearVelocity)
				{
					rigigBodyData["linear-velocity"] = LongMarch_ArrayToJsonValue(m_rigidBodyInfo.linearVelocity, 3);
				}
				if (m_rigidBodyInfo.colliderDimensionExtent != _default.m_rigidBodyInfo.colliderDimensionExtent)
				{
					rigigBodyData["collider-extent"] = m_rigidBodyInfo.colliderDimensionExtent;
				}
				if (m_rigidBodyInfo.entityTypeIngoreSet != _default.m_rigidBodyInfo.entityTypeIngoreSet)
				{
					LongMarch_Vector<std::string> vec;
					for (auto& type : m_rigidBodyInfo.entityTypeIngoreSet)
					{
						vec.push_back(ObjectFactory::s_instance->GetEntityNameFromType(type));
					}
					rigigBodyData["type-to-ingore"] = LongMarch_ArrayToJsonValue(vec, vec.size());
				}
			}
		}

		{
			value.append(std::move(output));
		}
	}
}

void longmarch::Body3DCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	LOCK_GUARD();
	{
		if (auto& bv = value["bounding-volume"]; !bv.isNull())
		{
			if (auto& val = bv["type"]; !val.isNull())
			{
				auto bodyType = val.asString();
				if (bodyType == "AABB")
				{
					auto min_val = bv["min"];
					ASSERT(min_val.size() == 3, "must be a vec3!");
					Vec3f min(min_val[0].asFloat(), min_val[1].asFloat(), min_val[2].asFloat());
					auto max_val = bv["max"];
					ASSERT(max_val.size() == 3, "must be a vec3!");
					Vec3f max(max_val[0].asFloat(), max_val[1].asFloat(), max_val[2].asFloat());
					m_boundingVolume = MemoryManager::Make_shared<AABB>(min, max);
				}
			}
		}

		if (auto& rigigBodyData = value["rigid-body"]; !rigigBodyData.isNull())
		{
			if (auto& val = rigigBodyData["type"]; !val.isNull())
			{
				auto bodyType = val.asString();
				if (bodyType == "dynamic")
				{
					m_rigidBodyInfo.type = RBType::dynamicBody;
				}
				else if (bodyType == "static")
				{
					m_rigidBodyInfo.type = RBType::staticBody;
				}
				else if (bodyType == "no_collision")
				{
					m_rigidBodyInfo.type = RBType::noCollision;
				}
			}

			if (auto& val = rigigBodyData["mass"]; !val.isNull())
			{
				m_rigidBodyInfo.mass = val.asFloat();
			}

			if (auto& val = rigigBodyData["restitution"]; !val.isNull())
			{
				m_rigidBodyInfo.restitution = val.asFloat();
			}

			if (auto& val = rigigBodyData["linear-damping"]; !val.isNull())
			{
				m_rigidBodyInfo.linearDamping = val.asFloat();
			}

			if (auto& val = rigigBodyData["linear-friction"]; !val.isNull())
			{
				m_rigidBodyInfo.friction = val.asFloat();
			}

			if (auto& val = rigigBodyData["linear-velocity"]; !val.isNull())
			{
				ASSERT(val.size() == 3, "must be a vec3!");
				m_rigidBodyInfo.linearVelocity.x = val[0].asFloat();
				m_rigidBodyInfo.linearVelocity.y = val[1].asFloat();
				m_rigidBodyInfo.linearVelocity.z = val[2].asFloat();
			}

			if (auto& val = rigigBodyData["collider-extent"]; !val.isNull())
			{
				m_rigidBodyInfo.colliderDimensionExtent = val.asFloat();
			}

			if (auto& val = rigigBodyData["type-to-ingore"]; !val.isNull())
			{
				for (int i = 0; i < val.size(); ++i)
				{
					m_rigidBodyInfo.entityTypeIngoreSet.emplace(ObjectFactory::s_instance->GetEntityTypeFromName(val[i].asString()));
				}
			}
		}
	}

	{
		m_mass = m_rigidBodyInfo.mass;
		m_invMass = m_invMass = 1.0f / (m_mass + FLT_EPSILON);
	}
}

void longmarch::Body3DCom::ImGuiRender()
{

}
