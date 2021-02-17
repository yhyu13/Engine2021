#pragma once
#include "engine/math/Geommath.h"
#include "engine/core/exception/EngineException.h"

namespace longmarch
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, uInt, Bool,
		HFloat, HFloat2, HFloat3, HFloat4
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:  return 4;
		case ShaderDataType::Float2: return 4 * 2;
		case ShaderDataType::Float3: return 4 * 3;
		case ShaderDataType::Float4: return 4 * 4;
		case ShaderDataType::HFloat:  return 2;
		case ShaderDataType::HFloat2: return 2 * 2;
		case ShaderDataType::HFloat3: return 2 * 3;
		case ShaderDataType::HFloat4: return 2 * 4;
		case ShaderDataType::Int:    return 4;
		case ShaderDataType::Int2:   return 4 * 2;
		case ShaderDataType::Int3:   return 4 * 3;
		case ShaderDataType::Int4:   return 4 * 4;
		case ShaderDataType::uInt:   return 4;
		case ShaderDataType::Bool:   return 1;
		}

		ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Diviser;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized = false;

		BufferElement() {}
		BufferElement(ShaderDataType type, const std::string& name, uint32_t diviser = 0, bool normalized = false)
			: Name(name), Type(type), Diviser(diviser), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		bool IsIntegralDataType() const
		{
			switch (Type)
			{
			case ShaderDataType::Int:    return true;
			case ShaderDataType::Int2:   return true;
			case ShaderDataType::Int3:   return true;
			case ShaderDataType::Int4:   return true;
			case ShaderDataType::uInt:   return true;
			}
			return false;
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:  return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::HFloat:  return 1;
			case ShaderDataType::HFloat2: return 2;
			case ShaderDataType::HFloat3: return 3;
			case ShaderDataType::HFloat4: return 4;
			case ShaderDataType::Int:    return 1;
			case ShaderDataType::Int2:   return 2;
			case ShaderDataType::Int3:   return 3;
			case ShaderDataType::Int4:   return 4;
			case ShaderDataType::uInt:   return 1;
			case ShaderDataType::Bool:   return 1;
			}

			ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	struct DrawIndexedIndirectCommand {
		explicit DrawIndexedIndirectCommand(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, uint32_t _baseVertex, uint32_t _baseInstance)
			:
			indexCount(_indexCount), instanceCount(_instanceCount), firstIndex(_firstIndex), baseVertex(_baseVertex), baseInstance(_baseInstance)
		{}
		uint32_t  indexCount;			// Number of element(indices) to be drawn
		uint32_t  instanceCount;		// Number of instance to be drawn
		uint32_t  firstIndex;			// Index of the first element(indices) of current mesh in the entire IBO
		uint32_t  baseVertex;			// Index of the first vertex of current mesh in the entire VBO
		uint32_t  baseInstance;			// Index of the accumulated instance offset
	};

	class IndexedIndirectCommandBuffer
	{
	public:
		virtual ~IndexedIndirectCommandBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual size_t GetCapacity() const = 0;
		virtual size_t GetCount() const = 0;
		virtual size_t GetElementSize() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		static std::shared_ptr<IndexedIndirectCommandBuffer> Create(const void* data, size_t size);
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual size_t GetCount() const = 0;
		virtual size_t GetCapacity() const = 0;

		virtual void AppendBufferData(const std::shared_ptr<VertexBuffer>& read_buffer) = 0;
		virtual void AppendBufferData(const void* data, size_t count) = 0;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<VertexBuffer>& read_buffer, size_t read_offset) = 0;
		virtual void UpdateBufferData(const void* data, size_t count) = 0;
		virtual void UpdateBufferSubData(const void* data, size_t count, size_t write_offset) = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static std::shared_ptr<VertexBuffer> Create(const void* data, size_t count);

		inline size_t SetElementSize(size_t s) { m_elementSize = s; }
		inline size_t GetElementSize() const { return m_elementSize; }
	protected:
		size_t m_elementSize = { sizeof(float) };
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual size_t GetCount() const = 0;
		virtual size_t GetCapacity() const = 0;

		virtual void AppendBufferData(const std::shared_ptr<IndexBuffer>& read_buffer) = 0;
		virtual void AppendBufferData(const void* data, size_t count) = 0;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<IndexBuffer>& read_buffer, size_t read_offset) = 0;
		virtual void UpdateBufferData(const void* data, size_t count) = 0;
		virtual void UpdateBufferSubData(const void* data, size_t count, size_t write_offset) = 0;

		static std::shared_ptr<IndexBuffer> Create(const void* data, size_t size, size_t elementSize);

		inline void SetElementSize(size_t s) { m_elementSize = s; }
		inline size_t GetElementSize() const { return m_elementSize; }
	protected:
		size_t m_elementSize = { sizeof(uint32_t) };
	};

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		/*
			The buffer data in the range is invalidated after this call for performance
		*/
		static void* GetUniformBufferMapping(size_t size, size_t offset);
		static void EndUniformBufferMapping();
		static std::shared_ptr<UniformBuffer> Create(const void* data, size_t size);
	};

	class ShaderStorageBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() = default;

		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		/*
			The buffer data in the range is invalidated after this call for performance
		*/
		static void* GetShaderStorageBufferMapping(size_t size, size_t offset);
		static void EndShaderStorageBufferMapping();
		static std::shared_ptr<ShaderStorageBuffer> Create(const void* data, size_t size);
	};

	class FrameBuffer
	{
	public:
		enum class BUFFER_FORMAT
		{
			UINT8,
			Float16,
		};

		virtual ~FrameBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindTexture(uint32_t slot) const = 0;
		virtual void GenerateMipmaps() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetRenderTargetID() const = 0;
		static std::shared_ptr<FrameBuffer> Create(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT format);

		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

	protected:
		FrameBuffer::BUFFER_FORMAT m_format;
		uint32_t m_width;
		uint32_t m_height;
	};

	class ShadowBuffer
	{
	public:

		enum class SHADOW_MAP_TYPE
		{
			BASIC = 0,
			MOMENT4,
			MOMENT4_CUBE,

			ARRAY_BASIC,
			ARRAY_MOMENT4
		};

		virtual ~ShadowBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindTexture(uint32_t slot) const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetRenderTargetID() const = 0;
		static std::shared_ptr<ShadowBuffer> Create(uint32_t width, uint32_t height, ShadowBuffer::SHADOW_MAP_TYPE type);

		virtual void BindLayer(uint32_t slot) const { throw NotImplementedException(); }
		static std::shared_ptr<ShadowBuffer> CreateArray(uint32_t width, uint32_t height, uint32_t depth, ShadowBuffer::SHADOW_MAP_TYPE type);

		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

		inline Vec3u GetArrayBufferSize() const
		{
			return Vec3u(m_width, m_height, m_depth);
		}

		inline ShadowBuffer::SHADOW_MAP_TYPE GetType() const
		{
			return m_type;
		}

	protected:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		SHADOW_MAP_TYPE m_type;
	};

	class SkyBoxBuffer
	{
	public:

		enum class BUFFER_FORMAT
		{
			Float16,
			Float32,
		};

		virtual ~SkyBoxBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindTexture(uint32_t slot) const = 0;
		virtual void BindMipMap(uint32_t level) const = 0; // This method should be used when writing to mipmap levels
		virtual void GenerateMipmaps() const = 0;

		virtual uint32_t GetMaxMipMapLevel() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetRenderTargetID() const = 0;
		static std::shared_ptr<SkyBoxBuffer> Create(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT type);

		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

		inline SkyBoxBuffer::BUFFER_FORMAT GetBufferFormat() const
		{
			return m_buffer_format;
		}

	protected:
		uint32_t m_width;
		uint32_t m_height;
		BUFFER_FORMAT m_buffer_format;
	};

	class GBuffer
	{
	public:
		enum class GBUFFER_TYPE : uint32_t
		{
			EMPTY = 0,
			DEFAULT,
			THIN,
			NUM
		};

		enum class GBUFFER_TEXTURE_TYPE : uint32_t
		{
			EMPTY = 0,
			DEPTH,
			NORMAL_VELOCITY,
			ALBEDO_EMSSIVE,
			AO_METALLIC_ROUGHNESS,
			NUM
		};

		virtual ~GBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetTexutureID(GBUFFER_TEXTURE_TYPE tex) const = 0;

		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const = 0;

		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

		static std::shared_ptr<GBuffer> Create(uint32_t width, uint32_t height, GBUFFER_TYPE type);

	protected:
		uint32_t m_width;
		uint32_t m_height;
	};

	class ComputeBuffer
	{
	public:

		enum class BUFFER_FORMAT : uint32_t
		{
			EMPTY = 0,
			Float16,
			Float32,
			NUM
		};

		enum class TEXTURE_BIND_MODE : uint32_t
		{
			EMPTY = 0,
			READ_ONLY,
			WRITE_ONLY,
			READ_WRITE,
			NUM
		};

		virtual ~ComputeBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindTexture(uint32_t slot, ComputeBuffer::TEXTURE_BIND_MODE mode) const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetRenderTargetID() const = 0;
		static std::shared_ptr<ComputeBuffer> Create(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT format);

		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

		inline ComputeBuffer::BUFFER_FORMAT GetBufferFormat() const
		{
			return m_buffer_format;
		}

	protected:
		uint32_t m_width;
		uint32_t m_height;
		BUFFER_FORMAT m_buffer_format;
	};
}