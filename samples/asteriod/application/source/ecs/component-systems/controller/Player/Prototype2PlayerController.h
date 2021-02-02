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

#pragma once
#include "engine/ecs/BaseComponentSystem.h"

namespace longmarch
{
	class Prototype2PlayerController final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Prototype2PlayerController);
		COMSYS_DEFAULT_COPY(Prototype2PlayerController);

		Prototype2PlayerController() = default;

		virtual void Init() override;
		virtual void PreRenderUpdate(double dt) override;
		virtual void Update(double dt) override;
		virtual void PostRenderUpdate(double dt) override;

	private:
		int m_bgmChannel;
	};
}