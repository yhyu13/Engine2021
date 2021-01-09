#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all logging info
	 * It should act like a dockable menu
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class EngineConsoleDock final : public BaseWidget
	{
	public:
		NONCOPYABLE(EngineConsoleDock);
		EngineConsoleDock()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 400 });
		}
		virtual void Render() override;

	private:
		ImVec2 m_Size;
	};
}
