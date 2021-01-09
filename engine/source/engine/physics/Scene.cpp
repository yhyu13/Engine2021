#include "engine-precompiled-header.h"
#include "Scene.h"
#include "engine/physics/CollisionsManager.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#define MAX_ITERATIONS 3

namespace AAAAgames
{
    Scene::Scene(const Vec3f& gravity)
        : m_gravity(gravity),
        m_enableSleep(true),
        m_enableFriction(true),
        m_enableUpdate(true)
    {

    }

    Scene::~Scene()
    {
        RemoveAllBodies();
    }

    void Scene::BroadPhase(A4GAMES_Vector<Manifold>& contacts)
    {
        //contacts.clear();

        // naive implementation
        //for (auto& rb : m_rbList)
        //{

        //}
    }

    A4GAMES_Vector<Manifold> Scene::NarrowPhase(A4GAMES_Vector<Manifold>& contacts, f32 dt)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // TODO: use BVH to optimize
        ///////////////////////////////////////////////////////////////////////////////////////////

        A4GAMES_Vector<Manifold> manifold;

        for (auto iter = m_rbList.begin(); iter != m_rbList.end(); ++iter)
        {
            for (auto iter2 = iter + 1; iter2 != m_rbList.end(); ++iter2)
            {
                // get the type of body
                auto& rb1 = *iter;
                auto& rb2 = *iter2;

                RBType rb1Type = rb1->GetRBType();
                RBType rb2Type = rb2->GetRBType();

                // Skip two static bodies
                if (rb1Type == RBType::staticBody && rb2Type == RBType::staticBody)
                {
                    continue;
                }

                if (rb1->m_entityTypeIngoreSet.Contains(rb2->GetEntity().m_type)
                    || rb2->m_entityTypeIngoreSet.Contains(rb1->GetEntity().m_type))
                {
                    continue;
                }
                
                Manifold contactManifold;

                // if there is collision, store the manifold and move on to the next pair
                if (DynamicShapevsShape(rb1->GetShape(), rb1->GetLinearVelocity(), rb2->GetShape(), rb2->GetLinearVelocity(), dt, contactManifold))
                {
                    contactManifold.m_A = rb1.get();
                    contactManifold.m_B = rb2.get();

                    contactManifold.m_gravity = m_gravity;
                    contactManifold.m_friction = (rb1->GetFriction() + rb2->GetFriction()) * 0.5f;

                    manifold.push_back(contactManifold);
                }
            }
        }

