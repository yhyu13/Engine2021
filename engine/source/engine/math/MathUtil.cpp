#include "engine-precompiled-header.h"
#include "MathUtil.h"

uint32_t longmarch::LongMarch_NearestPowerOfTwo(uint32_t n)
{
	uint32_t v = n;

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++; // next power of 2

	uint32_t x = v >> 1; // previous power of 2

	return (v - n) > (n - x) ? x : v;
}

uint32_t longmarch::LongMarch_NextPowerOfTwo(uint32_t n)
{
	uint32_t v = n;

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++; // next power of 2

	return v;
}

float longmarch::LongMarch_Lerp(float a, float b, float ratio)
{
	float _ratio = MIN(MAX(ratio, 0.0f), 1.0f);
	return (1.0f - _ratio) * a + _ratio * b;
}

std::default_random_engine& replacement::detail::get_engine() noexcept
{
	// Seeding with 1 is silly, but required behavior
	static thread_local auto rndeng = std::default_random_engine(1);
	return rndeng;
}

std::uniform_int_distribution<int>& replacement::detail::get_distribution() noexcept
{
	static thread_local auto rnddst = std::uniform_int_distribution<int>{ 0, rand_max };
	return rnddst;
}

int replacement::rand() noexcept
{
	return detail::get_distribution()(detail::get_engine());
}

void replacement::srand(const unsigned seed) noexcept
{
	detail::get_engine().seed(seed);
	detail::get_distribution().reset();
}

void replacement::srand() noexcept
{
	std::random_device rnddev{};
	srand(rnddev());
}
