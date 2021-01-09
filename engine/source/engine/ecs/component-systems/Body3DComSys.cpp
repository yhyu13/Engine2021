#include "engine-precompiled-header.h"
#include "Body3DComSys.h"
#include "Scene3DComSys.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

AAAAgames::Body3DComSys::Body3DComSys() 
{
	m_systemSignature.AddComponent<Transform3DCom>();
	m_systemSignature.AddComponent<Body3DCom>();
}

AAAAgames::Body3DComSys::~Body3DComSys()
{
	PhysicsManager::GetInstance()->DeleteScene(m_scene);
}

void AAAAgames::Body3DComSys::Init()
{
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<Body3DComSys>(this, EngineEventType::GC, &Body3DComSys::_ON_GC));
		ManageEventSubHandle(queue->Subscribe<Body3DComSys>(this, EngineEventType::GC_RECURSIVE, &Body3DComSys::_ON_GC_RECURSIVE));
	}
	// store ptr to physics manager for updating bodies
	if (!m_scene)
	{
		m_scene = PhysicsManager::GetInstance()->CreateScene();
		const auto& engineConfiguration = FileSystem::GetCachedJsonCPP("$root:engine-config.json");
		if (auto& gravity = engineConfiguration["physics"]["gravity"]; !gravity.isNull())
		{
			m_scene->SetGravity(Vec3f(gravity[0].asFloat(), gravity[1].asFloat(), gravity[2].asFloat()));
		}
		else
		{
			m_scene->SetGravity(Vec3f(0, 0, -9.8));
		}
	}
	else
	{
		PhysicsManager::GetInstance()->AddScene(m_scene);
	}
	m_scene->SetGameWorld(m_parentWorld);
}

void AAAAgames::Body3DComSys::PreRenderUpdate(double dt)
{
	EARLY_RETURN(dt);
	/**************************************************************
	*	Initialize bounding volume
	**************************************************************/
	/*
	The reason to only init BV here is because PreRenderUpdate() is always called after all Update()
	are finished, inlcuding the creation of entities in this frame. So we are guranteed that the bounding volume could be initialized before it is rendered.

	Seocondly, an optional optimization in the MeshData class would destory all vertices data after data
	has been transfered to the GPU. So we need to initialze the bounding volume before it is rendered.
	*/

	auto entities = m_bufferedRegisteredEntities;

	ParEach(
		[this](EntityDecorator e)
	{
		// Generate view frustum culling BV if possible
		auto scene = e.GetComponent<Scene3DCom>();
		auto body = e.GetComponent<Body3DCom>();
		auto trans = e.GetComponent<Transform3DCom>();
		if (auto bv = body->GetBV(); !bv)
		{
			if (const auto& data = scene->GetSceneData(false); data)
			{
				auto& meshes = data->GetAllMesh();
				body->CreateBV<AABB>(meshes);
				body->GetBV()->SetOwnerEntity(e);
			}
		}
		if (auto bv = body->GetBV(); bv)
		{
			// Update view frustum culling BV
			bv->SetModelTrAndUpdate(trans->GetModelTr());
		}
		// check to see if there is a corresponding rigid body in the scene, if not then assign one and update with body info
		if (!body->HasRigidBody() && body->m_bodyInfo.type != RBType::noCollision)
		{
			// Generate rigid body BV if possible
			auto body = e.GetComponent<Body3DCom>();
			auto trans = e.GetComponent<Transform3DCom>();
			if (auto aabbPtr = std::dynamic_pointer_cast<AABB>(body->GetBV()); aabbPtr)
			{
				std::shared_ptr<RigidBody> newRB = m_scene->CreateRigidBody();
				switch (body->m_bodyInfo.type)
				{
				case RBType::dynamicBody:
				{
					newRB->SetAwake();
					newRB->SetRBType(RBType::dynamicBody);
					newRB->SetMass(body->m_bodyInfo.mass);
				}
				break;
				case RBType::staticBody:
				{
					newRB->SetRBType(RBType::staticBody);
					newRB->SetMass(1e8);
				}
				break;
				default:
					ENGINE_EXCEPT(L"Logic error!");
					break;
				}
				newRB->SetFriction(body->m_bodyInfo.friction);
				newRB->SetRestitution(body->m_bodyInfo.restitution);
				newRB->SetLinearDamping(body->m_bodyInfo.linearDamping);
				const float scale = body->m_bodyInfo.colliderDimensionExtent;
				newRB->SetAABBShape(aabbPtr->GetOriginalMin() * scale, aabbPtr->GetOriginalMax() * scale);
				newRB->SetEntity(e.GetEntity());
				newRB->m_entityTypeIngoreSet.AddIndex(body->m_bodyInfo.entityTypeIngoreSet);
				body->AssignRigidBody(newRB);
			}
		}
		if (body->HasRigidBody())
		{
			// Update rigid body BV
			// Assign transformCom to rigid body
			body->m_body->SetRBTrans(trans->GetModelTr());
			body->m_body->SetLinearVelocity(trans->GetGlobalVel());
			body->UpdateBody3DCom();
			body->UpdateRigidBody();
		}
	}
	).wait();
	{
		// for debugging purposes
		m_scene->RenderDebug();
	}
}