        return manifold;
    }

    void Scene::Solve(f32 dt)
    {
        std::shared_ptr<Shape> shapePtr = nullptr;

        // update all rigid bodies using euler
        for (auto& rb : m_rbList)
        {
            // no need to update static objects
            if (rb->GetRBType() == RBType::dynamicBody)
            {

                rb->SetPrevWorldPosition(rb->GetWorldPosition());

                if (rb->IsCollided())
                {
                    // apply gravity

                    rb->ApplyLinearForce(m_gravity * rb->GetGravityScale());

                    rb->SetPrevWorldPosition(rb->GetWorldPosition());

                    rb->SetLinearVelocity((rb->GetLinearVelocity() + rb->GetLinearAcceleration() * rb->GetSolveTimeLeft()));
                    rb->SetWorldPosition(rb->GetWorldPosition() + rb->GetLinearVelocity() * rb->GetSolveTimeLeft());


                    //rb->SetLinearVelocity(Vec3f(0.0f, 0.0f, 0.0f));
                }

                else
                {
                    // apply gravity
                    rb->ApplyLinearForce(m_gravity * rb->GetGravityScale());

                    rb->SetPrevWorldPosition(rb->GetWorldPosition());

                    rb->SetLinearVelocity((rb->GetLinearVelocity() + rb->GetLinearAcceleration() * dt));
                    rb->SetWorldPosition(rb->GetWorldPosition() + rb->GetLinearVelocity() * dt);
                }

                // apply damping to the velocities
                rb->SetLinearVelocity(rb->GetLinearVelocity() * 1.0f / (1.0f + dt * rb->GetLinearDamping()));
            }

            //////////////////////////////////////////////
            // update the shape associated with the object
            //////////////////////////////////////////////
            shapePtr = rb->GetShape();
            
            if (shapePtr.get() != nullptr)
            {
                
                //Vec3f posDiff = rb->GetWorldPosition() - rb->GetPrevWorldPosition();

                // for now just check for AABB shapes
                if (shapePtr->GetType() == Shape::SHAPE_TYPE::AABB)
                {
                    //AABB* aabbPtr = dynamic_cast<AABB*>(shapePtr.get());
                    std::shared_ptr<AABB> aabbPtr = std::dynamic_pointer_cast<AABB>(shapePtr);

                    //aabbPtr->SetMin(aabbPtr->GetMin() + posDiff);
                    //aabbPtr->SetMax(aabbPtr->GetMax() + posDiff);
                    aabbPtr->SetCenter(rb->GetWorldPosition() + rb->GetColliderDisplacement());
                }

                //switch (shapePtr->GetType())
                //{
                //case Shape::SHAPE_TYPE::AABB:
                //    std::dynamic_pointer_cast<AABB>(shapePtr)->
                //}

                // update the AABB in the AABB tree
                //m_aabbTree.UpdateObject(rb);
            }
        }

        for (auto& elem : m_rbList)
            elem->ClearAllForces();
    }

    // move simulation of Scene forward by given timestep
    void Scene::Step(f32 dt)
    {
        LOCK_GUARD2();
        m_contactPairs.clear();

        // reset collision status of all rigid bodies
        for (auto& elem : m_rbList)
        {
            elem->SetCollisionStatus(false, dt);
        }

        // do broadphase collision check
        A4GAMES_Vector<Manifold> contacts;
        
        // loop collision check and resolution until either max. iterations achieved or no collisions detected
        for (unsigned int i = 0; i < MAX_ITERATIONS; ++i)
        {
            contacts.clear();
            //BroadPhase(contacts);

            // NarrowPhase
            A4GAMES_Vector<Manifold> manifold = NarrowPhase(contacts, dt);

            if (manifold.empty())
                break;
            
            // temporary solution
            // solve contacts first, then update positions etc.
            
            
            for (auto& elem : manifold)
            {
                // resolve each contact in the list
                ResolveCollision(elem, dt, m_enableFriction);
                size_t hash;
                A4GAMES_HashCombine(hash, elem.m_A->GetEntity(), elem.m_B->GetEntity());
                m_contactPairs[hash] = elem;
            }
        }

        // construct islands, then call Solve() in islands
        // for now just do regular integration (euler) since islands aren't implemented yet
        Solve(dt);
        
        // addition collision check to push out anything with static collision so that objects don't "sink" into the ground
        {
            A4GAMES_Vector<Manifold> manifold = NarrowPhase(contacts, dt);

            for (auto& elem : manifold)
            {
                // resolve each contact in the list
                ResolveCollision(elem, dt, m_enableFriction);
                size_t hash;
                A4GAMES_HashCombine(hash, elem.m_A->GetEntity(), elem.m_B->GetEntity());
                m_contactPairs[hash] = elem;
            }
        }

        // add code for collision event here using pairs in m_contactPairs
        {
            auto queue = EventQueue<EngineEventType>::GetInstance();
            for (const auto& [_, elem] : m_contactPairs)
            {
                auto e1 = EntityDecorator{ elem.m_A->GetEntity(), m_parentWorld };
                auto e2 = EntityDecorator{ elem.m_B->GetEntity(), m_parentWorld };
                auto e = MemoryManager::Make_shared<EngineCollisionEvent>(e1, e2, (void*)(&elem));
                queue->Publish(e);
            }
        }

        // add code for collision event here using pairs in m_contactPairs

    }

    // by default give a AABB
    std::shared_ptr<RigidBody> Scene::CreateRigidBody()
    {
        LOCK_GUARD2();
        //mutex mtxTest;

        auto rb = MemoryManager::Make_shared<RigidBody>();
        //rb->SetShape(Shape::SHAPE_TYPE::AABB);

        m_rbList.push_back(rb);
        //m_aabbTree.InsertObject(rb);

        return rb;
    }

    // note: need to make sure that after calling the remove functions, the relevant rigid bodies also have the pointers removed
    void Scene::RemoveRigidBody(const std::shared_ptr<RigidBody>& rb)
    {
        LOCK_GUARD2();
        std::erase(m_rbList, rb);
    }

    void Scene::RemoveAllBodies()
    {
        LOCK_GUARD2();
        m_rbList.clear();
    }

    void Scene::SetGameWorld(GameWorld* world)
    {
        LOCK_GUARD2();
        m_parentWorld = world;
    }

    void Scene::SetGravity(const Vec3f& g)
    {
        LOCK_GUARD2();
        m_gravity = g;
    }

    void Scene::EnableSleep(bool enabled)
    {
        LOCK_GUARD2();
        m_enableSleep = enabled;
    }

    void Scene::EnableFriction(bool enabled)
    {
        LOCK_GUARD2();
        m_enableFriction = enabled;
    }

    void Scene::EnableUpdate(bool update)
    {
        LOCK_GUARD2();
        m_enableUpdate = update;
    }

    bool Scene::IsUpdateEnabled() const
    {
        LOCK_GUARD2();
        return m_enableUpdate;
    }

    void Scene::RenderDebug()
    {
        LOCK_GUARD2();
        // render the BVH tree
        //m_aabbTree.Render();

        // render all the rigid body shapes
        for (auto& elem : m_rbList)
        {
            std::shared_ptr<Shape> shapePtr = elem->GetShape();

            if(shapePtr != nullptr && elem->GetShape()->GetType() != Shape::SHAPE_TYPE::EMPTY)
                elem->Render();
        }
    }
}