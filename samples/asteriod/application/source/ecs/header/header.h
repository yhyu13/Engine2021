/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 08/29/2020
- End Header ----------------------------*/

#pragma once

#include "../EntityType.h"

#include "../components/AIControllerCom.h"

#include "../component-systems/controller/AIControllerComSys.h"
#include "../component-systems/controller/Player/PlayerControllerComSys.h"
#include "../component-systems/controller/Player/Prototype2PlayerController.h"
#include "../component-systems/controller/Player/Prototype2PlayerArsenalComSys.h"
#include "../component-systems/controller/Camera/Prototype2CameraController.h"
#include "../component-systems/controller/NPC/NPCPathFindingControllerComSys.h"
#include "../component-systems/controller/Enemy/Prototype2EnemyGeneratorSystem.h"
#include "../object-factory/MainObjectFactory.h"