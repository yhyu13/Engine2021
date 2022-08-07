#pragma once
#include "engine/math/Geommath.h"
#include "OpenGLUtil.h"

namespace longmarch {

	class OpenGLDebugDraw
	{
	public:
		static void Init();
		static void End();

		static void DrawLine(const Vec3f& fromPosition, const Vec3f& toPosition, Vec3f color, float lineWidth = 1.0f);		
		static void DrawCircle(const Vec3f& centerPosition, float radius, Vec3f color);
		static void DrawAABB(const Vec3f& position, const glm::vec2& size, float rotation, Vec3f& color);
	};
}


