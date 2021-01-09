#pragma once
#include "engine/ui/BaseWidget.h"

namespace AAAAgames {

	/**
	 * @brief Display all ENG_TIME() timed code blcoks
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
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