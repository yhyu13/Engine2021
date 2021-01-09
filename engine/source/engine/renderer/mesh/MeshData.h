#pragma once
#include "engine/EngineEssential.h"
#include "engine/math/Geommath.h"
#include "../VertexArray.h"

namespace longmarch
{
#define MESH_TRI_DATA_FORMAT 1
#define MESH_VERTEX_DATA_FORMAT 4

	class MeshData
	{
	private:
#if MESH_VERTEX_DATA_FORMAT == 0
		struct Vertex3D_0
		{
		public:
			Vec3f pnt;
			Vec3f nrm;
			Vec2f tex;
			Vec3f tan;
			Vertex3D_0()
				: pnt(Vec3f(0)), nrm(Vec3f(0)), tex(Vec2f(0)), tan(Vec3f(0))
			{}
			explicit Vertex3D_0(const Vec3f& p, const Vec3f& n, const Vec2f& t, const Vec3f& a)
				: pnt(p), nrm(n), tex(t), tan(a)
			{}
		};
	public:
		using VertexData = Vertex3D_0;
#elif MESH_VERTEX_DATA_FORMAT == 1
		struct Vertex3D_1
		{
		public:
			Vec3f pnt;
			Vec4f qTanget;
			Vec2f tex;
			Vertex3D_1()
				: pnt(Vec3f(0)), qTanget(Vec4f(0)), tex(Vec2f(0))
			{}
			explicit Vertex3D_1(const Vec3f& p, const Vec4f& n, const Vec2f& t)
				: pnt(p), qTanget(n), tex(t)
			{}
		};
	public:
		using VertexData = Vertex3D_1;
#elif MESH_VERTEX_DATA_FORMAT == 2
		struct Vertex3D_2
		{
		public:
			Vec3f pnt;
			Vec2f nrm;
			Vec2f tex;
			Vec2f tan;
			Vertex3D_2()
				: pnt(Vec3f(0)), nrm(Vec2f(0)), tex(Vec2f(0)), tan(Vec2f(0))
			{}
			explicit Vertex3D_2(const Vec3f& p, const Vec2f& n, const Vec2f& t, const Vec2f& a)
				: pnt(p), nrm(n), tex(t), tan(a)
			{}
		};
	public:
		using VertexData = Vertex3D_2;
#elif MESH_VERTEX_DATA_FORMAT == 3
		struct Vertex3D_3
		{
		public:
			Vec3f pnt;
			Vec2hf_pack nrm;
			Vec2hf_pack tex;
			Vec2hf_pack tan;
			Vertex3D_3()
				: pnt(Vec3f(0)), nrm(Vec2hf_pack(0)), tex(Vec2hf_pack(0)), tan(Vec2hf_pack(0))
			{}
			explicit Vertex3D_3(const Vec3f& p, const Vec2hf_pack& n, const Vec2hf_pack& t, const Vec2hf_pack& a)
				: pnt(p), nrm(n), tex(t), tan(a)
			{}
		};
	public:
		using VertexData = Vertex3D_3;
#elif MESH_VERTEX_DATA_FORMAT == 4
		struct Vertex3D_4
		{
		public:
			Vec4hf_pack pnt{ };
			Vec2hf_pack nrm{ };
			Vec2hf_pack tex{ };
			Vec2hf_pack tan{ };
			Vec2hf_pack boneIndexWeightPairs[3]{};
			Vertex3D_4() = default;
			explicit Vertex3D_4(const Vec4hf_pack& p, const Vec2hf_pack& n, const Vec2hf_pack& t, const Vec2hf_pack& a)
				: pnt(p), nrm(n), tex(t), tan(a)
			{}
			inline void AddBoneIndexWeightPair(const Vec2f& boneIndexAndWeight)
			{
				for (auto i = 0; i < 3; ++i)
				{
					auto& pair = boneIndexWeightPairs[i];
					if (pair == Vec2hf_pack()) //!< override if empty
					{
						pair = std::move(Geommath::PackVec2ToHVec2(boneIndexAndWeight));
						return;
					}
					else if (Geommath::UnPackVec2ToHVec2(pair).g < boneIndexAndWeight.g) //!< override if weight is higher
					{
						pair = std::move(Geommath::PackVec2ToHVec2(boneIndexAndWeight));
						return;
					}
				}
			}
		};
	public:
		using VertexData = Vertex3D_4;
#endif

#if MESH_TRI_DATA_FORMAT == 0
		using TriData = Vec3u16;
#elif MESH_TRI_DATA_FORMAT == 1
		using TriData = Vec3u;
#endif

		using VertexList = LongMarch_Vector<VertexData>;
		using IndexList = LongMarch_Vector<TriData>;

	public:
		bool Init();
		void Draw();

		inline const void* GetVertexDataPtr() const { return &(vertices[0].pnt[0]); }
		inline const void* GetIndexDataPtr() const { return &(indices[0][0]); }

		//! Get vertex data total memory size in bytes
		inline size_t GetVertexDataSize() const { return vertices.size() * GetVertexStrucSize(); }
		//! Get index data total memory size in bytes
		inline size_t GetIndexDataSize() const { return indices.size() * GetIndexStructSize(); }

		inline constexpr static size_t GetVertexStrucSize() { return sizeof(VertexData); }
		inline constexpr static size_t GetIndexStructSize() { return sizeof(TriData); }
		inline constexpr static size_t GetIndexStructElementSize() { return sizeof(TriData) / 3; }

	public:
		VertexList vertices;
		IndexList indices;

	private:
		struct
		{
			std::shared_ptr<VertexArray> MeshVertexArray{ nullptr };
			std::shared_ptr<VertexBuffer> MeshVertexBuffer{ nullptr };
			std::shared_ptr<IndexBuffer> MeshIndexBuffer{ nullptr };
		} m_BufferData;
		bool m_init{ false };
	};
}