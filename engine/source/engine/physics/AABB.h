#pragma once
#include "Shape.h"
#include "engine/renderer/mesh/Mesh.h"

namespace longmarch
{
    class MS_ALIGN16 AABB : public Shape
    {
    public:
        NONCOPYABLE(AABB);
        AABB();
        explicit AABB(const Vec3f& min, const Vec3f& max);
        explicit AABB(const std::shared_ptr<MeshData>& mesh);
        explicit AABB(const LongMarch_Vector<std::shared_ptr<MeshData>>& meshs);
        void InitWithMeshData(const MeshData::VertexList& vertex_data, const MeshData::IndexList& index_data);

        const LongMarch_Vector<Vec3f> GetAllVertex() const;
        Vec3f GetMax() const;
        Vec3f GetMin() const;
        Vec3f GetOriginalMax() const;
        Vec3f GetOriginalMin() const;
        Vec3f GetDiag();

        void SetMax(const Vec3f& max);
        void SetMin(const Vec3f& min);
        void SetOriginalMax(const Vec3f& max);
        void SetOriginalMin(const Vec3f& min);
        void SetCenter(const Vec3f& center);
        float ComputeSurfaceArea() const;
        bool Overlaps(const AABB& aabb) const;
        void Merge(const AABB& aabb);
        void Merge(const AABB& aabb1, const AABB& aabb2);

        virtual float GetRadius() override;
        virtual Vec3f GetCenter() override;
        virtual void SetModelTrAndUpdate(const Mat4& transform) override;
        virtual bool VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace) override;
        virtual bool DistanceTest(const Vec3f& center, float Near, float Far) override;
        virtual void RenderShape() override;

    private:
        void Reset();
        void ResetOriginal();
        Vec3f GetOriginalDiag() const;
        Vec3f GetOriginalCenter() const;
        void Update(const Vec3f& point);
        void UpdateOriginal(const Vec3f& point);
        const LongMarch_Vector<Vec3f> GetAllVertexOriginal() const;

    private:
        Vec3f Min, Max;
        Vec3f o_min, o_max;
    };
}
