#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	/* Data class of sprite */
	struct MS_ALIGN8 AttachedMovementCom final : public BaseComponent<AttachedMovementCom> 
	{
		AttachedMovementCom()
		{
			followPos = false;
			followRot = false;
			followVelocity = false;
			rPos = Vec2f(0);
			rVelocity = Vec2f(0);
			rRot = 0;
		}

		bool followPos;
		bool followRot;
		bool followVelocity;
		Vec2f rPos;
		Vec2f rVelocity;
		float rRot;
	};
}
