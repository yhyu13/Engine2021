#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	class GameProfilerPage final : public BaseWidget
	{
	public:
		GameProfilerPage()
		{
			m_IsVisible = false;
		}
		void Render() override;
	};
}