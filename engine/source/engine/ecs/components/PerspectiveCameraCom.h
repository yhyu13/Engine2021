#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/renderer/camera/PerspectiveCamera.h"

namespace longmarch
{
	/* Data class of mesh, material */
	struct MS_ALIGN8 PerspectiveCameraCom final: public BaseComponent<PerspectiveCameraCom>
	{
		void SetCamera(const PerspectiveCamera & cam);
		PerspectiveCamera* GetCamera();

		virtual void JsonSerialize(Json::Value& value) const override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:
		PerspectiveCamera m_camera;
	};
}
