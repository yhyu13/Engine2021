#include "engine-precompiled-header.h"
#include "DistributionMath.h"

ArrayF32 longmarch::DistributionMath::Gaussian1D(int sampleNum, float mean, float std)
{
	float totalSum = 0;
	ArrayF32 weightValueVec(sampleNum);
	int half = (sampleNum / 2);
	float inv_std = 1.0f / std;
	float inv_std_pi = inv_std / sqrtf(2 * PI);
	for (int i = 0; i < sampleNum; ++i)
	{
		float i_val = i - half;
		float power_val = (-0.5f * powf((i_val - mean) * inv_std, 2));
		float val = inv_std_pi * powf(expf(1), power_val);
		totalSum += val;
		weightValueVec[i] = val;
	}
	float inv_sum = 1.0f / totalSum;
	for (int i = 0; i < sampleNum; ++i)
	{
		weightValueVec[i] *= inv_sum;
	}
	return weightValueVec;
}

std::pair<ArrayF32, ArrayF32> longmarch::DistributionMath::Gaussian1DHalf(int sampleNum, float mean, float std)
{
	float totalSum = 0;
	int half = (sampleNum / 2);
	int size = half + 1;
	ArrayF32 weightValueVec(size);
	ArrayF32 offsets(size);
	float inv_std = 1.0f / std;
	float inv_std_pi = inv_std / sqrtf(2 * PI);
	for (int i = 0; i < size; ++i)
	{
		float i_val = i;
		float power_val = (-0.5f * powf((i_val - mean) * inv_std, 2));
		float val = inv_std_pi * powf(expf(1), power_val);
		totalSum += val;
		weightValueVec[i] = val;
		offsets[i] = i;
	}
	float inv_sum = 1.0f / (2.0f*totalSum - weightValueVec[0]);
	for (int i = 0; i < size; ++i)
	{
		weightValueVec[i] *= inv_sum;
	}
	return std::pair<ArrayF32, ArrayF32>(offsets, weightValueVec);
}

std::pair<ArrayF32, ArrayF32> longmarch::DistributionMath::Gaussian1DHalfBilinear(int sampleNum, float mean, float std)
{
	auto [offsets, weights] = Gaussian1DHalf(sampleNum, mean, std);
	int new_size = (weights.X()-1) / 2 + 1;
	if (new_size <= 1)
	{
		return std::pair<ArrayF32, ArrayF32>(offsets, weights);
	}
	ArrayF32 new_weights(new_size);
	ArrayF32 new_offsets(new_size);
	float sum = 1.0f;

	new_weights[0] = weights[0] / sum;
	new_offsets[0] = 0;

	for (int i = 1; i < new_size; i++)
	{
		int index = (i - 1) * 2 + 1;
		float weight = weights[index] + weights[index + 1];
		new_offsets[i] = ((weights[index] * index + weights[index + 1] * (index + 1)) / weight);
		new_weights[i] = (weight / sum);
	}

	return std::pair<ArrayF32, ArrayF32>(new_offsets, new_weights);
}
