#include "engine-precompiled-header.h"
#include "OOBB.h"
#include "engine/renderer/Renderer3D.h"

longmarch::OOBB::OOBB()
    :
    Shape(SHAPE_TYPE::OOBB)
{
    ResetOriginal();
    Reset();
}

longmarch::OOBB::OOBB(const Vec3f& min, const Vec3f& max)
    :
    Shape(SHAPE_TYPE::OOBB)
{
    o_min = Min = min;
    o_max = Max = max;
}

longmarch::OOBB::OOBB(const std::shared_ptr<MeshData>& mesh)
    :
    Shape(SHAPE_TYPE::OOBB)
{
    ResetOriginal();
    InitWithMeshData(mesh->vertices, mesh->indices);
}

longmarch::OOBB::OOBB(const LongMarch_Vector<std::shared_ptr<MeshData>>& meshs)
    :
    Shape(SHAPE_TYPE::OOBB)
{
    ResetOriginal();
    for (const auto& mesh : meshs)
    {
        InitWithMeshData(mesh->vertices, mesh->indices);
    }
}

void longmarch::OOBB::InitWithMeshData(const MeshData::VertexList& vertex_data, const MeshData::IndexList& index_data)
{
    if (vertex_data.empty())
    {
        throw EngineException(
            _CRT_WIDE(__FILE__), __LINE__,
            L"Mesh data does not exist! Either the mesh data has been destroied after sending to GPU or the mesh data has not be loaded yet!");
    }
    // Check if submesh is indexed
    if (!index_data.empty())
    {
        // Update bounding box for each indexed vertex
        for (const auto& tri : index_data)
        {
#if MESH_VERTEX_DATA_FORMAT == 4
            const auto& pnt1 = Geommath::UnPackVec4ToHVec4(vertex_data[tri[0]].pnt);
            const auto& pnt2 = Geommath::UnPackVec4ToHVec4(vertex_data[tri[1]].pnt);
            const auto& pnt3 = Geommath::UnPackVec4ToHVec4(vertex_data[tri[2]].pnt);
            UpdateOriginal(pnt1 * pnt1.w);
            UpdateOriginal(pnt2 * pnt2.w);
            UpdateOriginal(pnt3 * pnt3.w);
#else
			UpdateOriginal(vertex_data[tri[0]].pnt);
			UpdateOriginal(vertex_data[tri[1]].pnt);
			UpdateOriginal(vertex_data[tri[2]].pnt);
#endif
        }
    }
    else
    {
        // Update bounding box for each vertex
        for (const auto& vertex3d : vertex_data)
        {
#if MESH_VERTEX_DATA_FORMAT == 4
            const auto& pnt1 = Geommath::UnPackVec4ToHVec4(vertex3d.pnt);
            UpdateOriginal(pnt1 * pnt1.w);
#else
			UpdateOriginal(vertex3d.pnt);
#endif
        }
    }
}

void longmarch::OOBB::ResetOriginal()
{
    o_min = Vec3f((std::numeric_limits<float>::max)());
    o_max = Vec3f((std::numeric_limits<float>::lowest)());
}

void longmarch::OOBB::UpdateOriginal(const Vec3f& point)
{
    o_min = (glm::min)(o_min, point);
    o_max = (glm::max)(o_max, point);
}

const LongMarch_Vector<Vec3f> longmarch::OOBB::GetAllVertex() const
{
    LongMarch_Vector<Vec3f> ret = GetAllVertexOriginal();
    for (auto& v : ret)
    {
        v = Geommath::ToVec3(m_ObjectTr * Geommath::ToVec4(v));
    }
    return ret;
}

const LongMarch_Vector<Vec3f> longmarch::OOBB::GetAllVertexOriginal() const
{
    LongMarch_Vector<Vec3f> ret(8);
    {
        const Vec3f &_min = o_min, _max = o_max;
        Vec4f abc = Vec4f(_max - _min, 0);
        ret[0] = _min;
        ret[1] = _min + abc.xww;
        ret[2] = _min + abc.xyw;
        ret[3] = _min + abc.wyw;

        ret[4] = _min + abc.wwz;
        ret[5] = _min + abc.xwz;
        ret[6] = _min + abc.xyz;
        ret[7] = _min + abc.wyz;
    }
    return ret;
}

