#pragma once

#include "engine/ecs/BaseComponent.h"

namespace AAAAgames
{
	/* Data class of LIfe Time */
	struct ActiveCom final : BaseComponent<ActiveCom>
	{
		ActiveCom() = default;
		explicit ActiveCom(bool active)
		{
			m_active = active;
		}

		void SetActive(bool active)
		{
			LOCK_GUARD2();
			m_active = active;
		}

		bool IsActive()
		{
			LOCK_GUARD2();
			return m_active;
		}

	private:
		bool m_active{ true };
	};
}
