#include "engine-precompiled-header.h"
#include "AABB.h"

#include "engine/renderer/Renderer3D.h"

AAAAgames::AABB::AABB()
	:
	Shape(SHAPE_TYPE::AABB)
{
	Reset();
	ResetOriginal();
}

AAAAgames::AABB::AABB(const Vec3f& min, const Vec3f& max)
	:
	Shape(SHAPE_TYPE::AABB)
{
	Reset();
	Update(min);
	Update(max);
	ResetOriginal();
	UpdateOriginal(min);
	UpdateOriginal(max);
}

AAAAgames::AABB::AABB(MeshData* mesh)
	:
	Shape(SHAPE_TYPE::AABB)
{
	ResetOriginal();
	InitWithMeshData(mesh->vertices, mesh->indices);
}

AAAAgames::AABB::AABB(const A4GAMES_Vector<MeshData*>& meshs)
	:
	Shape(SHAPE_TYPE::AABB)
{
	ResetOriginal();
	for (const auto& mesh : meshs)
	{
		InitWithMeshData(mesh->vertices, mesh->indices);
	}
}

void AAAAgames::AABB::InitWithMeshData(const MeshData::VertexList& vertex_data, const MeshData::IndexList& index_data)
{
	LOCK_GUARD_NC();
	if (vertex_data.empty())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Mesh data does not exist! Either the mesh data has been destroied after sending to GPU or the mesh data has not be loaded yet!");
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

void AAAAgames::AABB::ResetOriginal()
{
	o_min = Vec3f((std::numeric_limits<float>::max)());
	o_max = -o_min;
}

void AAAAgames::AABB::UpdateOriginal(const Vec3f& point)
{
	o_min = (glm::min)(o_min, point);
	o_max = (glm::max)(o_max, point);
}

const std::vector<Vec3f> AAAAgames::AABB::GetAllVertex()
{
	LOCK_GUARD_NC();
	std::vector<Vec3f> ret(8);
	Vec3f _min = Min, _max = Max;
	Vec4f abc = Vec4f(_max - _min, 0);
	ret[0] = _min;
	ret[1] = _min + abc.xww;
	ret[2] = _min + abc.xyw;
	ret[3] = _min + abc.wyw;

	ret[4] = _min + abc.wwz;
	ret[5] = _min + abc.xwz;
	ret[6] = _min + abc.xyz;
	ret[7] = _min + abc.wyz;
	return ret;
}

Vec3f AAAAgames::AABB::GetMax() const
{
	return Max;
}

Vec3f AAAAgames::AABB::GetMin() const
{
	return Min;
}

Vec3f AAAAgames::AABB::GetOriginalMax() const
{
	return o_max;
}

Vec3f AAAAgames::AABB::GetOriginalMin() const
{
	return o_min;
}

void AAAAgames::AABB::SetModelTrAndUpdate(const Mat4& transform)
{
	LOCK_GUARD_NC();
	Reset(); // TODO: PRolem
	m_ObjectTr = transform;
	for (const auto& v : GetAllVertexOriginal())
	{
		Update(Geommath::Mat4ProdVec3(transform, v));
	}
}

void AAAAgames::AABB::Reset()
{
	Min = Vec3f((std::numeric_limits<float>::max)());
	Max = -Min;
}

const std::vector<Vec3f> AAAAgames::AABB::GetAllVertexOriginal()
{
	std::vector<Vec3f> ret(8);
	{
		Vec3f _min = o_min, _max = o_max;
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

void AAAAgames::AABB::Update(const Vec3f& point)
{
	Min = (glm::min)(Min, point);
	Max = (glm::max)(Max, point);
}

bool AAAAgames::AABB::VFCTest(const ViewFrustum& VF, const Mat4& worldSpaceToViewFrustumSpace)
{
	LOCK_GUARD_NC();
	/*
		Reference: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
		Reference: http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
	*/
	const auto& _min = Min;
	const auto& _max = Max;

	// Original: const auto& plane_tr = Geommath::SmartInverseTranspose(Geommath::SmartInverse(worldSpaceToViewFrustumSpace));
	// Simplified as: glm::transpose(worldSpaceToViewFrustumSpace);
	// Could be event more simplified as worldSpaceToViewFrustumSpace to take advantage of row vector operation

	/*
	const auto& plane_tr = worldSpaceToViewFrustumSpace;
	for (const auto& plane : VF.planes)
	{
		const auto& pl = Geommath::NormalizedPlaneEq(plane * plane_tr); // vec * mat would treat the vector as a row vector, this is equivalent to trans(mat) * vec
		// N-P vertex test
		Vec3f p;
		for (int i = 0; i < 3; ++i)
		{
			p[i] = (pl[i] > 0) ? _max[i] : _min[i];
		}
		if (Geommath::PlaneDistanceFromPoint(pl, p) < 0)
		{
			m_isCulled = true;
			return m_isCulled;
		}
	}*/
	m_isCulled = false;
	return m_isCulled;
}

bool AAAAgames::AABB::DistanceTest(const Vec3f& center, float Near, float Far)
{
	LOCK_GUARD_NC();
	auto r = GetRadius();
	auto pos = GetCenter();
	auto distance = glm::length(center - pos);
	{
		m_isCulled = (Far >= Near) && (distance < (Near - r) || distance >(Far + r));
		return m_isCulled;
	}
}

void AAAAgames::AABB::RenderShape()
{
	LOCK_GUARD_NC();
	Mat4 local_tr = Geommath::ToTranslateMatrix(GetCenter()) * Geommath::ToScaleMatrix(GetDiag());
	Renderer3D::RenderBoundingBox(local_tr);
}

void AAAAgames::AABB::SetMax(const Vec3f& max)
{
	Max = max;
}

void AAAAgames::AABB::SetMin(const Vec3f& min)
{
	Min = min;
}

void AAAAgames::AABB::SetOriginalMax(const Vec3f& max)
{
	o_max = max;
}

void AAAAgames::AABB::SetOriginalMin(const Vec3f& min)
{
	o_min = min;
}


void AAAAgames::AABB::SetCenter(const Vec3f& center)
{
	Vec3f extents = (Max - Min) * 0.5f;

	Min = center - extents;
	Max = center + extents;
}

f32 AAAAgames::AABB::ComputeSurfaceArea() const
{
	Vec3f widths(Max - Min);

	return 2.0f * (widths.x * widths.y + widths.x * widths.z + widths.y * widths.z);
}

bool AAAAgames::AABB::Overlaps(const AABB& aabb) const
{
	bool isOverlapping = true;

	// break if non-overlapping axis is found
	for (size_t i = 0; i < 3; ++i)
	{
		if (aabb.Max[i] < Min[i] || aabb.Min[i] > Max[i])
		{
			isOverlapping = false;
			break;
		}
	}

	return isOverlapping;
}

void AAAAgames::AABB::Merge(const AABB& aabb)
{
	for (size_t i = 0; i < 3; ++i)
	{
		Min[i] = std::min(Min[i], aabb.Min[i]);
		Max[i] = std::max(Max[i], aabb.Max[i]);
	}
}

void AAAAgames::AABB::Merge(const AABB& aabb1, const AABB& aabb2)
{
	for (size_t i = 0; i < 3; ++i)
	{
		Min[i] = std::min(aabb1.Min[i], aabb2.Min[i]);
		Max[i] = std::max(aabb1.Max[i], aabb2.Max[i]);
	}
}

Vec3f AAAAgames::AABB::GetDiag()
{
	return (Max - Min);
	//return o_max = o_min;
}

float AAAAgames::AABB::GetRadius()
{
	return glm::length(GetDiag() * 0.5f);
}

Vec3f AAAAgames::AABB::GetCenter()
{
	return (Min + Max) * 0.5f;
}

Vec3f AAAAgames::AABB::GetOriginalDiag() const
{
	return (o_max - o_min);
}

Vec3f AAAAgames::AABB::GetOriginalCenter() const
{
	return (o_min + o_max) * 0.5f;
}