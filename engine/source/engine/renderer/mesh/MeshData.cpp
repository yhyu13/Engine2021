#include "engine-precompiled-header.h"
#include "MeshData.h"
#include "../Renderer3D.h"

bool longmarch::MeshData::Init()
{
	if (m_init)
	{
		return true;
	}
	else
	{
		m_init = true;
		ENGINE_EXCEPT_IF(vertices.empty() || indices.empty(), L"Vertices and indexes are empty before VBO and IBO are initialized!");
		// Canoical rendering method that allow each mesh to store its own VAO
		{
			// Register mesh data on GPU
			m_BufferData.MeshVertexArray = VertexArray::Create();
			m_BufferData.MeshVertexBuffer = VertexBuffer::Create(GetVertexDataPtr(), GetVertexDataSize());
#if MESH_VERTEX_DATA_FORMAT == 0
			m_BufferData.MeshVertexBuffer->SetLayout({
				{ VertexDataType::Float3, "a_VertexPnt" },
				{ VertexDataType::Float3, "a_VertexNrm" },
				{ VertexDataType::Float2, "a_VertexTex" },
				{ VertexDataType::Float3, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 1
			m_BufferData.MeshVertexBuffer->SetLayout({
				{ VertexDataType::Float3, "a_VertexPnt" },
				{ VertexDataType::Float4, "a_VertexNrm" },
				{ VertexDataType::Float2, "a_VertexTex" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 2
			m_BufferData.MeshVertexBuffer->SetLayout({
				{ VertexDataType::Float3, "a_VertexPnt" },
				{ VertexDataType::Float2, "a_VertexNrm" },
				{ VertexDataType::Float2, "a_VertexTex" },
				{ VertexDataType::Float2, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 3
			m_BufferData.MeshVertexBuffer->SetLayout({
				{ VertexDataType::Float3, "a_VertexPnt" },
				{ VertexDataType::HFloat2, "a_VertexNrm" },
				{ VertexDataType::HFloat2, "a_VertexTex" },
				{ VertexDataType::HFloat2, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 4
			m_BufferData.MeshVertexBuffer->SetLayout({
				{ VertexDataType::HFloat4, "a_VertexPnt" },
				{ VertexDataType::HFloat2, "a_VertexNrm" },
				{ VertexDataType::HFloat2, "a_VertexTex" },
				{ VertexDataType::HFloat2, "a_VertexTan" },
				{ VertexDataType::HFloat2, "a_BoneIndexWeight1" },
				{ VertexDataType::HFloat2, "a_BoneIndexWeight2" },
				{ VertexDataType::HFloat2, "a_BoneIndexWeight3" }
				});
#endif
			m_BufferData.MeshVertexArray->AddVertexBuffer(m_BufferData.MeshVertexBuffer);
			m_BufferData.MeshIndexBuffer = IndexBuffer::Create(GetIndexDataPtr(), GetIndexDataSize(), MeshData::GetIndexStructElementSize());
			m_BufferData.MeshVertexArray->SetIndexBuffer(m_BufferData.MeshIndexBuffer);
		}
		return true;
	}
}

void longmarch::MeshData::Draw()
{
	if (Init())
	{
		Renderer3D::DrawMesh(m_BufferData.MeshVertexArray);
	}
}