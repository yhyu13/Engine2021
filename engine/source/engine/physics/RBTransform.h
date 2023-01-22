#pragma once

#include "engine\math\Geommath.h"

namespace longmarch
{
    struct RBTransform
    {
        Vec3f m_pos;
        Quaternion m_rot;
        Vec3f m_scale;

        friend bool operator==(const RBTransform& Lhs, const RBTransform& Rhs)
        {
            return Lhs.m_pos == Rhs.m_pos && Lhs.m_rot == Rhs.m_rot && Lhs.m_scale == Rhs.m_scale;
        }

        friend bool operator!=(const RBTransform& Lhs, const RBTransform& Rhs)
        {
            return !(Lhs == Rhs);
        }
    };
}
