#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {

	/**
	 * @brief Display all ENG_TIME() timed code blcoks
	 */
	class EngineProfilerPage final : public BaseWidget
	{
	public:
		NONCOPYABLE(EngineProfilerPage);
		EngineProfilerPage()
		{
			m_IsVisible = false;
		}
		void Render() override;
	};
}