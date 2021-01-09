#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {

	/**
	 * @brief Main menu for 3D application editor
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class _3DEngineMainMenu : public BaseWidget
	{
	public:
		NONCOPYABLE(_3DEngineMainMenu);
		_3DEngineMainMenu()
		{
			m_IsVisible = true;
			m_Size = ScaleSize({ 500, 535 });
		}
		virtual void Render() override;

		void RenderEngineSettingMenu();
		void RenderEngineGraphicSettingMenu();
	private:
		ImVec2 m_Size;
	};
}