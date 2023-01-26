#pragma once
#include "engine/math/Geommath.h"
#include "engine/EngineEssential.h"
#include "engine/ecs/Entity.h"

namespace longmarch
{
    class Shape
    {
    public:
        enum class SHAPE_TYPE
        {
            EMPTY = 0,
            CIRCLE,
            AABB,
            OOBB,
            NUM
        };

        NONCOPYABLE(Shape);
        Shape() = delete;

        explicit Shape(SHAPE_TYPE type)
            :
            m_type(type)
        {
        }

        virtual ~Shape() = default;
        virtual Vec3f GetCenter() = 0;
        virtual float GetRadius() = 0;
        //! Set the model transformation matrix
        virtual void SetModelTrAndUpdate(const Mat4& transform) = 0;
        //! Return true if VFC succeed
        virtual bool VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace) = 0;
        //! Return true if distance culling succeed
        virtual bool DistanceTest(const Vec3f& center, float Near, float Far) = 0;
        //! Implement this interface for the renderer to visualize this bounding volume
        virtual void RenderShape() = 0;
        virtual void GetBoundingBoxMinMax(Vec3f& Min, Vec3f& Max) const = 0;

        inline bool IsVFCulled()
        {
            return m_isCulled;
        }

        inline SHAPE_TYPE GetType()
        {
            return m_type;
        }

        inline Entity GetOwnerEntity()
        {
            return m_ownerEntity;
        }

        inline void SetOwnerEntity(const Entity& e)
        {
            m_ownerEntity = e;
        }

        //! (1) copy the same shape type (2) Convert one shape to another
        static void Copy(Shape* shape1, Shape* shape2);
    protected:
        Mat4 m_ObjectTr;
        Entity m_ownerEntity;
        SHAPE_TYPE m_type {SHAPE_TYPE::EMPTY};
        bool m_isCulled {false};
        bool m_isObjectTrInit {false};
    };
}
