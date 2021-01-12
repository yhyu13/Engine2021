#pragma once

#include "Contact.h"
#include "RigidBody.h"
#include "engine/core/utility/TypeHelper.h"

/*
    Custum hash function for Manifold
*/
namespace std
{
    template <>
    struct hash<longmarch::Manifold>
    {
        std::size_t operator()(const longmarch::Manifold& e) const
        {
            size_t hash;
            LongMarch_HashCombine(hash, e.m_A->GetEntity(), e.m_B->GetEntity());
            return hash;
        }
    };
}