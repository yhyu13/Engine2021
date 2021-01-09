#include "engine-precompiled-header.h"
#include "CollisionsManager.h"

#include "dynamics/RigidBody.h"

namespace longmarch
{
	//TODO: Recheck this works for A->B and not B->A
	glm::vec2 CheckDirection(glm::vec2 diff)
	{
		glm::vec2 direction[] = {
			glm::vec2(0.0f, 1.0f),	// up

			glm::vec2(1.0f, 0.0f),	// right

			glm::vec2(0.0f, -1.0f),	// down

			glm::vec2(-1.0f, 0.0f) // left
		};

		diff = glm::normalize(diff);

		float max = 0.0f;
		float dot_prod;
		int best = -1;
		for (int i = 0; i < 4; ++i)
		{
			dot_prod = glm::dot(diff, direction[i]);

			if (dot_prod > max)
			{
				max = dot_prod;
				best = i;
			}
		}

		return direction[best];
	}

	bool longmarch::CircleCollisions(Shape* Circle1, float PosX1, float PosY1,
		Shape* Circle2, float PosX2, float PosY2)
	{
		throw NotImplementedException();
		////Collision Math And Logic
		//float CCDistSq, Radius1, Radius2;

		//Radius1 = ((Circle*)Circle1)->Radius;
		//Radius2 = ((Circle*)Circle2)->Radius;

		//glm::vec2 Pos2(PosX2, PosY2);
		//glm::vec2 Pos1(PosX1, PosY1);

		//CCDistSq = pow((PosX2 - PosX1), 2) + pow((PosY2 - PosY1), 2);

		////Actual Collision test
		//float dissq = pow((Radius1 + Radius2), 2);
		//if (CCDistSq > dissq)
		//	return false;
		//else
		//{
		//	glm::vec2 CenterDistance = Pos2 - Pos1;
		//	glm::vec2 clamped1 = glm::clamp(CenterDistance, -Radius1, Radius1);
		//	glm::vec2 clamped2 = glm::clamp(CenterDistance, -Radius2, Radius2);

		//	glm::vec2 edgepoint1 = CenterDistance + clamped1;
		//	glm::vec2 edgepoint2 = CenterDistance + clamped2;

		//	glm::vec2 separation = edgepoint2 - edgepoint1;

		//	float penetration = std::max(separation.x, separation.y);
		//	glm::vec2 Collision_Normal = CheckDirection(separation);

		//	if (separation.x > 0)// && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - separation.x / 100;
		//		PosX2 = PosX2 + separation.x / 100;
		//	}
		//	else //if(Separation.x < 0 && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - separation.x / 100;
		//		PosX2 = PosX2 + separation.x / 100;
		//	}
		//	if (separation.y > 0)// && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - separation.y / 100;
		//		PosY2 = PosY2 + separation.y / 100;
		//	}
		//	else //if (Separation.y < 0 && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - separation.y / 100;
		//		PosY2 = PosY2 + separation.y / 100;
		//	}
		//	return true;
		//}
	}

