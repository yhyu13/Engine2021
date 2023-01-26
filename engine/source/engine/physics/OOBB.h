#pragma once
#include "Shape.h"
#include "engine/renderer/mesh/Mesh.h"

namespace longmarch
{
    class MS_ALIGN16 OOBB final : public Shape
    {
    public:
        NONCOPYABLE(OOBB);
        OOBB();
        explicit OOBB(const Vec3f& min, const Vec3f& max);
        explicit OOBB(const std::shared_ptr<MeshData>& mesh);
        explicit OOBB(const LongMarch_Vector<std::shared_ptr<MeshData>>& meshs);
        void InitWithMeshData(const MeshData::VertexList& vertex_data, const MeshData::IndexList& index_data);
        const LongMarch_Vector<Vec3f> GetAllVertex() const;
        Vec3f GetDiag();
        virtual float GetRadius() override;
        virtual Vec3f GetCenter() override;
        virtual void SetModelTrAndUpdate(const Mat4& transform) override;
        virtual bool VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace) override;
        virtual bool DistanceTest(const Vec3f& center, float Near, float Far) override;
        virtual void RenderShape() override;
        virtual void GetBoundingBoxMinMax(Vec3f& Min, Vec3f& Max) const override;

        virtual const Vec3f& GetMin() const;
        virtual const Vec3f& GetMax() const;
    private:
        void Reset();
        void ResetOriginal();
        Vec3f GetOriginalDiag() const;
        Vec3f GetOriginalCenter() const;
        void Update(const Vec3f& point);
        void UpdateOriginal(const Vec3f& point);
        const LongMarch_Vector<Vec3f> GetAllVertexOriginal() const;
        
    private:
        Vec3f Min, Max; // These two variables are not the same as for AABB, they are here just to get diag and center.
        Vec3f o_min, o_max;
    };
}
