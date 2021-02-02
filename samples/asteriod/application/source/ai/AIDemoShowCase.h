#pragma once
#include "ecs/header/header.h"
#include "ui/header.h"
#include "application-precompiled-header.h"
#include "engine/ecs/header/header.h"
#include "ecs/header/header.h"

namespace longmarch
{
	void AIDemoShowCase() //Runs prefectly with taksh_scene-game & game2
	{
		//	auto world = GameWorld::GetCurrent();
		//	auto rm = ResourceManager<Scene3DNode>::GetInstance();

		//	auto bunny_mesh = rm->TryGet("bunny")->Get()->Copy();

		//	// Creating an entity
		//	auto root_1 = world->GenerateEntity3D((EntityType)(EngineEntityType::STATIC_OBJ), true, true);

		//	// Creating transform of the given entity
		//	auto trans = root_1.GetComponent<Transform3DCom>();
		//	trans->SetGlobalPos(Vec3f(0.0f, 0.0f, -110.8f)); // To reach the ground floor
		//	trans->SetLocalScale(Vec3f(30.0f, 30.0f, 30.0f)); // Let it have some size
		//	trans->SetGlobalRot(Quaternion(90.0f, 90.0f, 0.0f, 0.0f)); // rotate X-90 deg to stand upright

		//	// Creating a new scene variable to connect the mesh with
		//	auto scene2 = root_1.GetComponent<Scene3DCom>();
		//	scene2->SetVisiable(true);

		//	// Mesh's material data
		//	bunny_mesh->ModifyAllMaterial([&](Material* material) {
		//		material->Kd = Vec3f(1.0f, 0.2f, 0.2f);
		//		//material->Ks = 1.f * Vec3f(0.0f, 0.0f, 0.0f);
		//		material->roughness = 1.0f;
		//		material->metallic = 1.0f;
		//		material->emissive = false;
		//		});
		//	scene2->SetSceneData(bunny_mesh);

		//	// Adding AIControllerCom to the root entity and fetching it
		//	root_1.AddComponent(AIControllerCom(root_1));
		//	auto aicomp = root_1.GetComponent<AIControllerCom>();
		//	
		//	// Setting up BT and Blackboard
		//	aicomp->SetBT(("$asset:/BehaviorTrees/TestBehaviorTree.json"));

		//	// ****Another bunny using blackboard of previous to stay apart *****
		//	auto bunny_mesh2 = rm->TryGet("bunny")->Get()->Copy();

		//	// Creating an entity
		//	auto root_2 = world->GenerateEntity3D((EntityType)(GameEntityType::PLAYER), true, true);

		//	// Creating transform of the given entity
		//	auto trans2 = root_2.GetComponent<Transform3DCom>();
		//	trans2->SetGlobalPos(Vec3f(0.0f, 0.0f, -110.8f)); // To reach the ground floor
		//	trans2->SetLocalScale(Vec3f(30.0f, 30.0f, 30.0f)); // Let it have some size
		//	trans2->SetGlobalRot(Quaternion(90.0f, 90.0f, 0.0f, 0.0f)); // rotate X-90 deg to stand upright

		//	// Creating a new scene variable to connect the mesh with
		//	auto scene3 = root_2.GetComponent<Scene3DCom>();
		//	scene3->SetVisiable(true);

		//	// Mesh's material data
		//	bunny_mesh2->ModifyAllMaterial([&](Material* material) {
		//		material->Kd = Vec3f(0.5f, 0.2f, 0.2f);
		//		//material->Ks = 1.f * Vec3f(0.0f, 0.0f, 0.0f);
		//		material->roughness = 1.0f;
		//		material->metallic = 1.0f;
		//		material->emissive = false;
		//		});
		//	scene3->SetSceneData(bunny_mesh2);

		//	// Adding PlayerController to the second
		//	root_2.AddComponent(PlayerController());
		//	auto controller = root_2.GetComponent<PlayerController>();

		//	// Give Player the transform
		//	controller->SetPlayerTransform(trans2);

		//	// ****** Creating 2 of them to play a behavior of patrol ******
		//	auto bunny_mesh3 = rm->TryGet("bunny")->Get()->Copy();
		//	auto bunny_mesh4 = rm->TryGet("bunny")->Get()->Copy();

		//	auto root_3 = world->GenerateEntity3D((EntityType)(EngineEntityType::STATIC_OBJ), true, true);
		//	auto root_4 = world->GenerateEntity3D((EntityType)(EngineEntityType::STATIC_OBJ), true, true);

		//	auto trans3 = root_3.GetComponent<Transform3DCom>();
		//	trans3->SetGlobalPos(Vec3f(40.0f, 40.0f, -110.8f)); // To reach the ground floor
		//	trans3->SetLocalScale(Vec3f(30.0f, 30.0f, 30.0f)); // Let it have some size
		//	trans3->SetGlobalRot(Quaternion(90.0f, 90.0f, 0.0f, 0.0f)); // rotate X-90 deg to stand upright

		//	auto trans4 = root_4.GetComponent<Transform3DCom>();
		//	trans4->SetGlobalPos(Vec3f(-40.0f, -40.0f, -110.8f)); // To reach the ground floor
		//	trans4->SetLocalScale(Vec3f(30.0f, 30.0f, 30.0f)); // Let it have some size
		//	trans4->SetGlobalRot(Quaternion(90.0f, 90.0f, 0.0f, 0.0f)); // rotate X-90 deg to stand upright

		//	// Creating a new scene variable to connect the mesh with
		//	auto scene4 = root_3.GetComponent<Scene3DCom>();
		//	scene4->SetVisiable(true);

		//	// Creating a new scene variable to connect the mesh with
		//	auto scene5 = root_4.GetComponent<Scene3DCom>();
		//	scene5->SetVisiable(true);

		//	// Mesh's material data
		//	bunny_mesh3->ModifyAllMaterial([&](Material* material) {
		//		material->Kd = Vec3f(1.0f, 0.2f, 0.2f);
		//		//material->Ks = 1.f * Vec3f(0.0f, 0.0f, 0.0f);
		//		material->roughness = 1.0f;
		//		material->metallic = 1.0f;
		//		material->emissive = false;
		//		});
		//	scene4->SetSceneData(bunny_mesh3);

		//	// Mesh's material data
		//	bunny_mesh4->ModifyAllMaterial([&](Material* material) {
		//		material->Kd = Vec3f(1.0f, 0.2f, 0.2f);
		//		//material->Ks = 1.f * Vec3f(0.0f, 0.0f, 0.0f);
		//		material->roughness = 1.0f;
		//		material->metallic = 1.0f;
		//		material->emissive = false;
		//		});
		//	scene5->SetSceneData(bunny_mesh4);

		//	// Adding AIControllerCom to the root entity and fetching it
		//	root_3.AddComponent(AIControllerCom(root_3));
		//	auto aicomp2 = root_3.GetComponent<AIControllerCom>();

		//	// Setting up BT and Blackboard
		//	aicomp2->SetBT(("$asset:/BehaviorTrees/Patrol_in_XAndCatch.json"));

		//	// Adding AIControllerCom to the root entity and fetching it
		//	root_4.AddComponent(AIControllerCom(root_4));
		//	auto aicomp3 = root_4.GetComponent<AIControllerCom>();

		//	// Setting up BT and 
		//	aicomp3->SetBT(("$asset:/BehaviorTrees/Patrol_in_YAndCatch.json"));

		//	// One common Blackboard for all
		//	Blackboard bb;
		//	bb.set_info(trans2->GetGlobalPos()); // The one with controller component

		//	// Setting Up Floor
		//	auto floor_mesh = rm->TryGet("cube")->Get()->Copy();
		//	
		//	// Creating an entity
		//	auto root_5 = world->GenerateEntity3D((EntityType)(EngineEntityType::STATIC_OBJ), true, true);
		//	
		//	// Creating transform of the given entity
		//	auto trans5 = root_5.GetComponent<Transform3DCom>();
		//	trans5->SetGlobalPos(Vec3f(-50, -50, -111)); // To reach the ground floor
		//	trans5->SetLocalScale(Vec3f(100, 100, 0.1)); // Let it have some size
		//	trans5->SetGlobalRot(Quaternion(0.0f, 0.0f, 0.0f, 0.0f)); // rotate X-90 deg to stand upright
		//	
		//	// Creating a new scene variable to connect the mesh with
		//	auto scene6 = root_5.GetComponent<Scene3DCom>();
		//	scene6->SetVisiable(true);
		//	
		//	// Mesh's material data
		//	floor_mesh->ModifyAllMaterial([&](Material* material) {
		//		material->Kd = Vec3f(1.0f, 1.0f, 1.0f);
		//		//material->Ks = 1.f * Vec3f(0.0f, 0.0f, 0.0f);
		//		material->roughness = 0.1f;
		//		material->metallic = 1.0f;
		//		material->emissive = false;
		//		});
		//	scene6->SetSceneData(floor_mesh);
		//
	}
}