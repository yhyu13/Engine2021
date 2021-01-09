#pragma once

#include <stdlib.h>
#include <cstdint>
#include "engine/core/utility/TypeHelper.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	namespace pathfinding
	{
		enum class Heuristic : uint8_t
		{
			OCTILE,
			CHEBYSHEV,
			MANHATTAN,
			EUCLIDEAN,
		};

		enum class PathResult : uint8_t
		{
			IDLE,
			PROCESSING,
			COMPLETE,
			IMPOSSIBLE
		};

		enum class Method : uint8_t
		{
			ASTAR,
			ROY_FLOYD_WARSHALL
		};

		using WaypointList = LongMarch_Vector<Vec3f>;

		struct PathRequest
		{
			Vec3f start;
			Vec3f goal;
			WaypointList path;

			struct Settings
			{
				Method method;
				Heuristic heuristic;
				float weight;
				bool smoothing;
				bool rubberBanding;
				bool singleStep;
				bool debugColoring;
			} settings;

			bool newRequest;
		};
	}
}