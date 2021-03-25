#pragma once
#include "engine/ui/BaseWidget.h"
#include "../BaseGameWidgetManager.h"

namespace longmarch {

	/**
	 * @brief BaseWidget manager for 3D application editor
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
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