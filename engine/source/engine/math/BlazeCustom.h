#pragma once
#include <blaze/Blaze.h>

namespace AAAAgames
{
	namespace BlazeCustom
	{
		using mat3f32 = blaze::StaticMatrix<float, 3UL, 3UL, blaze::columnMajor, blaze::aligned, blaze::padded>;
		using mat4f32 = blaze::StaticMatrix<float, 4UL, 4UL, blaze::columnMajor, blaze::aligned, blaze::padded>;
		using vec3f32 = blaze::StaticVector<float, 3UL, blaze::columnVector, blaze::aligned, blaze::padded>;
		using vec4f32 = blaze::StaticVector<float, 4UL, blaze::columnVector, blaze::aligned, blaze::padded>;
		using quatf32 = vec4f32;

		using mat3f64 = blaze::StaticMatrix<double, 3UL, 3UL, blaze::columnMajor, blaze::aligned, blaze::padded>;
		using mat4f64 = blaze::StaticMatrix<double, 4UL, 4UL, blaze::columnMajor, blaze::aligned, blaze::padded>;
		using vec3f64 = blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded>;
		using vec4f64 = blaze::StaticVector<double, 4UL, blaze::columnVector, blaze::aligned, blaze::padded>;
		using quatf64 = vec4f64;
	}
}