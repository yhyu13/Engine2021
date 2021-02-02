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
#include "Prototype2CameraController.h"
#include "engine/ecs/header/header.h"
#include "ecs/EntityType.h"

longmarch::Prototype2CameraController::Prototype2CameraController()
{
	springArm.relative_pos = Vec3f{ 0.f,0.f,20.f };
}

void longmarch::Prototype2CameraController::Update(double dt)
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
	auto player_cam_trans = GetComponent<Transform3DCom>(player_cam);

	player_cam_trans->SetGlobalPos(trans->GetGlobalPos() + springArm.relative_pos);
}
