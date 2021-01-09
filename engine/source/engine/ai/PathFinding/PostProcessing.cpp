#include "engine-precompiled-header.h"
#include "PostProcessing.h"

//! Genearte the arc length LUT from a give path
LongMarch_Vector<float> longmarch::pathfinding::GetArcLengthLUT(const LongMarch_Vector<Vec3f>& result)
{
	LongMarch_Vector<float> ret(result.size());
	ret[0] = 0;
	for (auto i(1u); i < result.size(); ++i)
	{
		ret[i] = ret[i - 1] + Geommath::Length(result[i] - result[i - 1]);
	}
	return ret;
}

//! Applying the Catmull-Rom interpolation onto a list of vectors
LongMarch_Vector<Vec3f> longmarch::pathfinding::CatmullRomSmoothing(const LongMarch_Vector<Vec3f>& result, const SmoothingSetting& setting)
{
	if (result.size() <= 1)
	{
		return result;
	}

	float mid_points_coarse_grain_stride = setting.mid_points_coarse_grain_stride;
	float end_points_fine_grain_range = setting.end_points_fine_grain_range;
	float end_points_fine_grain_stride = setting.end_points_fine_grain_stride;
	float angle_rad_tolerance = setting.angle_rad_tolerance;
	float delta_distance_tolerance = setting.delta_distance_tolerance;

	// Using Catmull-Rom Spline
	static auto CatmullRom = [](const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const float t) -> auto
	{
		const float factor1 = (-0.5f * t * t * t + t * t - 0.5f * t);
		const float factor2 = (1.5f * t * t * t - 2.5f * t * t + 1.0f);
		const float factor3 = (-1.5f * t * t * t + 2.0f * t * t + 0.5f * t);
		const float factor4 = (0.5f * t * t * t - 0.5f * t * t);
		return factor1 * p0 + factor2 * p1 + factor3 * p2 + factor4 * p3;
	};

	//! Adaptive subdivide the parametric curve based on cos angle of midpoint
	std::function<void(LongMarch_Vector<Vec3f>&, const Vec3f&, const Vec3f&, const Vec3f&, const Vec3f&, const float, const float)> RecursiveAdpativeStep = [&](LongMarch_Vector<Vec3f>& result, const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const float t0, const float t1)
	{
		auto t_mid = (t0 + t1) * 0.5f;
		auto pt0 = CatmullRom(p0, p1, p2, p3, t0);
		auto pt1 = CatmullRom(p0, p1, p2, p3, t1);
		auto pt_mid = CatmullRom(p0, p1, p2, p3, t_mid);
		auto A = Geommath::Length(pt_mid - pt0);
		auto B = Geommath::Length(pt_mid - pt1);
		auto C = Geommath::Length(pt1 - pt0);
		auto cos = (A * A + B * B - C * C) / (2.f * A * B);
		if (!isnan(cos) && !isinf(cos))
		{
			if (cos > cosf(angle_rad_tolerance) || (std::abs(A + B - C) > delta_distance_tolerance))
			{
				RecursiveAdpativeStep(result, p0, p1, p2, p3, t0, t_mid);
				result.emplace_back(pt_mid);
				RecursiveAdpativeStep(result, p0, p1, p2, p3, t_mid, t1);
			}
		}
	};

	// Copy result and insert copies of the firs and last element at the begin and the end
	auto result_ = result;
	result_.insert(result_.begin(), *(result_.begin()));
	result_.insert(result_.end(), *(result_.end() - 1));

	LongMarch_Vector<Vec3f> ret;
	ret.reserve(result_.size() * int(1.0f / end_points_fine_grain_stride));

	// rubber banding stops if we have reach the iterator just before the first copy of the target
	auto head_it = result_.begin();
	while (head_it != (result_.end() - 3))
	{
		float step = 0.0f;
		auto& p0 = *(head_it);
		auto& p1 = *(head_it + 1);
		auto& p2 = *(head_it + 2);
		auto& p3 = *(head_it + 3);

		// Catmull-Rom would generate interpolated points start at the second control point, so we add it frist
		ret.emplace_back(p1);

		// Fine grain at starting points
		while ((step += end_points_fine_grain_stride) <= end_points_fine_grain_range)
		{
			RecursiveAdpativeStep(ret, p0, p1, p2, p3, step - end_points_fine_grain_stride, step);
			ret.emplace_back(CatmullRom(p0, p1, p2, p3, step));
		}
		// Coarse grain mid points
		if (step != end_points_fine_grain_range)
		{
			step = end_points_fine_grain_range;
		}
		while ((step += mid_points_coarse_grain_stride) <= (1.0f - end_points_fine_grain_range))
		{
			RecursiveAdpativeStep(ret, p0, p1, p2, p3, step - mid_points_coarse_grain_stride, step);
			ret.emplace_back(CatmullRom(p0, p1, p2, p3, step));
		}
		// Fine grain at ending points
		if (step != 1.0f - end_points_fine_grain_range)
		{
			step = 1.0f - end_points_fine_grain_range;
		}
		while ((step += end_points_fine_grain_stride) <= 1.0f)
		{
			RecursiveAdpativeStep(ret, p0, p1, p2, p3, step - end_points_fine_grain_stride, step);
			if (step < 1.0f) //!< Do not add end point
			{
				ret.emplace_back(CatmullRom(p0, p1, p2, p3, step));
			}
		}
		++head_it;
	}
	// Still need to add the last element
	ret.emplace_back(*(result_.end() - 1));
	return ret;
}

