#pragma once

#include "engine/core/EngineCore.h"
#include "engine/math/Geommath.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

#include "dynamics/Island.h"
#include "collision/DynamicTree.h"

namespace longmarch
{
    class Scene : BaseAtomicClass
    {
    public:
        Scene() = default;
        explicit Scene(const Vec3f& gravity);
        ~Scene();

        LongMarch_Vector<LongMarch_Vector<RigidBody*>> BroadPhase(const LongMarch_Vector<RigidBody*>& rbs);
        LongMarch_Vector<Manifold> NarrowPhase(const LongMarch_Vector<RigidBody*>& island, float dt);

        void Solve(float dt);
        void Step(float dt); //!< move simulation of Scene forward by given timestep

        void SetGameWorld(GameWorld* world);
        void SetGravity(const Vec3f& g);
        std::shared_ptr<RigidBody> CreateRigidBody();
        void RemoveRigidBody(const std::shared_ptr<RigidBody>& rb);
        void RemoveAllBodies();

        void EnableSleep(bool enabled);
        void EnableFriction(bool enabled);
        void EnableUpdate(bool update);
        bool IsUpdateEnabled() const;

		void RenderDebug();

    private:
        LongMarch_Vector<std::shared_ptr<RigidBody>> m_rbList;
        LongMarch_UnorderedSet<Manifold> m_contactPairs;
        //DynamicAABBTree m_aabbTree;

        GameWorld* m_parentWorld{ nullptr };
        Vec3f m_gravity{ Vec3f(0,0,-9.8) };
        bool m_enableSleep{ true };
        bool m_enableFriction{ true };
        bool m_enableUpdate{ true };

        FastBVH::BVH<float, RigidBody*> m_bvh;
    };
}
