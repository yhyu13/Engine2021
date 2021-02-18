#include "engine-precompiled-header.h"
#include "Transform3DComSys.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/ChildrenCom.h"
#include "engine/ecs/components/ParentCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/IDNameCom.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#ifdef DEBUG_DRAW
void longmarch::Transform3DComSys::DebugDraw(EntityDecorator e)
{
	auto trans = e.GetComponent<Transform3DCom>();
	if (trans->debug.showRotation)
	{
		if (!trans->debug.initRotation)
		{
			trans->debug.initRotation = true;
			auto parent = EntityDecorator{ trans->m_this, trans->m_world };
			auto world = parent.GetVolatileWorld();
			auto node_ = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::DEBUG_OBJ, true, false);
			/*
			Entity generation logics should happen in component-system instead of in component because it might trigger a component data reallocation that invalidate the component data on our hands.
			Let system generate a new entity and get a fresh new component afterward to avoid invalidated component data.
			*/
			trans.Update();
			trans->debug.rotationNode = node_;
			{
				auto parentChildCom = parent.GetComponent<ChildrenCom>();
				parentChildCom->AddEntity(node_);
				auto rm = ResourceManager<Scene3DNode>::GetInstance();
				auto meshName = "debugArrow";
				auto mesh = rm->TryGet(meshName)->Get()->Copy();
				auto scene = node_.GetComponent<Scene3DCom>();
				scene->SetVisiable(true);
				scene->SetCastShadow(false);
				scene->SetCastReflection(false);
				mesh->ModifyAllMaterial([&](Material* material)
				{
					material->Kd = Vec3f(0.85, 0.1, 0.1);
				});
				scene->SetSceneData(mesh);

				auto id = node_.GetComponent<IDNameCom>();
				id->SetName("Rotation_Arrow");

				auto trans = node_.GetComponent<Transform3DCom>();
				trans->m_apply_parent_rot = false;
				trans->m_apply_parent_trans = true;
				trans->m_apply_parent_scale = false;
				trans->SetRelativeToParentPos(Vec3f(0, 0, 1.0f));
			}
		}
		else
		{
			auto parent = EntityDecorator{ trans->m_this, trans->m_world };
			auto parentTransCom = parent.GetComponent<Transform3DCom>();
			auto parentRotation = parentTransCom->GetGlobalRot();
			auto node_ = trans->debug.rotationNode;
			{
				auto trans = node_.GetComponent<Transform3DCom>();
				trans->SetGlobalRot(parentRotation);
			}
		}
	}
	else
	{
		if (trans->debug.initRotation)
		{
			trans->debug.initRotation = false;
			auto queue = EventQueue<EngineEventType>::GetInstance();
			auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(trans->debug.rotationNode);
			queue->Publish(e);
		}
	}
	if (trans->debug.showVelocity)
	{
		if (!trans->debug.initVelocity)
		{
			trans->debug.initVelocity = true;
			auto parent = EntityDecorator{ trans->m_this, trans->m_world };
			auto world = parent.GetVolatileWorld();
			auto node_ = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::DEBUG_OBJ, true, false);

			trans.Update();
			trans->debug.velocityNode = node_;
			{
				auto parentChildCom = parent.GetComponent<ChildrenCom>();
				parentChildCom->AddEntity(node_);
				auto rm = ResourceManager<Scene3DNode>::GetInstance();
				auto meshName = "debugArrow";
				auto mesh = rm->TryGet(meshName)->Get()->Copy();
				auto scene = node_.GetComponent<Scene3DCom>();
				scene->SetVisiable(true);
				scene->SetCastShadow(false);
				scene->SetCastReflection(false);
				mesh->ModifyAllMaterial([&](Material* material)
				{
					material->Kd = Vec3f(0.1, 0.1, 0.85);
				});
				scene->SetSceneData(mesh);

				auto id = node_.GetComponent<IDNameCom>();
				id->SetName("Velocity_Arrow");

				{
					auto trans = node_.GetComponent<Transform3DCom>();
					trans->m_apply_parent_rot = false;
					trans->m_apply_parent_trans = true;
					trans->m_apply_parent_scale = false;
					trans->SetRelativeToParentPos(Vec3f(0, 0, 0.1f));
				}
			}
		}
		else
		{
			auto parent = EntityDecorator{ trans->m_this, trans->m_world };
			auto parentTransCom = parent.GetComponent<Transform3DCom>();
			auto parentVelocity = parentTransCom->GetGlobalVel();
			auto speed = Geommath::Length(parentVelocity);
			auto node_ = trans->debug.velocityNode;
			{
				auto trans = node_.GetComponent<Transform3DCom>();
				trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, parentVelocity));
				trans->SetLocalScale(Vec3f(1, MAX(speed, 0.1), 1));
			}
		}
	}
	else
	{
		if (trans->debug.initVelocity)
		{
			trans->debug.initVelocity = false;
			auto queue = EventQueue<EngineEventType>::GetInstance();
			auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(trans->debug.velocityNode);
			queue->Publish(e);
		}
	}
}
#endif