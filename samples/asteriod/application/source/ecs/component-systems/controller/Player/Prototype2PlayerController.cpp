/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 11/25/2020
- End Header ----------------------------*/


#include "application-precompiled-header.h"
#include "Prototype2PlayerController.h"
#include "engine/ecs/header/header.h"
#include "engine/ui/ImGuiUtil.h"
#include "ecs/EntityType.h"
#include "events/CustomEvents.h"

void longmarch::Prototype2PlayerController::Init()
{
	m_bgmChannel = AudioManager::GetInstance()->PlaySoundByName("bgm0", AudioVector3{ 0,0,0 }, -10, 1);
}

void longmarch::Prototype2PlayerController::PreRenderUpdate(double dt)
{
	auto audio = AudioManager::GetInstance();
	auto camera = m_parentWorld->GetTheOnlyEntityWithType((EntityType)EngineEntityType::PLAYER_CAMERA);
	auto trans = GetComponent<Transform3DCom>(camera);
	audio->Set3dListenerAndOrientation(trans->GetGlobalPos());
	audio->SetChannel3dPosition(m_bgmChannel, trans->GetGlobalPos());
}

void longmarch::Prototype2PlayerController::Update(double dt)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		break;
	default:
		return;
	}

	auto player = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
	auto input = InputManager::GetInstance();

	/******************************************************************************
	**	Metal Slug like player movement for both keyboard & mouse contrl
	**  and gamepad control
	******************************************************************************/

	constexpr float gamepad_stick_threshold = .15f;
	constexpr float v_speed = 7.f;
	constexpr float v_max = 30.f;

	static float lerp_multiplier = 8.f;
	const static Quaternion tilted_left_target = Geommath::ToQuaternion(Vec3f(0,-30,0) * DEG2RAD);
	const static Quaternion tilted_right_target = Geommath::ToQuaternion(Vec3f(0, 30, 0) * DEG2RAD);
	const static Quaternion untilted_target = Geommath::ToQuaternion(Vec3f(0, 0, 0) * DEG2RAD);

	static Vec3f friction_global_v;
	static Quaternion global_q;
	static Vec2f gamepad_left_aixs;
	static Vec2f gamepad_right_aixs;

	bool bUINotHoldMouse = !ImGuiUtil::IsMouseCaptured();
	bool bUINotHoldKeyBoard = !ImGuiUtil::IsKeyBoardCaptured();
	
	{
		friction_global_v *= powf(0.001f, dt);
	}

	{
		// Gamepad inputs
		if (input->IsGamepadActive())
		{
			DEBUG_PRINT("Gampad control not implemented!");
		}
		else // Keyboard & Mouse inputs
		{
			// rotate to look at mouse cursor 
			{
				// THIS ONLY WORK WHEN BOTH PLAYER and PLAYER CAMERA are at the center of the screen
				// Since we look downward from +z to -z,
				// and the view space is in fact right-handed,
				// so we can simply use the view vector
				auto sudo_look_at_vector = Vec3f(input->GetCursorViewPosition(), 0);
				global_q = Geommath::FromVectorPair(Geommath::WorldFront, sudo_look_at_vector);
			}
			{
				// The player object should be static
				if (input->IsKeyPressed(KEY_W))
				{
					friction_global_v += v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_S))
				{
					friction_global_v += -v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_D))
				{
					friction_global_v += v_speed * Geommath::WorldRight;
				}
				if (input->IsKeyPressed(KEY_A))
				{
					friction_global_v += -v_speed * Geommath::WorldRight;
				}
			}
		}
	}
	// Clamping
	{
		friction_global_v = glm::clamp(friction_global_v, Vec3f(-v_max), Vec3f(v_max));
	}
	
	// Tilting player toward +/- x axis
	{	
		auto trans = GetComponent<Transform3DCom>(player);
		trans->SetGlobalVel(friction_global_v);
		trans->SetGlobalRot(global_q);
	}
}

void longmarch::Prototype2PlayerController::PostRenderUpdate(double dt)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		break;
	default:
		return;
	}

	auto input = InputManager::GetInstance();
	auto player = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
	// Spawn projectile on left click
	{
		if (input->IsMouseButtonTriggered(MOUSE_BUTTON_LEFT))
		{
			auto trans = GetComponent<Transform3DCom>(player);
			auto queue = EventQueue<Prototype2EventType>::GetInstance();
			auto e = MemoryManager::Make_shared<Prototype2PlayerGenerateProjectile>(trans->GetGlobalRot(), trans->GetGlobalPos());
			queue->Publish(e);
		}

		if (input->IsMouseButtonTriggered(MOUSE_BUTTON_RIGHT))
		{
			auto trans = GetComponent<Transform3DCom>(player);
			auto queue = EventQueue<Prototype2EventType>::GetInstance();
			auto e = MemoryManager::Make_shared<Prototype2PlayerGenerateSpaceShip>(trans->GetGlobalRot(), trans->GetGlobalPos());
			queue->Publish(e);
		}
	}
}
