/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/14/2020
- End Header ----------------------------*/

#pragma once
#include "engine/ecs/BaseComponentSystem.h"

namespace longmarch
{
	class PlayerControllerComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(PlayerControllerComSys);
		COMSYS_DEFAULT_COPY(PlayerControllerComSys);

		PlayerControllerComSys() = default;
		virtual void Update(double dt) override;
	};
}