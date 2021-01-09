#pragma once

#include "engine/math/Geommath.h"
#include "engine/core/utility/TypeHelper.h"

namespace AAAAgames
{
	namespace pathfinding
	{
		struct SmoothingSetting
		{
			float mid_points_coarse_grain_stride{ 0.2f };
			float end_points_fine_grain_range{ 0.1f };
			float end_points_fine_grain_stride{ 0.05f };
			float angle_rad_tolerance{ 175.f * DEG2RAD };
			float delta_distance_tolerance{ 0.01f };
		};

		//! Genearte the arc length LUT from a give path
		A4GAMES_Vector<float> GetArcLengthLUT(const A4GAMES_Vector<Vec3f>& result);

		//! Applying the C1 Catmull-Rom interpolation onto a list of vectors (commonly used for sommothing path finding results)
		A4GAMES_Vector<Vec3f> CatmullRomSmoothing(const A4GAMES_Vector<Vec3f>& result, const SmoothingSetting& setting);

		//! Applying the C2 cubic B-spline interpolation onto a list of vectors (commonly used for flying camera)
		A4GAMES_Vector<Vec3f> CubicBSplineSmoothing(const A4GAMES_Vector<Vec3f>& result, const SmoothingSetting& setting);
	}
}