	bool longmarch::AABBCollisions(Shape* AABB1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2)
	{
		throw NotImplementedException();
		////Collision Math And Logic
		//AABB* pAABB1 = (AABB*)AABB1;
		//AABB* pAABB2 = (AABB*)AABB2;

		//glm::vec2 tl0, br0, tl1, br1;

		//glm::vec2 Pos1(PosX1, PosY1);
		//glm::vec2 Pos2(PosX2, PosY2);

		//glm::vec2 AABB_HalfExtents1((pAABB1->GetWidth() / 2), (pAABB1->GetHeight() / 2));
		//glm::vec2 AABB_HalfExtents2((pAABB2->GetWidth() / 2), (pAABB2->GetHeight() / 2));

		////Rectangle 1
		//tl0.x = PosX1 - (pAABB1->GetWidth() / 2);
		//tl0.y = PosY1 + (pAABB1->GetHeight() / 2);
		//br0.x = PosX1 + (pAABB1->GetWidth() / 2);
		//br0.y = PosY1 - (pAABB1->GetHeight() / 2);

		////Rectangle 2
		//tl1.x = PosX2 - (pAABB2->GetWidth() / 2);
		//tl1.y = PosY2 + (pAABB2->GetHeight() / 2);
		//br1.x = PosX2 + (pAABB2->GetWidth() / 2);
		//br1.y = PosY2 - (pAABB2->GetHeight() / 2);

		////Actual Collision Test
		//if (br0.x < tl1.x || tl0.y < br1.y ||
		//	br1.x < tl0.x || br0.y > tl1.y)
		//	return false;

		////Trying for separation window Between two bodies for impulse throw
		////To find out the exact position of collision and throw bodies back accordingly
		//else
		//{
		//	glm::vec2 CenterDistance = Pos2 - Pos1;
		//	glm::vec2 clamped1 = glm::clamp(CenterDistance, -AABB_HalfExtents1, AABB_HalfExtents1);
		//	glm::vec2 clamped2 = glm::clamp(CenterDistance, -AABB_HalfExtents2, AABB_HalfExtents2);

		//	glm::vec2 EdgePoint1 = clamped1 + CenterDistance;
		//	glm::vec2 EdgePoint2 = clamped2 + CenterDistance;
		//	glm::vec2 Separation = EdgePoint2 - EdgePoint1;

		//	float penetration = std::min(Separation.x, Separation.y);
		//	glm::vec2 Collision_Normal = CheckDirection(Separation);

		//	//if (Collision_Normal.x == 0 && Collision_Normal.y == 1)
		//	//{
		//	//	std::cout << "\n TOP ";
		//	//}
		//	//else if (Collision_Normal.x == 1 && Collision_Normal.y == 0)
		//	//{
		//	//	std::cout << "\n RIGHT ";
		//	//}
		//	//else if (Collision_Normal.x == -1 && Collision_Normal.y == 0)
		//	//{
		//	//	std::cout << "\n LEFT ";
		//	//}
		//	//else if (Collision_Normal.x == 0 && Collision_Normal.y == -1)
		//	//{
		//	//	std::cout << "\n BOTTOM ";
		//	//}

		//	//Conditional trials
		//	if (Separation.x > 0)// && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - Separation.x / 100;
		//		PosX2 = PosX2 + Separation.x / 100;
		//	}
		//	else //if(Separation.x < 0 && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - Separation.x / 100;
		//		PosX2 = PosX2 + Separation.x / 100;
		//	}
		//	if (Separation.y > 0)// && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - Separation.y / 100;
		//		PosY2 = PosY2 + Separation.y / 100;
		//	}
		//	else //if (Separation.y < 0 && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - Separation.y / 100;
		//		PosY2 = PosY2 + Separation.y / 100;
		//	}
		//	return true;

		//}
	}

	bool longmarch::CircleAABBCollisions(Shape* Circle1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2)
	{
		throw NotImplementedException();
		////Collision Math And Logic
		//float Radius1;
		//Radius1 = ((Circle*)Circle1)->Radius;

		////Circle* pCircle1 = (Circle*)Circle1;
		//AABB* pAABB2 = (AABB*)AABB2;

		////Half Extents for reaching the edge point from center
		//glm::vec2 AABB_HalfExtents(pAABB2->GetWidth() / 2, pAABB2->GetHeight() / 2);
		//glm::vec2 Circle_Center(PosX1/* + Radius1*/, PosY1/* + Radius1*/);
		//glm::vec2 AABB_Center(PosX2/* + AABB_HalfExtents.x*/, PosY2/* + AABB_HalfExtents.y*/);

		//glm::vec2 difference = Circle_Center - AABB_Center;
		//glm::vec2 clamped = clamp(difference, -AABB_HalfExtents, AABB_HalfExtents);

		//glm::vec2 edgepoint = AABB_Center + clamped;

		//difference = edgepoint - Circle_Center;

		//if (glm::length(difference) > Radius1)
		//	return false;
		//else
		//{
		//	glm::vec2 CenterDistance(AABB_Center - Circle_Center);
		//	glm::vec2 clamped1 = glm::clamp(difference, -Radius1, Radius1);
		//
		//	glm::vec2 edgepoint1 = difference + clamped1;
		//	glm::vec2 separation = edgepoint1 - edgepoint;
		//	float penetration = std::min(separation.x, separation.y);
		//	glm::vec2 Collision_Normal = CheckDirection(separation);
		//
		//	if (separation.x > 0)// && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - separation.x / 100;
		//		PosX2 = PosX2 + separation.x / 100;
		//	}
		//	else //if(Separation.x < 0 && Separation.y == 0)
		//	{
		//		PosX1 = PosX1 - separation.x / 100;
		//		PosX2 = PosX2 + separation.x / 100;
		//	}
		//	if (separation.y > 0)// && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - separation.y / 100;
		//		PosY2 = PosY2 + separation.y / 100;
		//	}
		//	else //if (Separation.y < 0 && Separation.x == 0)
		//	{
		//		PosY1 = PosY1 - separation.y / 100;
		//		PosY2 = PosY2 + separation.y / 100;
		//	}
		//	return true;
		//}
	}

