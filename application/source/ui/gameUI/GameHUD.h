#pragma once
#include "engine/ui/BaseWidget.h"

namespace longmarch {
	class GameHUD final : public BaseWidget
	{
	public:
		GameHUD()
		{
			m_IsVisible = true;
		}
		void Render() override;
		void ShowEngineFPS();
	};
}