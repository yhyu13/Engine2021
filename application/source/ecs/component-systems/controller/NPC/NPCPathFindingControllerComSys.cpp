#include "application-precompiled-header.h"
#include "NPCPathFindingControllerComSys.h"

#include "engine/Engine.h"
#include "engine/ecs/header/header.h"
#include "ecs/EntityType.h"
#include "events/CustomEvents.h"
#include "engine/core/utility/Random.h"
#include "engine/ai/PathFinding/PostProcessing.h"

void AAAAgames::NPCPathFindingControllerComSys::Init()
{
	{
		auto queue = EventQueue<CS560EventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<NPCPathFindingControllerComSys>(this, CS560EventType::GEN_RND_PATH, &NPCPathFindingControllerComSys::_ON_GEN_RND_PATH));
		ManageEventSubHandle(queue->Subscribe<NPCPathFindingControllerComSys>(this, CS560EventType::SET_PATHER_CONTROL, &NPCPathFindingControllerComSys::_ON_SET_PATHER_CONTROL));
		ManageEventSubHandle(queue->Subscribe<NPCPathFindingControllerComSys>(this, CS560EventType::SET_PATHER_MISC, &NPCPathFindingControllerComSys::_ON_SET_PATHER_MISC));
		ManageEventSubHandle(queue->Subscribe<NPCPathFindingControllerComSys>(this, CS560EventType::SET_PATHER_SMOOTH_SETTING, &NPCPathFindingControllerComSys::_ON_SET_PATHER_SMOOTH_SETTING));
	}
}

void AAAAgames::NPCPathFindingControllerComSys::_ON_GEN_RND_PATH(EventQueue<CS560EventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<CS560GenRndPathEvent>(e);
	if (m_parentWorld == event->m_entity.GetWorld())
	{
		// Remove entity if it was added
		{
			RemoveEntity(event->m_entity);
			{
				// Remove previous debug draw
				auto& data = m_PatherLUT[event->m_entity];
				auto queue = EventQueue<EngineEventType>::GetInstance();
				auto e1 = MemoryManager::Make_shared<EngineGCRecursiveEvent>(data.debug_path_root_node);
				queue->Publish(e1);
				data.debug_path_root_node.Reset();
				auto e2 = MemoryManager::Make_shared<EngineGCRecursiveEvent>(data.debug_smooth_path_root_node);
				queue->Publish(e2);
				data.debug_smooth_path_root_node.Reset();
			}
		}
		{
			AddEntity(event->m_entity);

			A4GAMES_Vector<int> indices;
			A4GAMES_Range(indices, 0, NumPoints - 1, 1);
			std::shuffle(indices.begin(), indices.end(), Random::s_RandomEngine);

			int num_points = RAND_I(4, NumPoints);
			A4GAMES_Vector<Vec3f> chosen_path;
			while (num_points > 0)
			{
				chosen_path.emplace_back(Points[indices[--num_points]]);
			}
			auto smooth_setting = pathfinding::SmoothingSetting{ .mid_points_coarse_grain_stride = 0.2f,
																.end_points_fine_grain_range = 0.1f,
																.end_points_fine_grain_stride = 0.05f };
			auto smoothed_path = pathfinding::CatmullRomSmoothing(chosen_path, smooth_setting);
			PathMoverData mover{ .original_pathLUT = chosen_path, .smooth_pathLUT = smoothed_path , .smooth_path_arc_lengthLUT = pathfinding::GetArcLengthLUT(smoothed_path),.path_agent = event->m_entity };
			m_PatherLUT[event->m_entity] = (mover);
			auto& data = m_PatherLUT[event->m_entity];

			// Move the agent to the starting position
			if (auto trans = data.path_agent.GetComponent<Transform3DCom>(); trans.Valid())
			{
				auto& p0 = data.smooth_pathLUT[0];
				auto& p1 = data.smooth_pathLUT[1];
				trans->SetGlobalPos(p0);
				// TODO Remove the rotation matrix multiplier (it was just here for GAM550 path motion demo)
				trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, p1 - p0) * Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI) * Geommath::RotationMat(Geommath::ROT_AXIS::X, PI * 0.5f)));
			}

			// Debug drawing path
			{
				DebugDrawOriginalPath(data);
				DebugDrawSmoothPath(data);
			}
		}
	}
}