	bool longmarch::AABBCircleCollisions(Shape* AABB1, float PosX1, float PosY1,
		Shape* Circle2, float PosX2, float PosY2)
	{
		return longmarch::CircleAABBCollisions(Circle2, PosX2, PosY2, AABB1, PosX1, PosY1);
		////Collision Math And Logic
		//float Radius2;
		//Radius2 = Circle2->Radius;

		//Circle* pCircle2 = (Circle*)Circle2;
		//AABB* pAABB1 = (AABB*)AABB1;

		////Half Extents for reaching the edge point from center
		//glm::vec2 AABB_HalfExtents(pAABB1->GetWidth() / 2, pAABB1->GetHeight() / 2);
		//glm::vec2 Circle_Center(PosX2/* + Radius2*/, PosY2/* + Radius2*/);
		//glm::vec2 AABB_Center(PosX1/* + AABB_HalfExtents.x*/, PosY1/* + AABB_HalfExtents.y*/);

		//glm::vec2 difference = Circle_Center - AABB_Center;
		//glm::vec2 clamped = clamp(difference, -AABB_HalfExtents, AABB_HalfExtents);

		//glm::vec2 edgepoint = AABB_Center + clamped;

		//difference = edgepoint - Circle_Center;

		//if (glm::length(difference) <= Radius2)
		//	return true;

		//return false;
	}

	bool longmarch::CircleReflection(Shape* Circle1, float PosX1, float PosY1,
		Shape* Circle2, float PosX2, float PosY2)
	{
		throw NotImplementedException();
		//if (longmarch::CircleCollisions(Circle1, PosX1, PosY1, Circle2, PosX2, PosY2))
		//{
		//	glm::vec2 Circle1_Center(PosX1, PosY1);
		//	glm::vec2 Circle2_Center(PosX2, PosY2);
		//	glm::vec2 IntersectionPoint = (Circle1_Center * ((Circle*)Circle1)->Radius
		//		+ Circle2_Center * ((Circle*)Circle2)->Radius)
		//		/ (((Circle*)Circle1)->Radius + ((Circle*)Circle2)->Radius);

		//	glm::vec2 Circle1_Center_To_IntersectionPoint = Circle1_Center - IntersectionPoint;
		//	glm::normalize(Circle1_Center_To_IntersectionPoint);

		//	//Calculating the reflected velocity
		//   // glm::vec2 ReflectedVel = CurrVel - 2(CurrVel * Circle1_Center_To_IntersectionPoint)*Circle1_Center_To_IntersectionPoint;
		//}
		//return false;
	}

