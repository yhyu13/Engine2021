#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "editor/renderer/render-pass/PickingPass3D.h"

namespace AAAAgames
{
	/**
	 * @brief Editor's object picking sytem
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class EditorPickingComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(EditorPickingComSys);
		COMSYS_DEFAULT_COPY(EditorPickingComSys);

		EditorPickingComSys() = default;
		virtual void Init() override;
		virtual void RenderUI() override;

	private:
		void ManipulatePickedEntityGizmos(const Entity& e);

	private:
		PickingPass m_renderPass;
	};
}