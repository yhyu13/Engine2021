#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all logging info
	 * It should act like a dockable menu
	 */
	class EngineConsoleDock final : public BaseWidget
	{
	public:
		NONCOPYABLE(EngineConsoleDock);
		EngineConsoleDock();
		virtual void Render() override;

	private:
		ImVec2 m_Size;
	};
}
