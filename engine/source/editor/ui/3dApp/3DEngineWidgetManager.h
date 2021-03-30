#pragma once
#include "engine/ui/BaseWidget.h"
#include "../common/BaseEngineWidgetManager.h"

namespace longmarch {

	/**
	 * @brief BaseWidget manager for 3D application editor
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class _3DEngineWidgetManager final : public BaseEngineWidgetManager
	{
	public:
		NONCOPYABLE(_3DEngineWidgetManager);
		_3DEngineWidgetManager();

		virtual void PushWidgetStyle() override {};
		virtual void PopWidgetStyle() override {};
	};
}