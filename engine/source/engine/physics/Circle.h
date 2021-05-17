#pragma once
#include "Shape.h"

namespace longmarch
{
	class CACHE_ALIGN16 Circle final : public Shape
	{
	public:
		NONCOPYABLE(Circle);
		Circle()
			:
			center(Vec3f(0)),
			radius(0),
			Shape(SHAPE_TYPE::CIRCLE)
		{
		}
		explicit Circle(const Vec3f& c, float r)
			:
			center(c),
			radius(r),
			Shape(SHAPE_TYPE::CIRCLE)
		{
		}
		virtual Vec3f GetCenter() override { throw NotImplementedException(); };
		virtual float GetRadius() override { throw NotImplementedException(); };
		virtual void SetModelTrAndUpdate(const Mat4& transform) override { throw NotImplementedException(); }
		virtual bool VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace)override { throw NotImplementedException(); }
		virtual bool DistanceTest(const Vec3f& center, float Near, float Far) override { throw NotImplementedException(); }
		virtual void RenderShape() override { throw NotImplementedException(); };

	public:
		Vec3f center;
		float radius;
	};
}

