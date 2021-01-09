#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Transform3DCom.h"

namespace longmarch
{
	class Transform3DComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Transform3DComSys);
		COMSYS_DEFAULT_COPY(Transform3DComSys);

		Transform3DComSys()
		{
			m_systemSignature.AddComponent<Transform3DCom>();
		}

		virtual void PreRenderUpdate(double dt) override
		{
			ForEach(
				[dt, this](EntityDecorator e)
			{
#ifdef DEBUG_DRAW
				DebugDraw(e);
#endif
			}
			);
		}

		virtual void Update(double dt) override
		{
			EARLY_RETURN(dt);
			ForEach(
				[dt](EntityDecorator e)
			{
				e.GetComponent<Transform3DCom>()->Update(dt);
			}
			);
		}

#ifdef DEBUG_DRAW
	private:
		//! Debug drawing for transform component
		void DebugDraw(EntityDecorator e);
#endif
	};
}