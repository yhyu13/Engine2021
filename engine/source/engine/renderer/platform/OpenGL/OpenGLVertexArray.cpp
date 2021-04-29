#include "engine-precompiled-header.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace longmarch {

	static GLenum VertexDataTypeToOpenGLBaseType(VertexDataType type)
	{
		switch (type)
		{
		case longmarch::VertexDataType::Float:    return GL_FLOAT;
		case longmarch::VertexDataType::Float2:   return GL_FLOAT;
		case longmarch::VertexDataType::Float3:   return GL_FLOAT;
		case longmarch::VertexDataType::Float4:   return GL_FLOAT;
		case longmarch::VertexDataType::HFloat:    return GL_HALF_FLOAT;
		case longmarch::VertexDataType::HFloat2:   return GL_HALF_FLOAT;
		case longmarch::VertexDataType::HFloat3:   return GL_HALF_FLOAT;
		case longmarch::VertexDataType::HFloat4:   return GL_HALF_FLOAT;
		case longmarch::VertexDataType::Int:      return GL_INT;
		case longmarch::VertexDataType::Int2:     return GL_INT;
		case longmarch::VertexDataType::Int3:     return GL_INT;
		case longmarch::VertexDataType::Int4:     return GL_INT;
		case longmarch::VertexDataType::uInt:	 return GL_UNSIGNED_INT;
		case longmarch::VertexDataType::Bool:     return GL_BOOL;
		}

		ASSERT(false, "Unknown VertexDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
		:
		m_LayoutIndexCount(0)
	{
		glGenVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		ASSERT(vertexBuffer->GetLayout().GetElements().size() != 0, "Vertex Buffer has no layout!");

		Bind();
		vertexBuffer->Bind();

		uint32_t index = m_LayoutIndexCount;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			if (element.IsIntegralDataType())
			{
				glVertexAttribIPointer(index, element.GetComponentCount(), VertexDataTypeToOpenGLBaseType(element.Type), layout.GetStride(), (const void*)element.Offset);
			}
			else
			{
				glVertexAttribPointer(index, element.GetComponentCount(), VertexDataTypeToOpenGLBaseType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)element.Offset);
			}
			glVertexAttribDivisor(index, element.Diviser);
			index++;
		}
		m_LayoutIndexCount = index;
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		Bind();
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}