void AAAAgames::NPCPathFindingControllerComSys::_ON_SET_PATHER_CONTROL(EventQueue<CS560EventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<CS560PatherControlEvent>(e);
	if (m_parentWorld == event->m_entity.GetWorld())
	{
		if (auto it = m_PatherLUT.find(event->m_entity); it != m_PatherLUT.end())
		{
			auto& data = it->second;
			data.pause = event->m_pause;
			if (event->m_restart)
			{
				data.current_distance = 0.f;
				data.current_speed = 0.f;
				data.current_time = 0.f;

				// Move the agent to the starting position
				if (auto trans = data.path_agent.GetComponent<Transform3DCom>(); trans.Valid())
				{
					auto& p0 = data.smooth_pathLUT[0];
					auto& p1 = data.smooth_pathLUT[1];
					trans->SetGlobalPos(p0);
					// TODO Remove the rotation matrix multiplier (it was just here for GAM550 path motion demo)
					trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, p1 - p0) * Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI) * Geommath::RotationMat(Geommath::ROT_AXIS::X, PI * 0.5f)));
				}
			}
		}
	}
}

void AAAAgames::NPCPathFindingControllerComSys::_ON_SET_PATHER_MISC(EventQueue<CS560EventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<CS560PatherMiscEvent>(e);
	if (m_parentWorld == event->m_entity.GetWorld())
	{
		if (auto it = m_PatherLUT.find(event->m_entity); it != m_PatherLUT.end())
		{
			auto& data = it->second;
			data.target_speed = event->m_speed;
			data.speed_ramp_up_time = event->m_speed_ramp_up_time;
			data.speed_ramp_down_time = event->m_speed_ramp_down_time;

			auto total_distance = data.smooth_path_arc_lengthLUT.back();
			if (auto ramping_time = data.speed_ramp_up_time + data.speed_ramp_down_time;
				0.5f * data.target_speed * ramping_time < total_distance)
			{
				// Case where ease-in/out form a trapezoidal
				// Solving the v - t trapezoidal whose area equals distance
				data.total_time = 0.5f * (2.0f * total_distance / data.target_speed + ramping_time);
			}
			else
			{
				// Case where ease-in and ease-out is greater than the total distance, need to find a smaller target speed
				data.target_speed = 2.0f * total_distance / (ramping_time);
				data.total_time = ramping_time;
			}
		}
	}
}

void AAAAgames::NPCPathFindingControllerComSys::_ON_SET_PATHER_SMOOTH_SETTING(EventQueue<CS560EventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<CS560PatherSmoothSettingEvent>(e);
	if (m_parentWorld == event->m_entity.GetWorld())
	{
		if (auto it = m_PatherLUT.find(event->m_entity); it != m_PatherLUT.end())
		{
			auto& data = it->second;
			auto& chosen_path = data.original_pathLUT;
			auto smooth_setting = pathfinding::SmoothingSetting{ .mid_points_coarse_grain_stride = 0.2f,
																.end_points_fine_grain_range = 0.1f,
																.end_points_fine_grain_stride = 0.05f,
																.angle_rad_tolerance = event->m_angle,
																.delta_distance_tolerance = event->m_distance };
			auto smoothed_path = pathfinding::CatmullRomSmoothing(chosen_path, smooth_setting);
			data.smooth_pathLUT = smoothed_path;
			data.smooth_path_arc_lengthLUT = pathfinding::GetArcLengthLUT(smoothed_path);
			DebugDrawSmoothPath(data);
		}
	}
}

