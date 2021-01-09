#include "engine-precompiled-header.h"
#include "EditorCameraControllerComSys.h"

#include "engine/core/thread/BaseJob.h"
#include "engine//core/thread/coroutine.h"
#include "engine/ecs/header/header.h"
#include "engine/math/Quaternion.h"
#include "engine/ui/ImGuiUtil.h"
#include "editor/events/EditorCustomEvent.h"

void AAAAgames::EditorCameraControllerComSys::Init()
{
	{
		auto queue = EventQueue<EditorEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<EditorCameraControllerComSys>(this, EditorEventType::EDITOR_CAM_TELEPORT_TO, &EditorCameraControllerComSys::_ON_CAM_TELEPORT_TO_ENTITY));
	}
}

void AAAAgames::EditorCameraControllerComSys::_ON_CAM_TELEPORT_TO_ENTITY(EventQueue<EditorEventType>::EventPtr e)
{
	if (auto event = std::static_pointer_cast<EditorCameraTeleportToEntityEvent>(e); event)
	{
		auto camera = m_parentWorld->GetTheOnlyEntityWithType((EntityType)EngineEntityType::EDITOR_CAMERA);
		auto trans = GetComponent<Transform3DCom>(camera);

		if (event->m_entity.GetWorld() != m_parentWorld)
		{
			return;
		}
		auto trans_target = GetComponent<Transform3DCom>(event->m_entity);

		// Teleport cam to target
		trans->SetGlobalPos(trans_target->GetGlobalPos());
		// If the current camera is FIRST_PERSON, then offset backward with zoom,
		// If the current camera is LOOK_AT, then do nothing because LOOK_AT camera perfers to be centered at the object
		if (auto perspective = GetComponent<PerspectiveCameraCom>(camera);
			perspective->GetCamera()->type == PerspectiveCameraType::FIRST_PERSON)
		{
			float zoom_dist = 5.0f;
			if (auto body_target = GetComponent<Body3DCom>(event->m_entity); body_target.Valid())
			{
				if (auto bv = body_target->GetBV(); bv)
				{
					zoom_dist = bv->GetRadius() * 1.2f;
				}
			}
			trans->AddLocalPos(Vec3f(0, -zoom_dist, 0));
		}
	}
	else
	{
		ENGINE_EXCEPT(L"Event casting failed!");
	}
}

