#include "application-precompiled-header.h"
#include "PlayerControllerComSys.h"
#include "engine/ecs/header/header.h"
#include "ecs/EntityType.h"
#include "engine/ui/ImGuiUtil.h"

void longmarch::PlayerControllerComSys::Update(double dt)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		break;
	default:
		return;
	}

	auto player = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
	auto trans = GetComponent<Transform3DCom>(player);

	auto player_cam = m_parentWorld->GetTheOnlyEntityWithType((EntityType)EngineEntityType::PLAYER_CAMERA);
	auto cam_trans = GetComponent<Transform3DCom>(player_cam);
	auto cam = GetComponent<PerspectiveCameraCom>(player_cam)->GetCamera();

	auto input = InputManager::GetInstance();

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
	bool bUINotHoldMouse = !ImGuiUtil::IsMouseCaptured(true);
	bool bUINotHoldKeyBoard = !ImGuiUtil::IsKeyBoardCaptured(true);
	// Apply friction
	{
		friction_local_v *= powf(0.001f, dt);
		friction_global_v *= powf(0.001f, dt);
	}
	if ((input->IsKeyTriggered(KEY_1) && bUINotHoldKeyBoard) || input->IsGamepadButtonTriggered(GAMEPAD_BUTTON_RIGHT_THUMB))
	{
		static Quaternion rot = trans->GetGlobalRot();
		static Vec3f pos = trans->GetGlobalPos();
		static Quaternion rot2 = trans->GetGlobalRot();
		static Vec3f pos2 = trans->GetGlobalPos();
		switch (cam->type)
		{
		case longmarch::PerspectiveCameraType::LOOK_AT:
			cam->type = PerspectiveCameraType::FIRST_PERSON;
			PRINT("Switch to FIRST_PERSON camera");
			break;
		case longmarch::PerspectiveCameraType::FIRST_PERSON:
			cam->type = PerspectiveCameraType::LOOK_AT;
			PRINT("Switch to LOOK AT camera");
			break;
		}
	}
	// Keyboard & Mouse inputs
	{
		if (input->IsKeyTriggered(KEY_2) && bUINotHoldKeyBoard)
		{
			Renderer3D::s_Data.directional_light_display_mode++;
			Renderer3D::s_Data.directional_light_display_mode %= 4;
		}

		if (input->IsKeyTriggered(KEY_LEFT_SHIFT) && bUINotHoldKeyBoard)
		{
			speed_up_multi = (speed_up_multi == 1.0f) ? 2.0f : (speed_up_multi == 2.0f) ? 1.0f : 1.0f;
			v_max = (v_max == 30.0f) ? 40.0f : (v_max == 40.0f) ? 30.0f : 30.0f;
		}
		switch (cam->type)
		{
		case longmarch::PerspectiveCameraType::LOOK_AT:
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && bUINotHoldMouse)
			{
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = LongMarch_Lerp(1.0f, 2.0f * rotation_speed, fabs(mouse_delta_pixel.x - pixel_threshold) / pixel_max_threshold);
				float y_multi = LongMarch_Lerp(1.0f, 2.0f * rotation_speed, fabs(mouse_delta_pixel.y - pixel_threshold) / pixel_max_threshold);

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
		case longmarch::PerspectiveCameraType::FIRST_PERSON:
			// UE4 like movement with MMB pressed :
			// panning on local right / left and global up / down
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && bUINotHoldMouse)
			{
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = LongMarch_Lerp(v_speed, v_max, fabs(mouse_delta_pixel.x - pixel_threshold) / pixel_max_threshold);
				float y_multi = LongMarch_Lerp(v_speed, v_max, fabs(mouse_delta_pixel.y - pixel_threshold) / pixel_max_threshold);

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
				if (input->IsKeyPressed(KEY_W) && bUINotHoldKeyBoard)
				{
					friction_local_v += v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_S) && bUINotHoldKeyBoard)
				{
					friction_local_v += -v_speed * Geommath::WorldFront;
				}
				if (input->IsKeyPressed(KEY_D) && bUINotHoldKeyBoard)
				{
					friction_local_v += v_speed * Geommath::WorldRight;
				}
				if (input->IsKeyPressed(KEY_A) && bUINotHoldKeyBoard)
				{
					friction_local_v += -v_speed * Geommath::WorldRight;
				}
				mouse_delta_pixel = input->GetCursorPositionDeltaXY();
				constexpr float pixel_threshold = 1.0f;
				constexpr float pixel_max_threshold = 50.0f;
				float x_multi = LongMarch_Lerp(0.0f, 2.0f * rotation_speed, (fabs(mouse_delta_pixel.x) - pixel_threshold) / pixel_max_threshold);
				float y_multi = LongMarch_Lerp(0.0f, 2.0f * rotation_speed, (fabs(mouse_delta_pixel.y) - pixel_threshold) / pixel_max_threshold);

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
			case longmarch::PerspectiveCameraType::LOOK_AT:
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
			case longmarch::PerspectiveCameraType::FIRST_PERSON:
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

					float x_multi = LongMarch_Lerp(v_speed, v_max, left_trigger);
					float y_multi = LongMarch_Lerp(v_speed, v_max, right_trigger);

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
	case longmarch::PerspectiveCameraType::LOOK_AT:
		trans->SetLocalVel(Vec3f(0));
		trans->SetGlobalVel(Vec3f(0));
		trans->SetGlobalRotVel(-rv_yaw);
		trans->SetLocalRotVel(-rv_pitch);
		break;
	case longmarch::PerspectiveCameraType::FIRST_PERSON:
		trans->SetLocalVel(friction_local_v + local_v);
		trans->SetGlobalVel(friction_global_v + global_v);
		trans->SetGlobalRotVel(rv_yaw);
		trans->SetLocalRotVel(rv_pitch);
		break;
	}

	// Copy the player's transform into player camera's
	cam_trans->Copy(trans.GetPtr());
}