void AAAAgames::NPCPathFindingControllerComSys::DebugDrawOriginalPath(PathMoverData& data)
{
	// Remove previous debug
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(data.debug_path_root_node);
		queue->Publish(e);
		data.debug_path_root_node.Reset();
	}

	auto parent = data.path_agent;
	auto id = parent.GetComponent<IDNameCom>();
	auto entity_name = id->GetName();
	auto world = m_parentWorld;

	auto root_debug_node = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::STATIC_OBJ, true, true);
	data.debug_path_root_node = root_debug_node;

	auto& path = data.original_pathLUT;
	for (auto i(0u); i <= path.size() - 2; ++i)
	{
		auto debug_node = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::STATIC_OBJ, true, false);
		auto parentChildCom = root_debug_node.GetComponent<ChildrenCom>();
		parentChildCom->AddEntity(debug_node);

		auto rm = ResourceManager<Scene3DNode>::GetInstance();
		auto meshName = "debugArrow";
		auto mesh = rm->TryGet(meshName)->Get()->Copy();
		auto scene = debug_node.GetComponent<Scene3DCom>();
		scene->SetVisiable(true);
		mesh->ModifyAllMaterial([&](Material* material)
		{
			material->Kd = Vec3f(0.85, 0.1, 0.1);
		});
		scene->SetSceneData(mesh);

		auto id = debug_node.GetComponent<IDNameCom>();
		id->SetName(entity_name + "_original_path_" + Str(i));

		auto trans = debug_node.GetComponent<Transform3DCom>();
		trans->m_apply_parent_rot = false;
		trans->m_apply_parent_trans = true;
		trans->m_apply_parent_scale = false;

		auto parentTrCom = root_debug_node.GetComponent<Transform3DCom>();
		trans->SetParentModelTr(parentTrCom->GetSuccessionModelTr(trans));

		auto& p0 = path[i];
		auto& p1 = path[i + 1];
		trans->SetGlobalPos(p0);
		trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, p1 - p0));
		trans->SetLocalScale(Vec3f(1, Geommath::Length(p1 - p0), 1));
	}
}

void AAAAgames::NPCPathFindingControllerComSys::DebugDrawSmoothPath(PathMoverData& data)
{
	// Remove previous debug
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(data.debug_smooth_path_root_node);
		queue->Publish(e);
		data.debug_smooth_path_root_node.Reset();
	}

	auto parent = data.path_agent;
	auto id = parent.GetComponent<IDNameCom>();
	auto entity_name = id->GetName();
	auto world = m_parentWorld;

	auto root_debug_node = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::STATIC_OBJ, true, true);
	data.debug_smooth_path_root_node = root_debug_node;

	auto& path = data.smooth_pathLUT;
	for (auto i(0u); i <= path.size() - 2; ++i)
	{
		auto debug_node = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::STATIC_OBJ, true, false);
		auto parentChildCom = root_debug_node.GetComponent<ChildrenCom>();
		parentChildCom->AddEntity(debug_node);

		auto rm = ResourceManager<Scene3DNode>::GetInstance();
		auto meshName = "debugArrow";
		auto mesh = rm->TryGet(meshName)->Get()->Copy();
		auto scene = debug_node.GetComponent<Scene3DCom>();
		scene->SetVisiable(true);
		mesh->ModifyAllMaterial([&](Material* material)
		{
			material->Kd = Vec3f(0.1, 0.1, 0.85);
		});
		scene->SetSceneData(mesh);

		auto id = debug_node.GetComponent<IDNameCom>();
		id->SetName(entity_name + "_smooth_path_" + Str(i));

		auto trans = debug_node.GetComponent<Transform3DCom>();
		trans->m_apply_parent_rot = false;
		trans->m_apply_parent_trans = true;
		trans->m_apply_parent_scale = false;

		auto parentTrCom = root_debug_node.GetComponent<Transform3DCom>();
		trans->SetParentModelTr(parentTrCom->GetSuccessionModelTr(trans));

		auto& p0 = path[i];
		auto& p1 = path[i + 1];
		trans->SetGlobalPos(p0);
		trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, p1 - p0));
		trans->SetLocalScale(Vec3f(1, Geommath::Length(p1 - p0), 1));
	}
}