	bool longmarch::ResolveCollision(Shape* AABB1, float PosX1, float PosY1,
		Shape* AABB2, float PosX2, float PosY2)
	{
		if (longmarch::AABBCollisions(AABB1, PosX1, PosY1, AABB2, PosX2, PosY2) == true)
		{
			//Calculate Relative Velocity
			//glm::vec2 RelV = AABB2->mp_OwnerBody->GetVelocity() - AABB1->mp_OwnerBody->GetVelocity();

			//Calculate relative velocity in terms of the normal direction
			//float Vel_Along_Normal = glm::dot(RelV, normal);

			//Do not resolve if velocities are separating
			//if (Vel_Along_Normal > 0)
			//	return false;

			//Calculate Restitution
			//float e = std::min(AABB1->mp_OwnerBody->GetRestitution(), AABB2->mp_OwnerBody->GetRestitution());

			//Calculate impulse Scalar
			//float j = -(1 + e) * Vel_Along_Normal;
			//j /= 1 / AABB1->mp_OwnerBody->m_Mass + 1 / AABB2->mp_OwnerBody->m_Mass;

			//Apply Impulse
			//glm::vec2 Impulse = j * normal;
			//AABB1->mp_OwnerBody->m_VelX -= 1 / AABB1->mp_OwnerBody->m_Mass * Impulse;
			//AABB1->mp_OwnerBody->m_VelY -= 1 / AABB1->mp_OwnerBody->m_Mass * Impulse;

			//AABB2->mp_OwnerBody->m_VelX -= 1 / AABB2->mp_OwnerBody->m_Mass * Impulse;
			//AABB2->mp_OwnerBody->m_VelY -= 1 / AABB2->mp_OwnerBody->m_Mass * Impulse;

			//For calculating Raw Mass
			//float Mass_Sum = AABB1->mp_OwnerBody->m_Mass + AABB2->mp_OwnerBody->m_Mass;

			//float Ratio = AABB1->mp_OwnerBody->m_Mass / Mass_Sum;

			//AABB1->mp_OwnerBody->m_VelX -= Ratio * impulse;
			//AABB1->mp_OwnerBody->m_VelY -= Ratio * impulse;

			//Ratio = AABB2->mp_OwnerBody->m_Mass / Mass_Sum;

			//AABB2->mp_OwnerBody->m_VelX += Ratio * impulse;
			//AABB2->mp_OwnerBody->m_VelY += Ratio * impulse;

			return true;
		}

		return false;
	}

	bool longmarch::StaticCirclevsCircle(const std::shared_ptr<Circle>& circle1, const std::shared_ptr<Circle>& circle2)
	{
		// if squared distance between both circles is less than the squared combined radius of both circles, there is collision
		if (glm::length2(circle2->GetCenter() - circle1->GetCenter()) <
			glm::pow((circle1->GetRadius() + circle2->GetRadius()), 2))
			return true;

		return false;
	}

	bool longmarch::DynamicCirclevsCircle(const std::shared_ptr<Circle>& circle1, const Vec3f& Vel1,
		const std::shared_ptr<Circle>& circle2, const Vec3f& Vel2,
		float dt, Manifold& manifold)
	{
		// exit early if both are already overlapping
		if (StaticCirclevsCircle(circle1, circle2))
		{
			//manifold.m_A = dynamic_pointer_cast<Shape>(circle1);
			//manifold.m_B = dynamic_pointer_cast<Shape>(circle2);

			manifold.m_intersectTime = 0.0f;

			manifold.m_normal = glm::normalize(circle2->GetCenter() - circle1->GetCenter());
			manifold.m_contact.m_penetration = circle1->GetRadius() + circle2->GetRadius() - glm::length(circle2->GetCenter() - circle1->GetCenter());

			return true;
		}

		// treat circle 1 as a moving ray and circle 2 as a static sphere with enlarged radius
		Vec3f rayVel = Vel1 + Vel2;
		Vec3f rayDir = normalize(rayVel);
		float circRad = circle1->GetRadius() + circle2->GetRadius();
		float circRad2 = circRad * circRad;

		// test for collision between the ray and the circle

		Vec3f rayToSphere = circle2->GetCenter() - circle1->GetCenter();

		// pythagoras theorem, compute shortest perpendicular distance from ray's path to sphere origin
		float lengthProj = glm::dot(rayToSphere, rayDir);
		float dist2 = glm::dot(rayToSphere, rayToSphere) - lengthProj * lengthProj;

		if (dist2 > circRad2)
			return false;

		float lenToIntersect = sqrt(circRad2 - dist2);
		float intersect1 = lengthProj - lenToIntersect;
		float intersect2 = lengthProj + lenToIntersect;

		// make sure intersect1 is the shortest length
		if (intersect1 > intersect2)
		{
			std::swap(intersect1, intersect2);
		}

		// if smallest is negative, reject it
		if (intersect1 < 0)
		{
			intersect1 = intersect2;

			if (intersect1 < 0)
				return false;
		}

		// reject if length of first intersect > length traveled by ray in dt
		if (glm::length2(rayVel * dt) < intersect1 * intersect1)
			return false;

		// now we have intersect1, which is the length from ray position to intersection with sphere
		float timeToIntersect = intersect1 / (glm::length(rayVel) + glm::epsilon<float>());

		// fill in data in manifold
		//manifold.m_A = circle1;
		//manifold.m_B = circle2;

		manifold.m_intersectTime = timeToIntersect;

		manifold.m_normal = glm::normalize(circle1->GetCenter() - circle2->GetCenter() + rayVel * timeToIntersect);

		return true;
	}

