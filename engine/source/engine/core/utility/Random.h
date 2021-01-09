#pragma once
#include <random>

namespace longmarch
{
	class Random
	{
	public:
		inline static void Init()
		{
			s_RandomEngine.seed(std::random_device()());
		}
		inline static float Float() // Generate [-1,1] uniformly
		{
			return (float)s_Distribution(s_RandomEngine) / (float)(std::numeric_limits<uint32_t>::max)();
		}
	public:
		inline static std::mt19937 s_RandomEngine;
		inline static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};
}
