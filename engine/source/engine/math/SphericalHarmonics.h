#pragma once
#include "MathUtil.h"
#include "GridF32.h"
#include "engine/renderer/Image2D.h"

namespace AAAAgames
{
	class SphericalHarmonics
	{
	public:
		typedef ArrayF32 SHB;

		inline static SHB ConvCoeff = { SHB({ PI, PI2 / 3.0f, PI2 / 3.0f, PI2 / 3.0f, 
			PI / 4.0f, PI / 4.0f, PI / 4.0f, PI / 4.0f, PI / 4.0f}) };
		inline static SHB SHCoeff = { SHB({ 
			sqrtf(1 / (4 * PI)), 
			sqrtf(3 / (4 * PI)), 
			sqrtf(3 / (4 * PI)),
			sqrtf(3 / (4 * PI)),
			sqrtf(15 / (4 * PI)),
			sqrtf(15 / (4 * PI)),
			sqrtf(5 / (16 * PI)),
			sqrtf(15 / (4 * PI)),
			sqrtf(15 / (16 * PI))
			}) };
		static std::vector<Vec3f> Get_SH_From_RGBSHB(const SHB& RB, const SHB& GB, const SHB& BB)
		{
			std::vector<Vec3f> ret;
			ret.reserve(9);
			for (int i(0); i < 9; ++i)
			{
				ret.emplace_back(RB[i], GB[i], BB[i]);
			}
			return ret;
		}
		static SHB GetBase(const Vec3f& direction) {
			float x = direction[0];
			float y = direction[1];
			float z = direction[2];
			return SHB(
				{
				1.0f,
				y,
				z,
				x,
				x * y,
				y * z,
				(3.0f * z * z - 1.0f),
				x * z,
				(x * x - y * y)
				}
			);
		}
		/*
		Takes an image and calculates the Coefficients for the ambient light.
		Those coefficients contain the convolution with the cosine.
		*/
		static void SphericalHarmonicsOptimized(const std::shared_ptr<Image2D>& im, SHB& RB, SHB& GB, SHB& BB)
		{
			float normalizationFactor = 0.0f;
			float area = 0.0f;
			float hf = PI / im->GetHeight();
			float wf = (PI2) / im->GetWidth();
			SHB coefficients;
			SHB base;
			Vec3f dir;
			Vec3f color;
			for (uint32_t j = 0; j < im->GetHeight(); ++j)
			{
				float phi = hf * float(j+0.5);
				float  sinPhi = sin(phi) * hf * wf;

				for (uint32_t i = 0; i < im->GetWidth(); ++i)
				{
					float theta = wf * float(i+0.5);

					// Here, I am using the normal spherical coordinates formulas, however, in the shader the normal is used like normal.xzy changing the place of z and y to make sense of the data
					dir.x = cos(theta) * sin(phi);
					dir.y = sin(theta) * sin(phi);
					dir.z = cos(phi);
					base = GetBase(dir);
					color = im->GetPixel(i, j).xyz;

					base *= sinPhi;
					area += sinPhi;
					RB += (base * color[0]);
					GB += (base * color[1]);
					BB += (base * color[2]);
				}
			}
			normalizationFactor = (4 * PI / area); // * (1 / PI) 
			coefficients = ConvCoeff * SHCoeff * normalizationFactor;

			RB *= coefficients;
			GB *= coefficients;
			BB *= coefficients;

			PRINT("Generating SH for RBG in frequency spaces: ");
			PRINT("RB : " + Str(RB));
			PRINT("GB : " + Str(GB));
			PRINT("BB : " + Str(BB));
		}
		/*
		Applies the inverse transformation to the coefficients and save them in an irradiance map
		*/
		static void InverseSphericalHarmonics(std::shared_ptr<Image2D>& result, const SHB& RB, const SHB& GB, const SHB& BB)
		{
			float normalizationFactor = 0.0;
			float area = 0.0f;
			float hf = PI / result->GetHeight();
			float wf = (PI2) / result->GetWidth();
			SHB inv_coefficients;
			SHB base;
			Vec3f dir;
			Vec4f color(0,0,0,1);

			for (uint32_t j = 0; j < result->GetHeight(); ++j)
			{
				float phi = hf * float(j+0.5);
				float sinPhi = sin(phi) * hf * wf;
				for (uint32_t i = 0; i < result->GetWidth(); ++i)
				{
					area += sinPhi;
				}
			}
			normalizationFactor = 4 * PI / area;
			inv_coefficients = SHCoeff * (1.0f/normalizationFactor);

			for (uint32_t j = 0; j < result->GetHeight(); ++j)
			{
				float phi = hf * float(j+0.5);

				for (uint32_t i = 0; i < result->GetWidth(); ++i)
				{
					float theta = wf * float(i+0.5);

					dir.x = cos(theta) * sin(phi);
					dir.y = sin(theta) * sin(phi);
					dir.z = cos(phi);
					base = GetBase(dir);
					base *= inv_coefficients;

					color.x = MAX((float)base.dot(RB), 0.0f);
					color.y = MAX((float)base.dot(GB), 0.0f);
					color.z = MAX((float)base.dot(BB), 0.0f);
					result->WritePixel(i, j, color);		//transform to radiance to be used directly in my engine
				}
			}
		}
		static void Serialize(const SHB& RB, const SHB& GB, const SHB& BB)
		{
			PRINT("Serializing SH for RBG: ");
			PRINT("RB : " + Str(RB));
			PRINT("GB : " + Str(GB));
			PRINT("BB : " + Str(BB));
		}
	};
}