	bool longmarch::StaticAABBvsAABB(const std::shared_ptr<AABB>& AABB1, const std::shared_ptr<AABB>& AABB2)
	{
		Vec3f min1 = AABB1->GetMin(), min2 = AABB2->GetMin();
		Vec3f max1 = AABB1->GetMax(), max2 = AABB2->GetMax();

		// if a separating axis is found, then there is no intersection
		for (int i = 0; i < 3; ++i)
		{
			if (max1[i] < min2[i] || min1[i] > max2[i])
				return false;
		}

		return true;
	}

	bool longmarch::DynamicAABBvsAABB(const std::shared_ptr<AABB>& AABB1, const Vec3f& Vel1,
		const std::shared_ptr<AABB>& AABB2, const Vec3f& Vel2,
		float dt, Manifold& manifold)
	{
		// exit early if both are already overlapping
		if (StaticAABBvsAABB(AABB1, AABB2))
		{
			//manifold.m_A = dynamic_pointer_cast<Shape>(AABB1);
			//manifold.m_B = dynamic_pointer_cast<Shape>(AABB2);

			// compute manifold
			Vec3f relativeVec = AABB2->GetCenter() - AABB1->GetCenter();
			int intersectAxis = 0;
			float penetrationDepth = std::numeric_limits<float>::max();

			Vec3f aabb1HalfExtents = (AABB1->GetMax() - AABB1->GetMin()) * 0.5f;
			Vec3f aabb2HalfExtents = (AABB2->GetMax() - AABB2->GetMin()) * 0.5f;
			Vec3f combinedHalfExtents = aabb1HalfExtents + aabb2HalfExtents;

			// find the axis of least penetration
			for (int i = 0; i < 3; ++i)
			{
				float axisPenetration = combinedHalfExtents[i] - fabsf(relativeVec[i]);

				if (axisPenetration > 0 && axisPenetration < penetrationDepth)
				{
					intersectAxis = i;
					penetrationDepth = axisPenetration;
				}
			}

			manifold.m_intersectTime = 0.0f;

			// put the value as negative to make it easier to calculate response velocity
			manifold.m_normal = Vec3f(0.0f, 0.0f, 0.0f);
			manifold.m_normal[intersectAxis] = -1.0f;

			manifold.m_contact.m_penetration = penetrationDepth;
			manifold.m_staticCollision = true;

			return true;
		}

		// epsilon added to prevent possible division by 0
		Vec3f relativeVel = (Vel2 - Vel1) + Vec3f(glm::epsilon<float>(), glm::epsilon<float>(), glm::epsilon<float>());

		f32 tFirst = 0.0f;
		f32 tLast = dt;

		Vec3f min1 = AABB1->GetMin(), min2 = AABB2->GetMin();
		Vec3f max1 = AABB1->GetMax(), max2 = AABB2->GetMax();

		f32 firstIntersect = dt;	// time of first intersection
		int intersectAxis = 0;		// axis of intersection
		float penetrationDepth = 0.0f;

		for (int i = 0; i < 3; ++i)
		{
			if (relativeVel[i] <= 0.0f)
			{
				if (max2[i] < min1[i])
					return false;

				if (max1[i] < min2[i])
					tFirst = std::max((max1[i] - min2[i]) / relativeVel[i], tFirst);

				if (max2[i] > min1[i])
					tLast = std::min((min1[i] - max2[i]) / relativeVel[i], tLast);
			}

			else if (relativeVel[i] > 0.0f)
			{
				if (min2[i] > max1[i])
					return false;

				if (max2[i] < min1[i])
					tFirst = std::max((min1[i] - max2[i]) / relativeVel[i], tFirst);

				if (max1[i] > min2[i])
					tLast = std::min((max1[i] - min2[i]) / relativeVel[i], tLast);
			}

			if (tFirst > tLast)
				return false;

			else if (tFirst < firstIntersect && tFirst > 0)
			{
				firstIntersect = tFirst;
				intersectAxis = i;
				penetrationDepth = relativeVel[i] * (tLast - tFirst);
			}
		}

		manifold.m_intersectTime = tFirst;

		// axis of earliest intersection

		// put the value as negative to make it easier to calculate response velocity
		manifold.m_normal = Vec3f(0.0f, 0.0f, 0.0f);
		manifold.m_normal[intersectAxis] = -1.0f;

		manifold.m_contact.m_penetration = penetrationDepth;
		manifold.m_staticCollision = false;

		//std::cout << "tfirst: " << tFirst << std::endl;
		//std::cout << "tLast: " << tLast << std::endl;

		//std::cout << std::endl;

		return true;
	}

