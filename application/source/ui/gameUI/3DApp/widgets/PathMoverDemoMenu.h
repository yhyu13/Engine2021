#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	/**
	 * @brief Main menu for 3D application editor
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class PathMoverDemoMenu : public BaseWidget
	{
	public:
		NONCOPYABLE(PathMoverDemoMenu);
		PathMoverDemoMenu()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 535 });
		}
		virtual void Render() override;

		void RenderPathMotionMenu();
	private:
		ImVec2 m_Size;
	};
}