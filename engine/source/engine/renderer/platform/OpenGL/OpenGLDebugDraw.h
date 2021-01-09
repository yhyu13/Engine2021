#pragma once
#include "engine/math/Geommath.h"
#include "OpenGLUtil.h"

namespace AAAAgames {

	class OpenGLDebugDraw
	{
	public:
		static void Init();
		static void End();

		static void DrawLine(const glm::vec3& fromPosition, const glm::vec3& toPosition, glm::vec3 color, float lineWidth = 1.0f);		
		static void DrawCircle(const glm::vec3& centerPosition, float radius, glm::vec3 color);
		static void DrawAABB(const glm::vec3& position, const glm::vec2& size, float rotation, glm::vec3& color);
	};
}


