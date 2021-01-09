#pragma once
#include "MathUtil.h"
#include "GridF32.h"

namespace longmarch
{
	class DistributionMath
	{
	public:
		static DistributionMath* instance;

		static ArrayF32 Gaussian1D(int sampleNum, float mean, float std);
		static std::pair<ArrayF32, ArrayF32> Gaussian1DHalf(int sampleNum, float mean, float std);
		static std::pair<ArrayF32, ArrayF32> Gaussian1DHalfBilinear(int sampleNum, float mean, float std);
	};
}