void AAAAgames::EditorCameraControllerComSys::Update(double ts)
{
	// Switching between editing mode and game mode
	if (auto input = InputManager::GetInstance(); input->IsKeyPressed(KEY_LEFT_CONTROL) && input->IsKeyTriggered(KEY_P))
	{
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
		{
			m_editingWorld = m_parentWorld;
			auto queue = EventQueue<EditorEventType>::GetInstance();
			auto e = MemoryManager::Make_shared<EditorSwitchToGameModeEvent>(m_editingWorld);
			queue->Publish(e, 0); //!< Delayed event with 0 delay would result in a exection the in the frame
		}
		break;
		case Engine::ENGINE_MODE::INGAME:
		{
			auto queue = EventQueue<EditorEventType>::GetInstance();
			auto e = MemoryManager::Make_shared<EditorSwitchToEditingModeEvent>(m_editingWorld);
			queue->Publish(e, 0); //!< Delayed event with 0 delay would result in a exection the in the frame
		}
		break;
		}
	}

	float dt = static_cast<float>(ts);
	EntityType e_type;
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING: case Engine::ENGINE_MODE::INGAME_EDITING:
		e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
		break;
	default:
		return;
	}

	auto camera = m_parentWorld->GetTheOnlyEntityWithType(e_type);
	auto input = InputManager::GetInstance();
	auto trans = GetComponent<Transform3DCom>(camera);
	auto cam = GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

	if (input->IsKeyTriggered(KEY_F1))
	{
		Renderer3D::ToggleWireFrameMode();
	}
	if (input->IsKeyTriggered(KEY_F2))
	{
		Renderer3D::ToggleDrawBoundingVolumeMode();
	}
	if (input->IsKeyTriggered(KEY_F3))
	{
		/*
			Test: Unity like ForEach function
		*/
		ForEach(
			[](EntityDecorator e)
		{ DEBUG_PRINT("I Should not be printed!"); }
		);
		BackEach(
			[](EntityDecorator e)
		{ DEBUG_PRINT("I Should not be printed!"); }
		).wait();
		ParEach(
			[](EntityDecorator e)
		{ DEBUG_PRINT("I Should not be printed!"); }
		).wait();
		m_parentWorld->ForEach<Scene3DCom, Transform3DCom, ActiveCom>(
			[](EntityDecorator e, Scene3DCom& Scene3DCom_ref, Transform3DCom& Transform3DCom_ref, ActiveCom& ActiveCom_ref)
		{ DEBUG_PRINT("Hello!");
		}
		);
		auto handle1 = m_parentWorld->BackEach<Scene3DCom, Transform3DCom, ActiveCom>(
			[](EntityDecorator e, Scene3DCom& Scene3DCom_ref, Transform3DCom& Transform3DCom_ref, ActiveCom& ActiveCom_ref)
		{ DEBUG_PRINT("Hello1!");
		}
		);
		auto handle2 = m_parentWorld->BackEach<Scene3DCom, Transform3DCom, ActiveCom>(
			[](EntityDecorator e, Scene3DCom& Scene3DCom_ref, Transform3DCom& Transform3DCom_ref, ActiveCom& ActiveCom_ref)
		{ DEBUG_PRINT("Hello2!");
		}
		);
		auto handle3 = m_parentWorld->ParEach<Scene3DCom, Transform3DCom, ActiveCom>(
			[](EntityDecorator e, Scene3DCom& Scene3DCom_ref, Transform3DCom& Transform3DCom_ref, ActiveCom& ActiveCom_ref)
		{ DEBUG_PRINT("Hello3!");
		}, 20);
		auto handle4 = m_parentWorld->ParEach<Scene3DCom, Transform3DCom, ActiveCom>(
			[](EntityDecorator e, Scene3DCom& Scene3DCom_ref, Transform3DCom& Transform3DCom_ref, ActiveCom& ActiveCom_ref)
		{ DEBUG_PRINT("Hello4!");
		}, 10);
		{
			handle1.wait();
			handle2.wait();
			handle3.wait();
			handle4.wait();
		}

		/*
			Test : Unity like job system
		*/
		{
			struct SimpleJob : BaseJob
			{
				int a = { 1 };
				int b = { 2 };
				int* ret = { nullptr };
				virtual void Execute() override
				{
					(*ret) += (a + b);
				}
			};
			int ret = 3;
			SimpleJob j;
			j.ret = &ret;
			SimpleJob j2;
			j2.a = 3;
			j2.b = 4;
			j2.ret = &ret;
			DEBUG_PRINT(Str(*j.ret));
			DEBUG_PRINT(Str(*j2.ret));
			auto jHandle = j.Schedule();
			auto jHandle2 = j2.Schedule({ &jHandle });
			// jHandle.wait();
			jHandle2.wait();
			DEBUG_PRINT(Str(*j.ret));
			DEBUG_PRINT(Str(*j2.ret));
		}

		/*
			Test Service Locator Pattern
		*/
		{
			struct Base
			{
				Base() = default;
				virtual ~Base() = default;
				int x;
			};
			struct Derived : public Base
			{
				Derived() = default;
				explicit Derived(int j)
				{
					x = j;
				};
			};
			ServiceLocator::Register<Derived>("job", 2);
			auto base = ServiceLocator::GetSingleton<Base>("job");
			ASSERT(base->x == 2, "Service Locator test failed!");
			auto new_base = ServiceLocator::GetNewInstance<Base>("job");
			ASSERT(new_base->x == 2, "Service Locator test failed!");
		}

		/*
			Scheduler
		*/
		A4GAMES_NOGET(ThreadPool::GetInstance()->enqueue_task(
			[]()
		{
			using namespace std;
			using namespace chrono;
			auto start = high_resolution_clock::now();
			auto duration = [start]() {
				auto now = high_resolution_clock::now();
				auto msecs = duration_cast<milliseconds>(now - start).count();
				stringstream ss;
				ss << msecs / 1000.0;
				cout << "elapsed " << ss.str() << "s\t: ";
			};

			cout << "start" << endl;
			Scheduler t(1ms);
			auto e1 = t.set_timeout(3s, [&]() { duration(); cout << "timeout 3s" << endl; });
			auto e2 = t.set_interval(1s, [&]() { duration(); cout << "interval 1s" << endl; });
			auto e3 = t.set_timeout(4s, [&]() { duration(); cout << "timeout 4s" << endl; });
			auto e4 = t.set_interval(2s, [&]() { duration(); cout << "interval 2s" << endl; });
			auto e5 = t.set_timeout(5s, [&]() { duration(); cout << "timeout that never happens" << endl; });
			e5->signal(); // cancel this timeout
			this_thread::sleep_for(5s);
			e4->signal(); // cancel this interval
			cout << "cancel interval 2" << endl;
			this_thread::sleep_for(5s);
			cout << "end" << endl;
		}
		));

		/*
			Test Blaze
		*/
		{
			ENG_TIME("Blaze Test");
			using Vec3r = blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded>;
			Vec3r i(1);
			using Vec3rList = std::vector<Vec3r, blaze::AlignedAllocator<Vec3r>>;
			Vec3rList is;
			is.emplace_back(i);
			blaze::min(i, i);
			for (int i(0); i < 100; ++i)
			{
				is.emplace_back(i);
				i += i;
			}
			BlazeCustom::Quat<float, blaze::columnMajor>::_UNIT_TEST_();
			BlazeCustom::Quat<double, blaze::columnMajor>::_UNIT_TEST_();
			BlazeCustom::Quat<float, blaze::rowMajor>::_UNIT_TEST_();
			BlazeCustom::Quat<double, blaze::rowMajor>::_UNIT_TEST_();
		}
		/*
			Test Bullet
		*/
		//		{
		//			ENG_TIME("Bullet3 Test");
		//#define PRINTF_FLOAT "%7.3f"
		//
		//			constexpr float gravity = -10.0f;
		//			constexpr float initialY = 10.0f;
		//			constexpr float timeStep = 1.0f / 60.0f;
		//			// TODO some combinations of coefficients smaller than 1.0
		//			// make the ball go up higher / not lose height. Why?
		//			constexpr float groundRestitution = 0.9f;
		//			constexpr float sphereRestitution = 0.9f;
		//			constexpr int maxNPoints = 1;
		//			static std::vector<btVector3> collisions;
		//			class dummy
		//			{
		//			public:
		//				static void myTickCallback(btDynamicsWorld* dynamicsWorld, btScalar timeStep)
		//				{
		//					collisions.clear();
		//					int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
		//					for (int i = 0; i < numManifolds; i++) {
		//						btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		//						// TODO those are unused. What can be done with them?
		//						// I think they are the same objects as those in the main loop
		//						// dynamicsWorld->getCollisionObjectArray() and we could compare
		//						// the pointers to see which object collided with which.
		//						{
		//							const btCollisionObject* objA = contactManifold->getBody0();
		//							const btCollisionObject* objB = contactManifold->getBody1();
		//						}
		//						int numContacts = contactManifold->getNumContacts();
		//						for (int j = 0; j < numContacts; j++) {
		//							btManifoldPoint& pt = contactManifold->getContactPoint(j);
		//							const btVector3& ptA = pt.getPositionWorldOnA();
		//							const btVector3& ptB = pt.getPositionWorldOnB();
		//							const btVector3& normalOnB = pt.m_normalWorldOnB;
		//							collisions.push_back(ptA);
		//							collisions.push_back(ptB);
		//							collisions.push_back(normalOnB);
		//						}
		//					}
		//				}
		//			};
		//
		//			int i, j;
		//
		//			btDefaultCollisionConfiguration* collisionConfiguration
		//				= new btDefaultCollisionConfiguration();
		//			btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
		//			btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
		//			btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
		//			btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
		//				dispatcher, overlappingPairCache, solver, collisionConfiguration);
		//			dynamicsWorld->setGravity(btVector3(0, gravity, 0));
		//			dynamicsWorld->setInternalTickCallback(dummy::myTickCallback);
		//			btAlignedObjectArray<btCollisionShape*> collisionShapes;
		//
		//			// Ground.
		//			{
		//				btTransform groundTransform;
		//				groundTransform.setIdentity();
		//				groundTransform.setOrigin(btVector3(0, 0, 0));
		//				btCollisionShape* groundShape;
		//#if 1
		//				// x / z plane at y = -1.
		//				groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), -1);
		//#else
		//				// A cube of width 10 at y = -6.
		//				// Does not fall because we won't call:
		//				// colShape->calculateLocalInertia
		//				// TODO: remove this from this example into a collision shape example.
		//				groundTransform.setOrigin(btVector3(0, -6, 0));
		//				groundShape = new btBoxShape(
		//					btVector3(btScalar(5.0), btScalar(5.0), btScalar(5.0)));
		//
		//#endif
		//				collisionShapes.push_back(groundShape);
		//				btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		//				btRigidBody::btRigidBodyConstructionInfo rbInfo(0, myMotionState, groundShape, btVector3(0, 0, 0));
		//				btRigidBody* body = new btRigidBody(rbInfo);
		//				body->setRestitution(groundRestitution);
		//				dynamicsWorld->addRigidBody(body);
		//			}
		//
		//			// Sphere.
		//			{
		//				btCollisionShape* colShape = new btSphereShape(btScalar(1.0));
		//				collisionShapes.push_back(colShape);
		//				btTransform startTransform;
		//				startTransform.setIdentity();
		//				startTransform.setOrigin(btVector3(0, initialY, 0));
		//				btVector3 localInertia(0, 0, 0);
		//				btScalar mass(1.0f);
		//				colShape->calculateLocalInertia(mass, localInertia);
		//				btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		//				btRigidBody* body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(
		//					mass, myMotionState, colShape, localInertia));
		//				body->setRestitution(sphereRestitution);
		//				dynamicsWorld->addRigidBody(body);
		//			}
		//
		//			// Main loop.
		//
		//			std::printf("step body x y z collision a b normal\n");
		//			for (i = 0; i < maxNPoints; ++i) {
		//				{
		//					ENG_TIME("Bullet3 Test 1 Loop");
		//					dynamicsWorld->stepSimulation(timeStep);
		//				}
		//				for (j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; --j) {
		//					btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
		//					btRigidBody* body = btRigidBody::upcast(obj);
		//					btTransform trans;
		//					if (body && body->getMotionState()) {
		//						body->getMotionState()->getWorldTransform(trans);
		//					}
		//					else {
		//						trans = obj->getWorldTransform();
		//					}
		//					btVector3 origin = trans.getOrigin();
		//					std::printf("%d %d " PRINTF_FLOAT " " PRINTF_FLOAT " " PRINTF_FLOAT " ",
		//						i,
		//						j,
		//						float(origin.getX()),
		//						float(origin.getY()),
		//						float(origin.getZ()));
		//					if (collisions.empty()) {
		//						std::printf("0 ");
		//					}
		//					else {
		//						std::printf("1 ");
		//						// Yes, this is getting reprinted for all bodies when collisions happen.
		//						// It's just a quick and dirty way to visualize it, should be outside
		//						// of this loop normally.
		//						for (auto& v : collisions) {
		//							std::printf(
		//								PRINTF_FLOAT " " PRINTF_FLOAT " " PRINTF_FLOAT " ",
		//								v.getX(), v.getY(), v.getZ());
		//						}
		//					}
		//					puts("");
		//				}
		//			}
		//
		//			// Cleanup.
		//			for (i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
		//				btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		//				btRigidBody* body = btRigidBody::upcast(obj);
		//				if (body && body->getMotionState()) {
		//					delete body->getMotionState();
		//				}
		//				dynamicsWorld->removeCollisionObject(obj);
		//				delete obj;
		//			}
		//			for (i = 0; i < collisionShapes.size(); ++i) {
		//				delete collisionShapes[i];
		//			}
		//			delete dynamicsWorld;
		//			delete solver;
		//			delete overlappingPairCache;
		//			delete dispatcher;
		//			delete collisionConfiguration;
		//			collisionShapes.clear();
		//		}

				/*
					Test qu3e
				*/
		{
			ENG_TIME("qu3e Test");
			class Raycast : public q3QueryCallback
			{
			public:
				q3RaycastData data;
				r32 tfinal;
				q3Vec3 nfinal;
				q3Body* impactBody;

				bool ReportShape(q3Box* shape)
				{
					if (data.toi < tfinal)
					{
						tfinal = data.toi;
						nfinal = data.normal;
						impactBody = shape->body;
					}

					data.toi = tfinal;
					return true;
				}

				void Init(const q3Vec3& spot, const q3Vec3& dir)
				{
					data.start = spot;
					data.dir = q3Normalize(dir);
					data.t = r32(10000.0);
					tfinal = FLT_MAX;
					data.toi = data.t;
					impactBody = NULL;
				}
			};
			float dt = 1.0f / 60.0f;
			q3Scene scene(dt);
			Raycast rayCast;
			{
				// Create the floor
				q3BodyDef bodyDef;
				q3Body* body = scene.CreateBody(bodyDef);

				q3BoxDef boxDef;
				boxDef.SetRestitution(0);
				q3Transform tx;
				q3Identity(tx);
				boxDef.Set(tx, q3Vec3(50.0f, 1.0f, 50.0f));
				body->AddBox(boxDef);
			}

			{
				q3BodyDef bodyDef;
				bodyDef.position.Set(0.0f, 3.0f, 0.0f);
				bodyDef.axis.Set(q3RandomFloat(-1.0f, 1.0f), q3RandomFloat(-1.0f, 1.0f), q3RandomFloat(-1.0f, 1.0f));
				bodyDef.angle = q3PI * q3RandomFloat(-1.0f, 1.0f);
				bodyDef.bodyType = eDynamicBody;
				bodyDef.angularVelocity.Set(q3RandomFloat(1.0f, 3.0f), q3RandomFloat(1.0f, 3.0f), q3RandomFloat(1.0f, 3.0f));
				bodyDef.angularVelocity *= q3Sign(q3RandomFloat(-1.0f, 1.0f));
				bodyDef.linearVelocity.Set(q3RandomFloat(1.0f, 3.0f), q3RandomFloat(1.0f, 3.0f), q3RandomFloat(1.0f, 3.0f));
				bodyDef.linearVelocity *= q3Sign(q3RandomFloat(-1.0f, 1.0f));
				q3Body* body = scene.CreateBody(bodyDef);

				q3Transform tx;
				q3Identity(tx);
				q3BoxDef boxDef;
				boxDef.Set(tx, q3Vec3(1.0f, 1.0f, 1.0f));
				body->AddBox(boxDef);
			}

			{
				ENG_TIME("qu3e Test 1 Loop");
				scene.Step();
			}

			rayCast.Init(q3Vec3(3.0f, 5.0f, 3.0f), q3Vec3(-1.0f, -1.0f, -1.0f));
			scene.RayCast(&rayCast, rayCast.data);

			if (rayCast.impactBody)
			{
				rayCast.impactBody->SetToAwake();
				rayCast.impactBody->ApplyForceAtWorldPoint(rayCast.data.dir * 20.0f, rayCast.data.GetImpactPoint());
			}

			scene.RemoveAllBodies();
		}

		/*
			Test Sol Lua5.4.0 Binding
		*/
		{
			sol::state lua;
			lua.open_libraries(sol::lib::base);
			int x = 0;
			lua.set_function("beep", [&x] { ++x; DEBUG_PRINT("LUA is up!"); });
			lua.script("beep()");
			ASSERT(x == 1, "Lua test failed!");
			lua.script("print(\"LUA2 is up!\")");
		}

		/*
			Test emilib coroutine
		*/
		A4GAMES_NOGET(ThreadPool::GetInstance()->enqueue_task(
			[]()
		{
			constexpr double dt = 1.0 / 60;
			Timer t(dt);

			emilib::CoroutineSet coroutine_set;

			// Common state:
			bool time_for_captain_to_show = false;
			bool captain_has_been_painted = false;
			bool bomb_has_ben_set_up = false;
			constexpr double kSecondsBetweenLines = 1.0;

			// A coroutine for writing a script in a timely fashion:
			auto text_cr = coroutine_set.start("intro_text", [&](emilib::InnerControl& ic) {
				DEBUG_PRINT("In A.D. 2101");
				ic.wait_sec(kSecondsBetweenLines);
				DEBUG_PRINT("War was beginning.");
				ic.wait_sec(kSecondsBetweenLines);
				time_for_captain_to_show = true;
				ic.wait_for([&]() { return captain_has_been_painted; });
				DEBUG_PRINT("Captain: What happen?");
				ic.wait_sec(kSecondsBetweenLines);
				DEBUG_PRINT("Mechanic: Somebody set up us the bomb.");
				bomb_has_ben_set_up = true;
				ic.wait_sec(kSecondsBetweenLines);
				DEBUG_PRINT("Operator: We get signal.");
				ic.wait_sec(kSecondsBetweenLines);
				DEBUG_PRINT("Captain: What !");
			});

			// Start up second (unnecessary) coroutine for demonstrative purposes:
			coroutine_set.start("intro_graphics", [&](emilib::InnerControl& ic) {
				ic.wait_for([&]() { return time_for_captain_to_show; });
				DEBUG_PRINT("[INSERT CAPTAIN DRAWING HERE]");
				captain_has_been_painted = true;
			});

			while (!coroutine_set.empty()) {
				while (!t.Check(true)) {}
				coroutine_set.poll(dt); // Run all coroutines

				if (bomb_has_ben_set_up && text_cr) {
					DEBUG_PRINT("(Aborting early to demonstrate how)");
					text_cr->stop();
					coroutine_set.clear();
				}
			}
		}
		));
	}

	/******************************************************************************
	**	UE4 like editor camera movement for both keyboard & mouse contrl
	**  and gamepad control
	******************************************************************************/
	Vec2f gamepad_left_aixs;
	Vec2f gamepad_right_aixs;
	Vec2f mouse_delta_pixel;
	Vec2f mouse_max_pixel;
	Vec2f mouse_delta_normlized;
	Vec3f rv_pitch;
	Vec3f rv_yaw;
	Vec3f local_v;
	static Vec3f friction_local_v;
	Vec3f global_v;
	static Vec3f friction_global_v;
	constexpr float v_speed = 7.f;
	static float v_max = 30.f;
	constexpr float rotation_speed = 180.f;
	static float speed_up_multi = 1.0f;
	bool bUINotHoldMouse = !ImGuiUtil::IsMouseCaptured();
	bool bUINotHoldKeyBoard = !ImGuiUtil::IsKeyBoardCaptured();
	// Apply friction
	{
		friction_local_v *= powf(0.001f, dt);
		friction_global_v *= powf(0.001f, dt);
	}
	if ((input->IsKeyTriggered(KEY_LEFT_ALT) && bUINotHoldKeyBoard) || input->IsGamepadButtonTriggered(GAMEPAD_BUTTON_RIGHT_THUMB))
	{
		static Quaternion rot = trans->GetGlobalRot();
		static Vec3f pos = trans->GetGlobalPos();
		static Quaternion rot2 = trans->GetGlobalRot();
		static Vec3f pos2 = trans->GetGlobalPos();
		switch (cam->type)
		{
		case AAAAgames::PerspectiveCameraType::LOOK_AT:
			cam->type = PerspectiveCameraType::FIRST_PERSON;
			PRINT("Switch to FIRST_PERSON camera");
			break;
		case AAAAgames::PerspectiveCameraType::FIRST_PERSON:
			cam->type = PerspectiveCameraType::LOOK_AT;
			PRINT("Switch to LOOK AT camera");
			break;
		}
	}
	// Keyboard & Mouse inputs
	{
		if (input->IsKeyTriggered(KEY_LEFT_SHIFT) && bUINotHoldKeyBoard)
		{
			speed_up_multi = (speed_up_multi == 1.0f) ? 2.0f : (speed_up_multi == 2.0f) ? 1.0f : 1.0f;
			v_max = (v_max == 30.0f) ? 40.0f : (v_max == 40.0f) ? 30.0f : 30.0f;
		}
		switch (cam->type)
		{
		case AAAAgames::PerspectiveCameraType::LOOK_AT:
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && bUINotHoldMouse)
			{
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = A4GAMES_Lerp(1.0f, 2.0f * rotation_speed, fabs(mouse_delta_pixel.x - pixel_threshold) / pixel_max_threshold);
				float y_multi = A4GAMES_Lerp(1.0f, 2.0f * rotation_speed, fabs(mouse_delta_pixel.y - pixel_threshold) / pixel_max_threshold);

				if (mouse_delta_pixel.x > pixel_threshold)
				{
					rv_yaw += x_multi * DEG2RAD * Geommath::WorldUp;
				}
				if (mouse_delta_pixel.x < -pixel_threshold)
				{
					rv_yaw += -x_multi * DEG2RAD * Geommath::WorldUp;
				}
				if (mouse_delta_pixel.y > pixel_threshold)
				{
					rv_pitch += y_multi * DEG2RAD * Geommath::WorldRight;
				}
				if (mouse_delta_pixel.y < -pixel_threshold)
				{
					rv_pitch += -y_multi * DEG2RAD * Geommath::WorldRight;
				}
			}
			{
				auto& offsets = input->GetMouseScrollOffsets();
				float y_offset = offsets.y;
				cam->SetZoom(cam->GetZoom() - y_offset * speed_up_multi);
			}
			break;
		case AAAAgames::PerspectiveCameraType::FIRST_PERSON:
			// UE4 like movement with MMB pressed :
			// panning on local right / left and global up / down
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && bUINotHoldMouse)
			{
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = A4GAMES_Lerp(v_speed, v_max, fabs(mouse_delta_pixel.x - pixel_threshold) / pixel_max_threshold);
				float y_multi = A4GAMES_Lerp(v_speed, v_max, fabs(mouse_delta_pixel.y - pixel_threshold) / pixel_max_threshold);

				if (mouse_delta_pixel.x > pixel_threshold)
				{
					local_v += x_multi * Geommath::WorldRight;
				}
				if (mouse_delta_pixel.x < -pixel_threshold)
				{
					local_v += -x_multi * Geommath::WorldRight;
				}
				if (mouse_delta_pixel.y > pixel_threshold)
				{
					global_v += -y_multi * Geommath::WorldUp;
				}
				if (mouse_delta_pixel.y < -pixel_threshold)
				{
					global_v += y_multi * Geommath::WorldUp;
				}
				break;
			}
			// UE4 like movement with RMB pressed :
			// panning on local front/back and local right/left
			// Rotate with global yaw and local pitch
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && bUINotHoldMouse)
			{
				if (input->IsKeyPressed(KEY_W))
				{
					friction_local_v += v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_S))
				{
					friction_local_v += -v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_D))
				{
					friction_local_v += v_speed * Geommath::WorldRight;
				}
				if (input->IsKeyPressed(KEY_A))
				{
					friction_local_v += -v_speed * Geommath::WorldRight;
				}
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = A4GAMES_Lerp(0.0f, 2.0f * rotation_speed, (fabs(mouse_delta_pixel.x) - pixel_threshold) / pixel_max_threshold);
				float y_multi = A4GAMES_Lerp(0.0f, 2.0f * rotation_speed, (fabs(mouse_delta_pixel.y) - pixel_threshold) / pixel_max_threshold);

				if (mouse_delta_pixel.x > pixel_threshold)
				{
					rv_yaw += -x_multi * DEG2RAD * Geommath::WorldUp;
				}
				if (mouse_delta_pixel.x < -pixel_threshold)
				{
					rv_yaw += x_multi * DEG2RAD * Geommath::WorldUp;
				}
				if (mouse_delta_pixel.y > pixel_threshold)
				{
					rv_pitch += -y_multi * DEG2RAD * Geommath::WorldRight;
				}
				if (mouse_delta_pixel.y < -pixel_threshold)
				{
					rv_pitch += y_multi * DEG2RAD * Geommath::WorldRight;
				}
				break;
			}
			// If neither RMB or MMB is pressed, do UE4 like mouse scroll:
			// panning on local front/back
			if (bUINotHoldMouse)
			{
				auto& offsets = input->GetMouseScrollOffsets();
				float y_offset = offsets.y;
				local_v += (y_offset * speed_up_multi) * v_max * Geommath::WorldFront;
				break;
			}
		}
	}
	// Gamepad inputs
	{
		if (input->IsGamepadActive())
		{
			switch (cam->type)
			{
			case AAAAgames::PerspectiveCameraType::LOOK_AT:
			{
				{
					gamepad_right_aixs = input->GetGamepadRightStickXY();

					float x_multi = gamepad_right_aixs.x * rotation_speed;
					float y_multi = gamepad_right_aixs.y * rotation_speed;

					rv_yaw += x_multi * DEG2RAD * Geommath::WorldUp;
					rv_pitch += y_multi * DEG2RAD * Geommath::WorldRight;
				}
				// UE4 zoom in/out by left stick y-axis
				{
					gamepad_left_aixs = input->GetGamepadLeftStickXY();
					float y_multi = gamepad_left_aixs.y;
					cam->SetZoom(cam->GetZoom() + y_multi * speed_up_multi);
				}
			}
			break;
			case AAAAgames::PerspectiveCameraType::FIRST_PERSON:
			{
				// Move by left axis, rotate by right axis
				{
					gamepad_left_aixs = input->GetGamepadLeftStickXY();

					float x_multi = gamepad_left_aixs.x;
					float y_multi = gamepad_left_aixs.y;

					friction_local_v += x_multi * v_speed * Geommath::WorldRight;
					friction_local_v += -y_multi * v_speed * Geommath::WorldFront;
				}
				{
					gamepad_right_aixs = input->GetGamepadRightStickXY();

					float x_multi = gamepad_right_aixs.x * rotation_speed;
					float y_multi = gamepad_right_aixs.y * rotation_speed;

					rv_yaw += -x_multi * DEG2RAD * Geommath::WorldUp;
					rv_pitch += -y_multi * DEG2RAD * Geommath::WorldRight;
				}
				// Move up and down in global frame by pressing right and left trigger
				{
					float left_trigger = input->GetGamepadLeftTrigger();
					float right_trigger = input->GetGamepadRightTrigger();

					float x_multi = A4GAMES_Lerp(v_speed, v_max, left_trigger);
					float y_multi = A4GAMES_Lerp(v_speed, v_max, right_trigger);

					global_v += -x_multi * Geommath::WorldUp;
					global_v += y_multi * Geommath::WorldUp;
				}
				// UE4 zoom in/out by right/left bumper
				if (input->IsGamepadButtonPressed(GAMEPAD_BUTTON_LEFT_BUMPER))
				{
					local_v += -speed_up_multi * v_max * Geommath::WorldFront;
				}
				if (input->IsGamepadButtonPressed(GAMEPAD_BUTTON_RIGHT_BUMPER))
				{
					local_v += speed_up_multi * v_max * Geommath::WorldFront;
				}
				// UE4 stops the camera movement on triggering the left thumb button
				if (input->IsGamepadButtonTriggered(GAMEPAD_BUTTON_LEFT_THUMB))
				{
					local_v *= 0.0;
					global_v *= 0.0;
					friction_local_v *= 0.01;
					friction_global_v *= 0.01;
				}
			}
			break;
			}
		}
	}

	// Clamping
	{
		friction_local_v = glm::clamp(friction_local_v, Vec3f(-v_max), Vec3f(v_max));
		local_v = glm::clamp(local_v, Vec3f(-v_max), Vec3f(v_max));
		friction_global_v = glm::clamp(friction_global_v, Vec3f(-v_max), Vec3f(v_max));
		global_v = glm::clamp(global_v, Vec3f(-v_max), Vec3f(v_max));
	}
	switch (cam->type)
	{
	case AAAAgames::PerspectiveCameraType::LOOK_AT:
		trans->SetLocalVel(Vec3f(0));
		trans->SetGlobalVel(Vec3f(0));
		trans->SetGlobalRotVel(-rv_yaw);
		trans->SetLocalRotVel(-rv_pitch);
		break;
	case AAAAgames::PerspectiveCameraType::FIRST_PERSON:
		trans->SetLocalVel(friction_local_v + local_v);
		trans->SetGlobalVel(friction_global_v + global_v);
		trans->SetGlobalRotVel(rv_yaw);
		trans->SetLocalRotVel(rv_pitch);
		break;
	}
}

std::shared_ptr<BaseComponentSystem> AAAAgames::EditorCameraControllerComSys::Copy() const
{
	LOCK_GUARD_NC();
	auto ret = MemoryManager::Make_shared<EditorCameraControllerComSys>();
	ret->m_bufferedRegisteredEntities = m_bufferedRegisteredEntities; // Must copy
	ret->m_systemSignature = m_systemSignature; // Must copy
	ret->m_editingWorld = m_editingWorld; // Custom variables
	return ret;
}