	/*

	void ResolveAABBCollision(shared_ptr<RigidBody>& rb, Vec3f collisionNormal, float collisionTime)
	{
	}

	void ResolveCircleCollision(shared_ptr<RigidBody>& rb, Vec3f collisionNormal, float collisionTime)
	{
	}*/

	void ResolveStaticCollision(const Manifold& manifold)
	{
		Vec3f rb1AdjustVec, rb2AdjustVec;

		// cases where 1 of the RBs is static
		if (manifold.m_A->GetRBType() == RBType::staticBody)
		{
			// if for some reason both are static, exit just in case
			if (manifold.m_B->GetRBType() == RBType::staticBody)
				return;

			// rb2 needs to be adjusted, so push it out by the penetration value in the direction of the collision normal
			Vec3f collNormal = manifold.m_normal;

			if (glm::dot(manifold.m_A->GetWorldPosition() - manifold.m_B->GetWorldPosition(), collNormal) > 0)
				collNormal = -collNormal;

			manifold.m_B->SetWorldPosition(manifold.m_B->GetWorldPosition() + collNormal * manifold.m_contact.m_penetration);
		}

		else if (manifold.m_B->GetRBType() == RBType::staticBody)
		{
			// rb2 needs to be adjusted, so push it out by the penetration value in the direction of the collision normal
			Vec3f collNormal = manifold.m_normal;

			if (glm::dot(manifold.m_B->GetWorldPosition() - manifold.m_A->GetWorldPosition(), collNormal) > 0)
				collNormal = -collNormal;

			manifold.m_A->SetWorldPosition(manifold.m_A->GetWorldPosition() + collNormal * manifold.m_contact.m_penetration);
		}

		// case where both are dynamic, push them out evenly
		else
		{
			// rb2 needs to be adjusted, so push it out by the penetration value in the direction of the collision normal
			Vec3f collNormal = manifold.m_normal;

			if (glm::dot(manifold.m_A->GetWorldPosition() - manifold.m_B->GetWorldPosition(), collNormal) > 0)
				collNormal = -collNormal;

			manifold.m_B->SetWorldPosition(manifold.m_B->GetWorldPosition() + collNormal * manifold.m_contact.m_penetration * 0.5f);
			manifold.m_A->SetWorldPosition(manifold.m_A->GetWorldPosition() - collNormal * manifold.m_contact.m_penetration * 0.5f);
		}
	}

