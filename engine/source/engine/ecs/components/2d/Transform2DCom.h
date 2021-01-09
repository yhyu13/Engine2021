#pragma once

#include "engine/ecs/BaseComponent.h"
#include <glm/glm.hpp>

using namespace glm;

namespace longmarch
{
	struct CACHE_ALIGN32 Transform2DCom final : BaseComponent<Transform2DCom> {
		Transform2DCom()
		{
			pos = vec3(0);
			rotation = 0;
			velocity = vec3(0);
			rotational_velocity = 0;
		}
		explicit Transform2DCom(float x, float y, float z)
		{
			pos = vec3(x,y,z);
			rotation = 0;
			velocity = vec3(0);
			rotational_velocity = 0;
		}
		explicit Transform2DCom(const vec3& v, float r)
		{
			pos = v;
			rotation = r;
			velocity = vec3(0);
			rotational_velocity = 0;
		}
		void AddPos(const vec2& v)
		{
			pos.x += v.x;
			pos.y += v.y;
		}
		void SetPos(const vec2& v)
		{
			pos.x = v.x;
			pos.y = v.y;
		}
		void SetPos3D(const vec3& v)
		{
			pos.x = v.x;
			pos.y = v.y;
			pos.z = v.z;
		}
		void AddVelocity(const vec2& v)
		{
			velocity.x += v.x;
			velocity.y += v.y;
		}
		void SetVelocity(const vec2& v)
		{
			velocity.x = v.x;
			velocity.y = v.y;
		}
		const vec3& GetPos3D() const
		{
			return pos;
		}
		const vec2 GetPos() const
		{
			return vec2(pos.x, pos.y);
		}
		const vec2 GetVelocity() const
		{
			return vec2(velocity.x, velocity.y);
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
		vec3 pos;
		vec3 velocity;
		float rotation;
		float rotational_velocity;
	};
}