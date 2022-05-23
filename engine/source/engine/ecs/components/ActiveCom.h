#pragma once

#include "engine/ecs/BaseComponent.h"

namespace longmarch
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
			LOCK_GUARD();
			m_active = active;
		}

		bool IsActive()
		{
			LOCK_GUARD();
			return m_active;
		}

	private:
		bool m_active{ true };
	};
}
