#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/PerspectiveCameraCom.h"

namespace longmarch
{
	class PerspectiveCameraComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(PerspectiveCameraComSys);
		COMSYS_DEFAULT_COPY(PerspectiveCameraComSys);

		PerspectiveCameraComSys() {
			m_systemSignature.AddComponent<Transform3DCom>();
			m_systemSignature.AddComponent<PerspectiveCameraCom>();
		}

		virtual void Update(double dt) override
		{
			EARLY_RETURN(dt);
			ForEach(
				[dt](EntityDecorator e)
			{
				auto trans = e.GetComponent<Transform3DCom>();
				auto pos = trans->GetGlobalPos();
				auto rot = trans->GetGlobalRot();
				auto cam = e.GetComponent<PerspectiveCameraCom>()->GetCamera();
				cam->SetWorldPosition(pos);
				cam->SetGocalRotation(rot);
				cam->OnUpdate();
			}
			);
		}
	};
}