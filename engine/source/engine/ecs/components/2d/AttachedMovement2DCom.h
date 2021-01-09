#pragma once

#include "engine/ecs/BaseComponent.h"

namespace longmarch
{
	/* Data class of sprite */
	struct CACHE_ALIGN32 AttachedMovementCom final : BaseComponent<AttachedMovementCom> {

		AttachedMovementCom()
		{
			followPos = false;
			followRot = false;
			followVelocity = false;
			rPos = vec2(0);
			rVelocity = vec2(0);
			rRot = 0;
		}

		bool followPos;
		bool followRot;
		bool followVelocity;
		vec2 rPos;
		vec2 rVelocity;
		float rRot;
	};
}