	void ResolveCollision(const Manifold& manifold, float dt, bool enableFriction)
	{
		
		// if it's a static collision, we should just separate the objects
		if (manifold.m_staticCollision)
		{
			ResolveStaticCollision(manifold);
		}
		else
		{
			// for now resolution for both AABB and Circle will be the same (subject to change)
			auto& rb1 = manifold.m_A;
			auto& rb2 = manifold.m_B;

			// for static bodies, treat them to have really high mass
		//float rb1Mass = rb1->GetRBType() == RBType::staticBody ? 1000.0f : rb1->GetMass();
		//float rb2Mass = rb2->GetRBType() == RBType::staticBody ? 1000.0f : rb2->GetMass();

		//Vec3f collNormal = manifold.m_normal;

			Vec3f rb2_1 = rb1->GetWorldPosition() - rb2->GetWorldPosition();
			Vec3f rb1_2 = rb2->GetWorldPosition() - rb1->GetWorldPosition();

			Vec3f rb1_vel = rb1->GetRBType() == RBType::staticBody ? Vec3f() : rb1->GetLinearVelocity();
			Vec3f rb2_vel = rb2->GetRBType() == RBType::staticBody ? Vec3f() : rb2->GetLinearVelocity();

			// advance rb1 and rb2 until time of impact
			rb1->SetWorldPosition(rb1->GetWorldPosition() + rb1_vel * manifold.m_intersectTime);
			rb2->SetWorldPosition(rb2->GetWorldPosition() + rb2_vel * manifold.m_intersectTime);

			// compute the resulting velocity for rb1
			//rb1->SetLinearVelocity(rb1_vel - 2.0f * rb2Mass / (rb1Mass + rb2Mass) * glm::dot(rb1_vel - rb2_vel, rb2_1) / glm::length2(rb2_1) * rb2_1);
			//rb2->SetLinearVelocity(rb2_vel - 2.0f * rb1Mass / (rb1Mass + rb2Mass) * glm::dot(rb2_vel - rb1_vel, rb1_2) / glm::length2(rb1_2) * rb1_2);

			//float rb1Speed = glm::length((rb1Mass - rb2Mass) / (rb1Mass + rb2Mass) * rb1_vel + 2.0f * rb2Mass / (rb1Mass + rb2Mass) * rb2_vel);
			//float rb2Speed = glm::length((rb2Mass - rb1Mass) / (rb1Mass + rb2Mass) * rb2_vel + 2.0f * rb1Mass / (rb1Mass + rb2Mass) * rb1_vel);

			//float rb1Speed = glm::length((rb1Mass * rb1_vel + rb2Mass * rb2_vel + rb2Mass * (rb2_vel - rb1_vel)) / (rb1Mass + rb2Mass));
			//float rb2Speed = glm::length((rb1Mass * rb1_vel + rb2Mass * rb2_vel - rb1Mass * (rb2_vel - rb1_vel)) / (rb1Mass + rb2Mass));

			// reflect the velocity about the collision normal and scale it by the new speed
			//rb1->SetLinearVelocity(glm::normalize(rb1_vel + 2.0f * rb1_vel * manifold.m_normal) * rb1Speed);
			//rb2->SetLinearVelocity(glm::normalize(rb2_vel + 2.0f * rb2_vel * manifold.m_normal) * rb2Speed);

			//rb1->SetLinearVelocity((rb1Mass - rb2Mass) / (rb1Mass + rb2Mass) * rb1_vel + 2.0f * rb2Mass / (rb1Mass + rb2Mass) * rb2_vel);
			//rb2->SetLinearVelocity((rb2Mass - rb1Mass) / (rb1Mass + rb2Mass) * rb2_vel + 2.0f * rb1Mass / (rb1Mass + rb2Mass) * rb1_vel);

			// compute impulse to be applied to each object
			Vec3f impulse = manifold.m_normal;
			impulse = glm::dot(rb1_2, manifold.m_normal) > 0 ? -impulse : impulse;

			float restitution = (rb1->GetRestitution() + rb2->GetRestitution()) * 0.5f;

			//impulse = fabsf(-(1.0f + restitution) * (rb2_vel - rb1_vel) / (rb1->GetInvMass() + rb2->GetInvMass()));
			Vec3f relativeVel = rb2_vel - rb1_vel;
			float impulseMagnitude = fabsf(glm::dot(-(1.0f + restitution) * (rb2_vel - rb1_vel), impulse));

			impulse = impulse * impulseMagnitude / (rb1->GetInvMass() + rb2->GetInvMass());

			if (rb1->GetRBType() == RBType::dynamicBody)
			{
				rb1->ApplyLinearImpulse(impulse);
				rb1->SetCollisionStatus(true, rb1->GetSolveTimeLeft() - manifold.m_intersectTime);

				// apply friction if enabled
				if (enableFriction)
				{
					// get the normal force
					float normalForce = rb1->GetMass() * glm::dot(-manifold.m_gravity, glm::normalize(impulse));

					Vec3f frictionVec = glm::cross(rb1_vel, impulse);

					if (glm::length2(frictionVec) > glm::epsilon<float>())
					{
						// get tangent vector to the surface, opposite to the direction rb1 is moving in
						frictionVec = glm::normalize(glm::cross(frictionVec, impulse));

						// get the friction limit (friction cannot exceed the movement it is opposing)
						float maxFriction = fabsf(glm::dot(relativeVel, frictionVec));
						float frictionVal = std::min(fabsf(normalForce * manifold.m_friction), maxFriction);
						
						frictionVec *= frictionVal;
						rb1->ApplyLinearForce(frictionVec);
					}
				}
			}

			if (rb2->GetRBType() == RBType::dynamicBody)
			{
				rb2->ApplyLinearImpulse(-impulse);
				rb2->SetCollisionStatus(true, rb2->GetSolveTimeLeft() - manifold.m_intersectTime);

				if (enableFriction)
				{
					// get the normal force
					float normalForce = rb2->GetMass() * glm::dot(-manifold.m_gravity, glm::normalize(-impulse));

					Vec3f frictionVec = glm::cross(rb2_vel, -impulse);

					if (glm::length2(frictionVec) > glm::epsilon<float>())
					{
						frictionVec = glm::normalize(glm::cross(frictionVec, -impulse));

						//frictionVec *= normalForce * manifold.m_friction;
						//rb2->ApplyLinearForce(frictionVec);

						float maxFriction = fabsf(glm::dot(relativeVel, frictionVec));
						float frictionVal = std::min(fabsf(normalForce * manifold.m_friction), maxFriction);

						frictionVec *= frictionVal;
						rb2->ApplyLinearImpulse(frictionVec);
					}
				}
			}

			//rb1->SetCollisionStatus(true, rb1->GetSolveTimeLeft() - manifold.m_intersectTime);
			//rb2->SetCollisionStatus(true, rb2->GetSolveTimeLeft() - manifold.m_intersectTime);
		}
	}

