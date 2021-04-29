#pragma once
#include "engine/math/Geommath.h"
#include "engine/core/exception/EngineException.h"

namespace longmarch
{
	enum class VertexDataType
	{
		None = 0, Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, uInt, Bool,
		HFloat, HFloat2, HFloat3, HFloat4,
		NUM
	};

	struct VertexBufferElement
	{
		std::string Name;
		VertexDataType Type;
		uint32_t Diviser;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized = false;

		VertexBufferElement() {}
		VertexBufferElement(VertexDataType type, const std::string& name, uint32_t diviser = 0, bool normalized = false)
			: Name(name), Type(type), Diviser(diviser), Size(VertexDataTypeSize()), Offset(0), Normalized(normalized)
		{
		}

		uint32_t VertexDataTypeSize() const
		{
			switch (Type)
			{
			case VertexDataType::Float:  return 4;
			case VertexDataType::Float2: return 4 * 2;
			case VertexDataType::Float3: return 4 * 3;
			case VertexDataType::Float4: return 4 * 4;
			case VertexDataType::HFloat:  return 2;
			case VertexDataType::HFloat2: return 2 * 2;
			case VertexDataType::HFloat3: return 2 * 3;
			case VertexDataType::HFloat4: return 2 * 4;
			case VertexDataType::Int:    return 4;
			case VertexDataType::Int2:   return 4 * 2;
			case VertexDataType::Int3:   return 4 * 3;
			case VertexDataType::Int4:   return 4 * 4;
			case VertexDataType::uInt:   return 4;
			case VertexDataType::Bool:   return 1;
			}

			ASSERT(false, "Unknown VertexDataType!");
			return 0;
		}

		bool IsIntegralDataType() const
		{
			switch (Type)
			{
			case VertexDataType::Int:    return true;
			case VertexDataType::Int2:   return true;
			case VertexDataType::Int3:   return true;
			case VertexDataType::Int4:   return true;
			case VertexDataType::uInt:   return true;
			}
			return false;
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case VertexDataType::Float:  return 1;
			case VertexDataType::Float2: return 2;
			case VertexDataType::Float3: return 3;
			case VertexDataType::Float4: return 4;
			case VertexDataType::HFloat:  return 1;
			case VertexDataType::HFloat2: return 2;
			case VertexDataType::HFloat3: return 3;
			case VertexDataType::HFloat4: return 4;
			case VertexDataType::Int:    return 1;
			case VertexDataType::Int2:   return 2;
			case VertexDataType::Int3:   return 3;
			case VertexDataType::Int4:   return 4;
			case VertexDataType::uInt:   return 1;
			case VertexDataType::Bool:   return 1;
			}

			ASSERT(false, "Unknown VertexDataType!");
			return 0;
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() {}

		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }

