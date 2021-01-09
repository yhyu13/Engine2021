#pragma once
#include "engine/math/GridF32.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	/*
	Get LU decomposition for a squared matrix of size NxN
	*/
	GridF32 GetLU(const GridF32& matrix, int n);
	/*
	Using NxN LU decomposed matrix to solve linear equation (matrix passed in is already a LU matrix)
	*/
	ArrayF32 SolveLU(const GridF32& lu, const ArrayF32& b, int n);
	/*
	solve linear equation using LU decomposition for a squared matrix of size NxN
	*/
	ArrayF32 SolveUsingLU(const GridF32& matrix, const ArrayF32& b, int n);

	/*
	Get the coefficients of a paramatric cubic spline interporlated curve
	*/
	std::vector<Vec2f> GetCubicSplineCoeff(const std::vector<Vec2f>& pts);
	/*
	Get point on the paramatric cubic spline
	*/
	Vec2f CubicSplineInterpolate(float t, const std::vector<Vec2f>& coeff);
	/*
	Get the cubic spline interporlated curve that is represented by points.
	Return an empty vector is the input vector has size smaller than 2.
	 (argument alpha represent the step size for the paramerteric value t which is used internally,
	 the smaller the smoother the curve, default is 1.f)
	*/
	std::vector<Vec2f> GetCubicSpline(const std::vector<Vec2f>& pts, float alpha = 1.f);
}