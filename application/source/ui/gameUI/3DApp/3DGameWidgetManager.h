#pragma once
#include "engine/ui/BaseWidget.h"
#include "../BaseGameWidgetManager.h"

namespace AAAAgames {

	/**
	 * @brief BaseWidget manager for 3D application editor
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class _3DGameWidgetManager final : public BaseGameWidgetManager
	{
	public:
		NONCOPYABLE(_3DGameWidgetManager);
		_3DGameWidgetManager();

		virtual void PushWidgetStyle() override {};
		virtual void PopWidgetStyle() override {};
	};
}