		std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }

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
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class BaseDataBuffer
	{
	public:
		virtual ~BaseDataBuffer() = default;

		inline uint32_t GetFrameBufferID() const
		{
			return m_RendererID;
		}

	protected:
		uint32_t m_RendererID;
	};

	class VertexBuffer : public BaseDataBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual size_t GetCount() const = 0;
		virtual size_t GetCapacity() const = 0;

		virtual void AppendBufferData(const std::shared_ptr<VertexBuffer>& read_buffer) = 0;
		virtual void AppendBufferData(const void* data, size_t count) = 0;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<VertexBuffer>& read_buffer, size_t read_offset) = 0;
		virtual void UpdateBufferData(const void* data, size_t count) = 0;
		virtual void UpdateBufferSubData(const void* data, size_t count, size_t write_offset) = 0;

		virtual const VertexBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		static std::shared_ptr<VertexBuffer> Create(const void* data, size_t count);

		inline size_t SetElementSize(size_t s) { m_elementSize = s; }
		inline size_t GetElementSize() const { return m_elementSize; }

	protected:
		size_t m_elementSize = { sizeof(float) };
	};

	class IndexBuffer : public BaseDataBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

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

	class UniformBuffer : public BaseDataBuffer
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		virtual const VertexBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		/*
			The buffer data in the range is invalidated after this call for performance
		*/
		static void* GetUniformBufferMapping(size_t size, size_t offset);
		static void EndUniformBufferMapping();
		static std::shared_ptr<UniformBuffer> Create(const void* data, size_t size);
	};

	class ShaderStorageBuffer : public BaseDataBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() = default;

		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		virtual const VertexBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		/*
			The buffer data in the range is invalidated after this call for performance
		*/
		static void* GetShaderStorageBufferMapping(size_t size, size_t offset);
		static void EndShaderStorageBufferMapping();
		static std::shared_ptr<ShaderStorageBuffer> Create(const void* data, size_t size);
	};

	struct DrawIndexedIndirectCommand 
	{
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

	class IndexedIndirectCommandBuffer : public BaseDataBuffer
	{
	public:
		virtual ~IndexedIndirectCommandBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual size_t GetCapacity() const = 0;
		virtual size_t GetCount() const = 0;
		virtual size_t GetElementSize() const = 0;

		virtual void UpdateBufferData(const void* data, size_t size) = 0;
		static std::shared_ptr<IndexedIndirectCommandBuffer> Create(const void* data, size_t size);
	};

	class BaseTextureBuffer
	{
	public:
		enum class IMAGE_BIND_MODE : uint32_t
		{
			EMPTY = 0,
			READ_ONLY,
			WRITE_ONLY,
			READ_WRITE,
			NUM
		};

		virtual ~BaseTextureBuffer() = default;

		virtual void Bind() const { throw NotImplementedException(); }
		virtual void Unbind() const { throw NotImplementedException(); }
		virtual void BindTexture(uint32_t slot) { throw NotImplementedException(); }

		//! This method should be used to read/write with compute shader
		virtual void BindImage(uint32_t slot, BaseTextureBuffer::IMAGE_BIND_MODE mode) const { throw NotImplementedException(); }

		//! This method should be used when writing to mipmap levels
		virtual void BindMipMapLevel(uint32_t level) const { throw NotImplementedException(); }
		virtual void GenerateMipMapLevel() const { throw NotImplementedException(); }

		//! This method should be used on arry texture
		virtual void BindLayer(uint32_t slot) const { throw NotImplementedException(); }

		inline uint32_t GetFrameBufferID() const
		{
			return m_RendererID;
		}
		inline uint32_t GetRenderTargetID() const
		{
			return m_RenderTargetID;
		}
		inline Vec2u GetBufferSize() const
		{
			return Vec2u(m_width, m_height);
		}

	protected:
		uint32_t m_RendererID;
		uint32_t m_RenderTargetID;
		uint32_t m_width;
		uint32_t m_height;
	};

	class FrameBuffer : public virtual BaseTextureBuffer
	{
	public:
		enum class BUFFER_FORMAT
		{
			EMPTY = 0,
			UINT_R8,
			UINT_RGBA8,
			FLOAT_RGB16,
			FLOAT_RGBA16,
			FLOAT32_R,
			NUM
		};

		virtual ~FrameBuffer() = default;
		static std::shared_ptr<FrameBuffer> Create(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT format);

	protected:
		FrameBuffer::BUFFER_FORMAT m_format;
	};

	class ShadowBuffer : public virtual BaseTextureBuffer
	{
	public:
		enum class SHADOW_MAP_TYPE
		{
			EMPTY = 0,
			BASIC,
			BASIC_COMPARE,

			MOMENT4,
			MOMENT4_CUBE,

			ARRAY_BASIC,
			ARRAY_COMPARE,
			ARRAY_MOMENT4,
			NUM
		};

		virtual ~ShadowBuffer() = default;
		static std::shared_ptr<ShadowBuffer> Create(uint32_t width, uint32_t height, ShadowBuffer::SHADOW_MAP_TYPE type);
		static std::shared_ptr<ShadowBuffer> CreateArray(uint32_t width, uint32_t height, uint32_t depth, ShadowBuffer::SHADOW_MAP_TYPE type);

		inline Vec3u GetArrayBufferSize() const
		{
			return Vec3u(m_width, m_height, m_depth);
		}

		inline ShadowBuffer::SHADOW_MAP_TYPE GetType() const
		{
			return m_type;
		}

	protected:
		uint32_t m_depth;
		SHADOW_MAP_TYPE m_type;
	};

	class SkyBoxBuffer : public virtual BaseTextureBuffer
	{
	public:
		enum class BUFFER_FORMAT
		{
			EMPTY = 0,
			FLOAT_RGB16,
			FLOAT32_RGB,
			NUM
		};

		virtual ~SkyBoxBuffer() = default;
		static std::shared_ptr<SkyBoxBuffer> Create(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT type);

		inline uint32_t GetMaxMipMapLevel() const 
		{ 
			return m_max_level; 
		};

		inline SkyBoxBuffer::BUFFER_FORMAT GetBufferFormat() const
		{
			return m_buffer_format;
		}

	protected:
		uint32_t m_max_level;
		BUFFER_FORMAT m_buffer_format;
	};

	class GBuffer : public virtual BaseTextureBuffer
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
			NORMAL,
			VELOCITY,
			ALBEDO_EMSSIVE,
			BAKEDAO_METALLIC_ROUGHNESS,
			NUM
		};

		virtual ~GBuffer() = default;
		virtual uint32_t GetGBufferTexutureID(GBUFFER_TEXTURE_TYPE tex) const = 0;
		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const = 0;

		static std::shared_ptr<GBuffer> Create(uint32_t width, uint32_t height, GBUFFER_TYPE type);
	};

	class ComputeBuffer : public virtual BaseTextureBuffer
	{
	public:
		enum class BUFFER_FORMAT : uint32_t
		{
			EMPTY = 0,
			FLOAT_RGBA16,
			FLOAT_RGBA32,
			NUM
		};

		virtual ~ComputeBuffer() = default;
		static std::shared_ptr<ComputeBuffer> Create(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT format);

		inline ComputeBuffer::BUFFER_FORMAT GetBufferFormat() const
		{
			return m_format;
		}

	protected:
		BUFFER_FORMAT m_format;
	};


	class VoxelBuffer : public virtual BaseTextureBuffer
	{
	public:
		enum class BUFFER_FORMAT
		{
			EMPTY = 0,
			UINT_R8,
			UINT_RGBA8,
			UINT_RGB10A2,
			UINT_RG16,
			UINT_R32UI,
			NUM
		};
		enum class BUFFER_TYPE
		{
			EMPTY = 0,
			STATIC_FLAG,
			ALBEDO,
			NORMAL,
			EMISSIVE,
			RADIANCE,
			RADIANCE_MIPMAP,
			NUM
		};

		virtual ~VoxelBuffer() = default;
		virtual void BindImage(uint32_t slot, BaseTextureBuffer::IMAGE_BIND_MODE mode, VoxelBuffer::BUFFER_FORMAT format) const = 0;

		static std::shared_ptr<VoxelBuffer> Create(uint32_t width, uint32_t height, uint32_t depth, VoxelBuffer::BUFFER_TYPE type);

	protected:
		uint32_t m_depth;
		VoxelBuffer::BUFFER_TYPE m_type;
		VoxelBuffer::BUFFER_FORMAT m_format;
	};
}