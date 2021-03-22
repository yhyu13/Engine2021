#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Display all logging info
	 * It should act like a dockable menu
	 */
	class SceneDock final : public BaseWidget
	{
	public:
		NONCOPYABLE(SceneDock);
		SceneDock();
		virtual void Render() override;

	private:
		ImVec2 m_Size;
	};
}
