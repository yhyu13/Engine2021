#include "engine-precompiled-header.h"
#include "MathUtil.h"

uint32_t AAAAgames::A4GAMES_NearestPowerOfTwo(uint32_t n)
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

uint32_t AAAAgames::A4GAMES_NextPowerOfTwo(uint32_t n)
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

float AAAAgames::A4GAMES_Lerp(float a, float b, float ratio)
{
	float _ratio = MIN(MAX(ratio, 0.0f), 1.0f);
	return (1.0f - _ratio) * a + _ratio * b;
}
