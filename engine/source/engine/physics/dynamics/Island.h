#pragma once

#include <vector>
#include "engine/physics/dynamics/RigidBody.h"
#include "engine/physics/dynamics/Contact.inl"

namespace longmarch
{
    struct Island
    {
        // Solve the constraints in the island
        void Solve();

        // Add rigid body to the island
        void AddRigidBody(RigidBody* rb);

        // Add contact constraint to the island
        void AddContact(Contact* c);

        std::vector<std::shared_ptr<RigidBody>> m_bodies;
        std::vector<std::shared_ptr<Contact>> m_contacts;

        bool m_isSleeping;
    };
}
