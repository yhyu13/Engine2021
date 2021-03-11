#pragma once
#include "MathUtil.h"
#include "GridF32.h"

namespace longmarch
{
	class DistributionMath
	{
	public:
		static DistributionMath* instance;

		//! sampleNum = full length sample number. sampleNum is set to the next odd number if it is even
		static ArrayF32 Gaussian1D(int sampleNum, float mean, float std); 

		//! sampleNum = full length sample number, not a half. sampleNum is set to the next odd number if it is even
		static std::pair<ArrayF32, ArrayF32> Gaussian1DHalf(int sampleNum, float mean, float std); 

		//! sampleNum = full length sample number, not a half. sampleNum is set to the next odd number if it is even
		static std::pair<ArrayF32, ArrayF32> Gaussian1DHalfBilinear(int sampleNum, float mean, float std); 
	};
}