LongMarch_Vector<Vec3f> longmarch::pathfinding::CubicBSplineSmoothing(const LongMarch_Vector<Vec3f>& result, const SmoothingSetting& setting)
{
	if (result.size() <= 1)
	{
		return result;
	}

	float mid_points_coarse_grain_stride = setting.mid_points_coarse_grain_stride;
	float end_points_fine_grain_range = setting.end_points_fine_grain_range;
	float end_points_fine_grain_stride = setting.end_points_fine_grain_stride;
	float angle_rad_tolerance = setting.angle_rad_tolerance;
	float delta_distance_tolerance = setting.delta_distance_tolerance;

	// Using Cubic-B Spline with control point, which is equivalent to cubic Bezier curve with control point
	static auto Bezier = [](const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const float t) -> auto
	{
		const float factor1 = (-1.f * t * t * t + 3.f * t * t - 3.f * t + 1.f);
		const float factor2 = (3.f * t * t * t - 6.f * t * t + 3.f * t);
		const float factor3 = (-3.f * t * t * t + 3.f * t * t);
		const float factor4 = (1.f * t * t * t);
		return factor1 * p0 + factor2 * p1 + factor3 * p2 + factor4 * p3;
	};
	static auto GetCubicBSplineDerivative = [](const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3) -> auto
	{
		static Mat4 NaturalCubicBSplineMatrix = Mat4(2.f, 1.f, 0.f, 0.f,
			1.f, 4.f, 1.f, 0.f,
			0.f, 1.f, 4.f, 1.f,
			0.f, 0.f, 1.f, 2.f);
		static auto NaturalCubicBSplineMatrix_Inv = Geommath::SmartInverse(NaturalCubicBSplineMatrix);

		LongMarch_Vector<Vec3f> ret(4);
		for (int i = 0; i < 3; ++i)
		{
			Vec4f E(3.f * (p1[i] - p0[i]), 3.f * (p2[i] - p0[i]), 3.f * (p3[i] - p1[i]), 3.f * (p3[i] - p2[i]));
			Vec4f D = NaturalCubicBSplineMatrix_Inv * E;
			for (int j = 0; j < 4; ++j)
			{
				ret[j][i] = D[j];
			}
		}
		return ret;
	};

	//! Adaptive subdivide the parametric curve based on cos angle of midpoint
	std::function<void(LongMarch_Vector<Vec3f>&, const Vec3f&, const Vec3f&, const Vec3f&, const Vec3f&, const float, const float)> RecursiveAdpativeStep = [&](LongMarch_Vector<Vec3f>& result, const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const float t0, const float t1)
	{
		auto t_mid = (t0 + t1) * 0.5f;
		auto pt0 = Bezier(p0, p1, p2, p3, t0);
		auto pt1 = Bezier(p0, p1, p2, p3, t1);
		auto pt_mid = Bezier(p0, p1, p2, p3, t_mid);
		auto A = Geommath::Length(pt_mid - pt0);
		auto B = Geommath::Length(pt_mid - pt1);
		auto C = Geommath::Length(pt1 - pt0);
		auto cos = (A * A + B * B - C * C) / (2.f * A * B);
		if (!isnan(cos) && !isinf(cos))
		{
			if (cos > cosf(angle_rad_tolerance) || (std::abs(A + B - C) > delta_distance_tolerance))
			{
				RecursiveAdpativeStep(result, p0, p1, p2, p3, t0, t_mid);
				result.emplace_back(pt_mid);
				RecursiveAdpativeStep(result, p0, p1, p2, p3, t_mid, t1);
			}
		}
	};

	// Copy result and insert copies of the firs and last element at the begin and the end
	auto result_ = result;
	result_.insert(result_.begin(), *(result_.begin()));
	result_.insert(result_.end(), *(result_.end() - 1));

	LongMarch_Vector<Vec3f> ret;
	ret.reserve(result_.size() * int(1.0f / end_points_fine_grain_stride));

	// rubber banding stops if we have reach the iterator just before the first copy of the target
	auto head_it = result_.begin();
	while (head_it != (result_.end() - 3))
	{
		float step = 0.0f;
		auto& p0 = *(head_it);
		auto& p1 = *(head_it + 1);
		auto& p2 = *(head_it + 2);
		auto& p3 = *(head_it + 3);

		auto Ds = GetCubicBSplineDerivative(p0, p1, p2, p3);
		auto& D0 = Ds[0];
		auto& D1 = Ds[1];
		auto& D2 = Ds[2];
		auto& D3 = Ds[3];

		auto C0 = p1;
		auto C1 = p1 + 1.0f / 3.0f * D1;
		auto C2 = p2 - 1.0f / 3.0f * D2;
		auto C3 = p2;

		// Cubic B-spline would generate interpolated points start at the second control point, so we add it frist
		ret.emplace_back(p1);

		// Fine grain at starting points
		while ((step += end_points_fine_grain_stride) <= end_points_fine_grain_range)
		{
			RecursiveAdpativeStep(ret, C0, C1, C2, C3, step - end_points_fine_grain_stride, step);
			ret.emplace_back(Bezier(C0, C1, C2, C3, step));
		}
		// Coarse grain mid points
		if (step != end_points_fine_grain_range)
		{
			step = end_points_fine_grain_range;
		}
		while ((step += mid_points_coarse_grain_stride) <= (1.0f - end_points_fine_grain_range))
		{
			RecursiveAdpativeStep(ret, C0, C1, C2, C3, step - mid_points_coarse_grain_stride, step);
			ret.emplace_back(Bezier(C0, C1, C2, C3, step));
		}
		// Fine grain at ending points
		if (step != 1.0f - end_points_fine_grain_range)
		{
			step = 1.0f - end_points_fine_grain_range;
		}
		while ((step += end_points_fine_grain_stride) <= 1.0f)
		{
			RecursiveAdpativeStep(ret, C0, C1, C2, C3, step - end_points_fine_grain_stride, step);
			if (step < 1.0f) //!< Do not add end point
			{
				ret.emplace_back(Bezier(C0, C1, C2, C3, step));
			}
		}
		++head_it;
	}
	// Still need to add the last element
	ret.emplace_back(*(result_.end() - 1));
	return ret;
}