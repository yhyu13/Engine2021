#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	struct CACHE_ALIGN Transform2DCom final : public BaseComponent<Transform2DCom> 
	{
		Transform2DCom()
		{
			pos = Vec3f(0);
			rotation = 0;
			velocity = Vec3f(0);
			rotational_velocity = 0;
		}
		explicit Transform2DCom(float x, float y, float z)
		{
			pos = Vec3f(x,y,z);
			rotation = 0;
			velocity = Vec3f(0);
			rotational_velocity = 0;
		}
		explicit Transform2DCom(const Vec3f& v, float r)
		{
			pos = v;
			rotation = r;
			velocity = Vec3f(0);
			rotational_velocity = 0;
		}
		void AddPos(const Vec2f& v)
		{
			pos.x += v.x;
			pos.y += v.y;
		}
		void SetPos(const Vec2f& v)
		{
			pos.x = v.x;
			pos.y = v.y;
		}
		void SetPos3D(const Vec3f& v)
		{
			pos.x = v.x;
			pos.y = v.y;
			pos.z = v.z;
		}
		void AddVelocity(const Vec2f& v)
		{
			velocity.x += v.x;
			velocity.y += v.y;
		}
		void SetVelocity(const Vec2f& v)
		{
			velocity.x = v.x;
			velocity.y = v.y;
		}
		const Vec3f& GetPos3D() const
		{
			return pos;
		}
		const Vec2f GetPos() const
		{
			return Vec2f(pos.x, pos.y);
		}
		const Vec2f GetVelocity() const
		{
			return Vec2f(velocity.x, velocity.y);
		}
		void SetRotation(float r)
		{
			rotation = r;
		}
		float GetRotation()
		{
			return rotation;
		}

	private:
		Vec3f pos;
		Vec3f velocity;
		float rotation;
		float rotational_velocity;
	};
}
