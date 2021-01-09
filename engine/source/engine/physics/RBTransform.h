#pragma once

#include "engine\math\Geommath.h"

namespace AAAAgames
{
    struct RBTransform
    {
        Vec3f m_pos;
        Quaternion m_rot;
        Vec3f m_scale;
    };
}