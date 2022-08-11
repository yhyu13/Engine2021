#pragma once

#include "engine/ecs/BaseComponent.h"

namespace longmarch
{
    /* Data class of LIfe Time */
    struct MS_ALIGN8 ActiveCom final : public BaseComponent<ActiveCom>
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
        bool m_active{true};
    };
}