void AAAAgames::NPCPathFindingControllerComSys::Update(double dt)
{
	EntityType e_type;
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		e_type = (EntityType)GameEntityType::PLAYER;
		break;
	default:
		return;
	}

	ForEach(
		[this, dt](EntityDecorator e)
	{
		if (auto it = m_PatherLUT.find(e); it != m_PatherLUT.end())
		{
			auto& data = it->second;

			// Handle paused case
			if (data.pause)
			{
				return;
			}

			// Calculate speed, distance based on speed ramp up/down
			data.current_time += dt;
			if (auto t = data.current_time; t <= data.speed_ramp_up_time)
			{
				data.current_speed = t / data.speed_ramp_up_time * data.target_speed;
			}
			else if (t <= (data.total_time) - data.speed_ramp_down_time)
			{
				data.current_speed = data.target_speed;
			}
			else if (t <= data.total_time)
			{
				data.current_speed = (data.total_time - t) / data.speed_ramp_down_time * data.target_speed;
			}
			else
			{
				data.current_speed = 0.f;
			}
			data.current_distance += dt * data.current_speed;

			// Clamping distance on the path
			if (data.current_distance < 0.f)
			{
				data.current_distance = 0.f;
			}
			else if (data.current_distance > data.smooth_path_arc_lengthLUT.back())
			{
				data.current_distance = data.smooth_path_arc_lengthLUT.back();
			}

			// Update animation
			if (data.current_speed <= 0.01f)
			{
				if (auto anima = data.path_agent.GetComponent<Animation3DCom>(); anima.Valid())
				{
					anima->SetCurrentAnimation(Animation3DCom::AnimationSetting{ .name = "Idle", .playBackSpeed = 1.f, .refresh = false, .looping = true, .pause = false });
				}
			}
			else
			{
				if (auto anima = data.path_agent.GetComponent<Animation3DCom>(); anima.Valid())
				{
					constexpr auto standard_speed = 2.5f;
					auto play_back_speed = data.current_speed / standard_speed;
					anima->SetCurrentAnimation(Animation3DCom::AnimationSetting{ .name = "C_Walk_IP", .playBackSpeed = play_back_speed, .refresh = false, .looping = true ,.pause = false });
				}
			}

			// Update transformation
			if (auto t1 = A4GAMES_lowerBoundFindIndex(data.smooth_path_arc_lengthLUT, data.current_distance);
				t1 != -1 && t1 != 0)
			{
				auto t0 = t1 - 1;
				auto factor = (data.current_distance - data.smooth_path_arc_lengthLUT[t0]) / (data.smooth_path_arc_lengthLUT[t1] - data.smooth_path_arc_lengthLUT[t0]);
				auto& p0 = data.smooth_pathLUT[t0];
				auto& p1 = data.smooth_pathLUT[t1];
				auto currentPosOnPath = Geommath::Lerp(p0, p1, factor);
				if (auto trans = e.GetComponent<Transform3DCom>(); trans.Valid())
				{
					trans->SetGlobalPos(currentPosOnPath);
					// TODO Remove the rotation matrix multiplier (it was just here for GAM550 path motion demo)
					trans->SetGlobalRot(Geommath::FromVectorPair(Geommath::WorldFront, p1 - p0) * Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI) * Geommath::RotationMat(Geommath::ROT_AXIS::X, PI * 0.5f)));
				}
				else
				{
					ENGINE_EXCEPT(str2wstr(Str(e)) + L"has no Transform3DCom!");
				}
			}
		}
	}
	);
}