Vec3f longmarch::OOBB::GetDiag()
{
    return Max - Min;
}

inline float longmarch::OOBB::GetRadius()
{
    return glm::length(GetDiag() * 0.5f);
}

Vec3f longmarch::OOBB::GetCenter()
{
    return (Max + Min) * 0.5f;
}

void longmarch::OOBB::SetModelTrAndUpdate(const Mat4& transform)
{
    if (!m_isObjectTrInit || m_ObjectTr != transform)
    {
        m_ObjectTr = transform;
        m_isObjectTrInit = true;
        
        m_ObjectTr = transform;
        Min = std::move(Geommath::Mat4ProdVec3(transform, o_min));
        Max = std::move(Geommath::Mat4ProdVec3(transform, o_max));
    }
}

bool longmarch::OOBB::VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace)
{
    /*
        Reference: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
        Reference: http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
    */
    const auto& _min = o_min;
    const auto& _max = o_max;
    m_isCulled = false;

    // Original: const auto& plane_tr = Geommath::SmartInverseTranspose(Geommath::SmartInverse(worldSpaceToViewFrustumSpace * m_ObjectTr));
    // Simplified as: glm::transpose(worldSpaceToViewFrustumSpace * m_ObjectTr);
    // Could be even more simplified as worldSpaceToViewFrustumSpace * m_ObjectTr to take advantage of row vector operation
    const auto& plane_tr = worldSpaceToViewFrustumSpace * m_ObjectTr;
    for (const auto& plane : VF.planes)
    {
        // No need to normalize plane equation here due to it will be normalized later in Plane::Distance method
        //const auto& pl = Geommath::Plane::Normalize(plane * plane_tr);
        const auto pl = plane * plane_tr;
        
        // vec * mat would treat the vector as a row vector, this is equivalent to trans(mat) * vec
        // N-P vertex test
        // Vec3f p;
        // for (int i = 0; i < 3; ++i)
        // {
        //     p[i] = (pl[i] > 0) ? _max[i] : _min[i];
        // }
        // Branchless optimization
        Vec3f p{
            Geommath::Lerp(_max[0], _min[0], signbit(pl[0])),
            Geommath::Lerp(_max[1], _min[1], signbit(pl[1])),
            Geommath::Lerp(_max[2], _min[2], signbit(pl[2]))
        };
        
        if (Geommath::Plane::Distance(pl, p) < 0)
        {
            m_isCulled = true;
            break;
        }
    }
    return m_isCulled;
}

bool longmarch::OOBB::DistanceTest(const Vec3f& center, float Near, float Far)
{
    auto r = GetRadius();
    auto pos = GetCenter();
    auto distance = glm::length(center - pos);
    {
        m_isCulled = (Far >= Near) && (distance < (Near - r) || distance > (Far + r));
        return m_isCulled;
    }
}

void longmarch::OOBB::RenderShape()
{
    Mat4 local_tr = Geommath::ToTranslateMatrix(GetOriginalCenter()) * Geommath::ToScaleMatrix(GetOriginalDiag());
    Renderer3D::RenderBoundingBox(m_ObjectTr * local_tr);
}

void OOBB::GetBoundingBoxMinMax(Vec3f& Min, Vec3f& Max) const
{
    Min = GetMin();
    Max = GetMax();
}

const Vec3f& longmarch::OOBB::GetMin() const
{
    return Min;
}

const Vec3f& longmarch::OOBB::GetMax() const
{
    return Max;
}

Vec3f longmarch::OOBB::GetOriginalCenter() const
{
    return (o_max + o_min) * 0.5f;
}

Vec3f longmarch::OOBB::GetOriginalDiag() const
{
    return o_max - o_min;
}

void longmarch::OOBB::Reset()
{
    Min = Vec3f((std::numeric_limits<float>::max)());
    Max = Vec3f((std::numeric_limits<float>::lowest)());
}

void longmarch::OOBB::Update(const Vec3f& point)
{
    Min = (glm::min)(Min, point);
    Max = (glm::max)(Max, point);
}
