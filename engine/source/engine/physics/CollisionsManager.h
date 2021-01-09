#pragma once
#include "Circle.h"
#include "AABB.h"
#include "OOBB.h"
#include "dynamics/Contact.h"

namespace AAAAgames
{
	bool AABBCollisions(Shape* AABB1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2);

	bool CircleCollisions(Shape* Circle1, float PosX1, float PosY1,
		Shape* Circle2, float PosX2, float PosY2);

	bool CircleAABBCollisions(Shape* Circle1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2);

	bool AABBCircleCollisions(Shape* AABB1, float PosX1, float PosY1,
		Shape* Circle2, float PosX2, float PosY2);

	bool CircleReflection(Shape* Circle1, float PosX1, float PosY1,
		Shape* Circle, float PosX2, float PosY2);

	bool ResolveCollision(Shape* AABB1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2);

	bool StaticCirclevsCircle(const std::shared_ptr<Circle>& circle1, const std::shared_ptr<Circle>& circle2);

	bool DynamicCirclevsCircle(const std::shared_ptr<Circle>& circle1, const Vec3f& Vel1,
		const std::shared_ptr<Circle>& circle2, const Vec3f& Vel2,
		float dt, Manifold& manifold);

	bool StaticAABBvsAABB(const std::shared_ptr<AABB>& AABB1, const std::shared_ptr<AABB>& AABB2);

	bool DynamicAABBvsAABB(const std::shared_ptr<AABB>& AABB1, const Vec3f& Vel1,
						   const std::shared_ptr<AABB>& AABB2, const Vec3f& Vel2,
						   float dt, Manifold& manifold);

	bool DynamicShapevsShape(const std::shared_ptr<Shape>& shape1, const Vec3f& Vel1,
							 const std::shared_ptr<Shape>& shape2, const Vec3f& Vel2,
							 float dt, Manifold& manifold);

	/*
	void ResolveAABBCollision(shared_ptr<RigidBody>& rb, Vec3f collisionNormal, float collisionTime);

	void ResolveCircleCollision(shared_ptr<RigidBody>& rb, Vec3f collisionNormal, float collisionTime);
	*/

	void ResolveStaticCollision(const Manifold& manifold);

	void ResolveCollision(const Manifold& manifold, float dt, bool enableFriction);

	class CollisionsManager
	{
	public:

		static CollisionsManager* GetInstance()
		{
			static CollisionsManager instance;
			return &instance;
		}
		bool CheckCollisionAndGenerateDetection(Shape* pShape1, float PosX1, float PosY1,
			Shape* pShape2, float PosX2, float PosY2);

		bool (*CollisionFunctions[(unsigned int)Shape::SHAPE_TYPE::NUM][(unsigned int)Shape::SHAPE_TYPE::NUM])
			(Shape* pShape1, float PosX1, float PosY1,
				Shape* pShape2, float PosX2, float PosY2);
	private:
		CollisionsManager();
	};

}