	bool DynamicShapevsShape(const std::shared_ptr<Shape>& shape1, const Vec3f& Vel1,
		const std::shared_ptr<Shape>& shape2, const Vec3f& Vel2,
		float dt, Manifold& manifold)
	{
		// apply the appropriate collision check based on the given shapes
		Shape::SHAPE_TYPE shape1Type = shape1->GetType();
		Shape::SHAPE_TYPE shape2Type = shape2->GetType();

		if (shape1Type == Shape::SHAPE_TYPE::AABB && shape2Type == Shape::SHAPE_TYPE::AABB)
		{
			return DynamicAABBvsAABB(dynamic_pointer_cast<AABB>(shape1), Vel1, dynamic_pointer_cast<AABB>(shape2), Vel2, dt, manifold);
		}

		return false;
	}

	bool CollisionsManager::CheckCollisionAndGenerateDetection(Shape* pShape1, float PosX1, float PosY1,
		Shape* pShape2, float PosX2, float PosY2)
	{
		return CollisionFunctions[(unsigned int)(pShape1->GetType())][(unsigned int)(pShape2->GetType())](pShape1, PosX1, PosY1, pShape2, PosX2, PosY2);
	}

	CollisionsManager::CollisionsManager()
	{
		CollisionFunctions[(unsigned int)Shape::SHAPE_TYPE::CIRCLE][(unsigned int)Shape::SHAPE_TYPE::CIRCLE] = CircleCollisions;
		CollisionFunctions[(unsigned int)Shape::SHAPE_TYPE::CIRCLE][(unsigned int)Shape::SHAPE_TYPE::AABB] = CircleAABBCollisions;
		CollisionFunctions[(unsigned int)Shape::SHAPE_TYPE::AABB][(unsigned int)Shape::SHAPE_TYPE::CIRCLE] = AABBCircleCollisions;
		CollisionFunctions[(unsigned int)Shape::SHAPE_TYPE::AABB][(unsigned int)Shape::SHAPE_TYPE::AABB] = AABBCollisions;
	}
}