void AAAAgames::Body3DComSys::Update(double dt)
{
	EARLY_RETURN(dt);
	// Update physical scene here instead from PhysicsManager
	if (m_scene->IsUpdateEnabled())
	{
		m_scene->Step(dt);
	}
	ParEach(
		[this](EntityDecorator e)
		{
			auto body = e.GetComponent<Body3DCom>();
			auto trans = e.GetComponent<Transform3DCom>();

			if (body->HasRigidBody())
			{
				// Assign simulated rigid body back to transformCom
				const RBTransform& rbTrans = body->GetRBTrans();
				trans->SetGlobalPos(rbTrans.m_pos);
				//trans->SetGlobalRot(rbTrans.m_rot); // Rotation is not implemented in the physics engine
				trans->SetGlobalVel(body->m_body->GetLinearVelocity());
			}
		}
	).wait();
}

std::shared_ptr<BaseComponentSystem> AAAAgames::Body3DComSys::Copy() const
{
	LOCK_GUARD_NC();
	auto ret = MemoryManager::Make_shared<Body3DComSys>();
	ret->m_bufferedRegisteredEntities = m_bufferedRegisteredEntities; // Must copy
	ret->m_systemSignature = m_systemSignature; // Must copy
	ret->m_scene = MemoryManager::Make_shared<Scene>(*m_scene); // Custom variables
	return ret;
}

void AAAAgames::Body3DComSys::_ON_GC(EventQueue<EngineEventType>::EventPtr e)
{
	LOCK_GUARD_NC();
	auto event = std::static_pointer_cast<EngineGCEvent>(e);
	if (event->m_entity.Valid() && m_parentWorld == event->m_entity.GetWorld())
	{
		if (auto body = event->m_entity.GetComponent<Body3DCom>(); body.Valid())
		{
			m_scene->RemoveRigidBody(body->m_body);
		}
	}
}

void AAAAgames::Body3DComSys::_ON_GC_RECURSIVE(EventQueue<EngineEventType>::EventPtr e)
{
	LOCK_GUARD_NC();
	auto event = std::static_pointer_cast<EngineGCRecursiveEvent>(e);
	if (event->m_entity.Valid() && m_parentWorld == event->m_entity.GetWorld())
	{
		GCRecursive(event->m_entity);
	}
}

void AAAAgames::Body3DComSys::GCRecursive(EntityDecorator e)
{
	if (auto body = e.GetComponent<Body3DCom>(); body.Valid())
	{
		m_scene->RemoveRigidBody(body->m_body);
	}
	for (auto& child : m_parentWorld->GetComponent<ChildrenCom>(e)->GetChildren())
	{
		GCRecursive(EntityDecorator{ child ,e.GetWorld() });
	}
}
