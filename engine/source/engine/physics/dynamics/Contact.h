#pragma once

#include "engine/math/Geommath.h"

namespace longmarch
{
    class RigidBody;

    struct Contact
    {
        // point of contact
        Vec3f m_pos;

        // penetration depth
        float m_penetration;
    };

    struct Manifold
    {
        // simple implementation for now, just record the two colliding rigid bodies and the contact point
        RigidBody* m_A;
        RigidBody* m_B;

        float m_intersectTime;

        // collision normal
        Vec3f m_normal;

        Vec3f m_gravity;

        Contact m_contact;

        float m_friction;
        bool m_staticCollision;

        friend inline bool operator==(const Manifold& lhs, const Manifold& rhs) {
            return lhs.m_A == rhs.m_A && lhs.m_B == rhs.m_B;
        }

        friend inline bool operator!=(const Manifold& lhs, const Manifold& rhs) {
            return !(lhs.m_A == rhs.m_A && lhs.m_B == rhs.m_B);
        }
    };
}