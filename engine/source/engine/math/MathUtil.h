#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <random>

#ifndef PI
#define PI 3.14159265359f
#endif // !PI

#ifndef PI2
#define PI2 6.28318530718f
#endif // !PI

#ifndef PI_INVERSE
#define PI_INVERSE 0.31830988618f
#endif // !PI

#ifndef PI2_INVERSE
#define PI2_INVERSE 0.15915494309f
#endif // !PI

#ifndef DEG2RAD
#define DEG2RAD 0.01745329251f
#endif // !PI

#ifndef RAD2DEG
#define RAD2DEG 57.2957795131f
#endif // !PI
//
#define RAND_SEED_SET_TIME {std::srand((unsigned int)std::time(NULL));};
#define RAND_I(LO, HI) int(LO) + int(rand()) / (int(RAND_MAX / (int(HI) - int(LO))))
#define RAND_F(LO, HI) float(LO) + float(rand()) / (float(RAND_MAX / (float(HI) - float(LO))))
#define IN_RANGE(a, x, y) (a >= x && a <= y)

#define MAX(x, y) ((x)>(y))?(x):(y)
#define MIN(x, y) ((x)<(y))?(x):(y)

/*
	Z_ORDER(0) = 0.00,Z_ORDER(1) = 0.001,Z_ORDER(3) = 0.003,
*/
#define Z_ORDER(x) static_cast<float>(x)*0.0001f

namespace longmarch
{
	uint32_t LongMarch_NearestPowerOfTwo(uint32_t n);
	uint32_t LongMarch_NextPowerOfTwo(uint32_t n);
	float LongMarch_Lerp(float a, float b, float ratio);
}

#define NEAREST_POW2(x) LongMarch_NearestPowerOfTwo(x)
#define NEXT_POW2(x) LongMarch_NextPowerOfTwo(x)
