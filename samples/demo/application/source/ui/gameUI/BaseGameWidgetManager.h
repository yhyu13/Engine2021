#pragma once
#include "engine/ui/BaseWidgetManager.h"

namespace longmarch {
#define APP_WIG_MAN_NAME "app_wig_man"

	class BaseGameWidgetManager : public BaseWidgetManager
	{
	public:
		virtual void RenderUI() override;

		void LoadWidget(const fs::path& filepath);
	};
}
