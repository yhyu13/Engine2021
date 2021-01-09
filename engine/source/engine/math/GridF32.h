#pragma once
#include "engine/core/allocator/MemoryManager.h"
#include "engine/math/Geommath.h"
#include "engine/core/exception/EngineException.h"

namespace longmarch {
	/*
		Proxy class of GridF32 that enables double square operator
	*/
	class GridF32Proxy
	{
	public:
		GridF32Proxy() = delete;
		explicit GridF32Proxy(float* array, size_t size);
		float& operator[](size_t i)
		{
			if ((i + 1) <= (m_size))
			{
				return m_array[i];
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}
		const float& operator[](size_t i) const
		{
			if ((i + 1) <= (m_size))
			{
				return m_array[i];
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}
	private:
		float* m_array;
		size_t m_size;
	};

	/*
		Dynamically allocated 2D float array
	*/
	class GridF32
	{
	public:
		GridF32();
		explicit GridF32(size_t x, size_t y);
		GridF32(const GridF32& other);
		GridF32(GridF32&& other);
		~GridF32();
		size_t X();
		size_t Y();

		GridF32& operator=(const GridF32& rhs)
		{
			if (this != &rhs)
			{
				auto x = rhs.m_x;
				auto y = rhs.m_y;
				m_x = x;
				m_y = y;
				m_grid = (float**)MemoryManager::Allocate(x * sizeof(float*));
				for (size_t i = 0; i < x; ++i)
				{
					m_grid[i] = (float*)MemoryManager::Allocate(y * sizeof(float));
					for (size_t j = 0; j < y; ++j)
					{
						m_grid[i][j] = rhs.m_grid[i][j];
					}
				}
			}
			return *this;
		}
		GridF32& operator=(GridF32&& rhs)
		{
			if (this != &rhs)
			{
				auto x = rhs.m_x;
				auto y = rhs.m_y;
				m_x = x;
				m_y = y;
				m_grid = rhs.m_grid;
				rhs.m_grid = nullptr;
			}
			return *this;
		}
		GridF32Proxy operator[](size_t i)
		{
			if ((i + 1) <= (m_x))
			{
				return GridF32Proxy(m_grid[i], m_y);
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}
		const GridF32Proxy operator[](size_t i) const
		{
			if ((i + 1) <= (m_x))
			{
				return GridF32Proxy(m_grid[i], m_y);
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}

	private:
		float** m_grid;
		size_t m_x;
		size_t m_y;
	};

	/*
		Dynamically allocated 1D float array
	*/
	class ArrayF32
	{
	public:
		ArrayF32();
		explicit ArrayF32(size_t x);
		explicit ArrayF32(const std::initializer_list<float>& elements);
		ArrayF32(const ArrayF32& rhs);
		ArrayF32(ArrayF32&& rhs);
		~ArrayF32();
		size_t X() const;
		ArrayF32 Inv() const;

		ArrayF32& operator=(const ArrayF32& rhs)
		{
			if (this != &rhs)
			{
				auto x = rhs.m_x;
				if (m_x != rhs.m_x)
				{
					this->~ArrayF32();
					m_x = x;
					m_grid = (float*)MemoryManager::Allocate(x * sizeof(float));
				}
				for (size_t i = 0; i < x; ++i)
				{
					m_grid[i] = rhs.m_grid[i];
				}
			}
			return *this;
		}
		ArrayF32& operator=(ArrayF32&& rhs)
		{
			if (this != &rhs)
			{
				this->~ArrayF32();
				auto x = rhs.m_x;
				m_x = x;
				m_grid = rhs.m_grid;
				rhs.m_grid = nullptr;
			}
			return *this;
		}
		float& operator[](size_t i)
		{
			if ((i + 1) <= (m_x))
			{
				return m_grid[i];
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}

		const float& operator[](size_t i) const
		{
			if ((i + 1) <= (m_x))
			{
				return m_grid[i];
			}
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Array " + str2wstr(Str(i)) + L" out of bound!");
		}

		ArrayF32 operator*(float value)
		{
			ArrayF32 ret(*this);
			for (size_t i(0); i < ret.m_x; ++i)
			{
				ret.m_grid[i] *= value;
			}
			return (ret);
		}

		ArrayF32 operator+(float value)
		{
			ArrayF32 ret(*this);
			for (size_t i(0); i < ret.m_x; ++i)
			{
				ret.m_grid[i] += value;
			}
			return ret;
		}

		ArrayF32 operator*(const ArrayF32& rhs)
		{
			ArrayF32 ret(*this);
			ASSERT(rhs.m_x == m_x, "LHS has size: " + Str(m_x) + ", but RHS has size: " + Str(rhs.m_x) + "!");
			for (size_t i(0); i < ret.m_x; ++i)
			{
				ret.m_grid[i] *= rhs.m_grid[i];
			}
			return ret;
		}

		ArrayF32 operator+(const ArrayF32& rhs)
		{
			ArrayF32 ret(*this);
			ASSERT(rhs.m_x == m_x, "LHS has size: " + Str(m_x) + ", but RHS has size: " + Str(rhs.m_x) + "!");
			for (size_t i(0); i < ret.m_x; ++i)
			{
				ret.m_grid[i] += rhs.m_grid[i];
			}
			return ret;
		}

		ArrayF32& operator*=(float value)
		{
			for (size_t i(0); i < m_x; ++i)
			{
				m_grid[i] *= value;
			}
			return *this;
		}

		ArrayF32& operator+=(float value)
		{
			for (size_t i(0); i < m_x; ++i)
			{
				m_grid[i] += value;
			}
			return *this;
		}

		ArrayF32& operator*=(const ArrayF32& rhs)
		{
			for (size_t i(0); i < m_x; ++i)
			{
				m_grid[i] *= rhs.m_grid[i];
			}
			return *this;
		}

		ArrayF32& operator+=(const ArrayF32& rhs)
		{
			ASSERT(rhs.m_x == m_x, "LHS has size: " + Str(m_x) + ", but RHS has size: " + Str(rhs.m_x) + "!");
			for (size_t i(0); i < m_x; ++i)
			{
				m_grid[i] += rhs.m_grid[i];
			}
			return *this;
		}

		float dot(const ArrayF32& rhs)
		{
			ASSERT(rhs.m_x == m_x, "LHS has size: " + Str(m_x) + ", but RHS has size: " + Str(rhs.m_x) + "!");
			float ret = 0.f;
			for (size_t i(0); i < m_x; ++i)
			{
				ret += (m_grid[i] * rhs.m_grid[i]);
			}
			return ret;
		}

	private:
		float* m_grid;
		size_t m_x;
	};
}

std::ostream& operator<<(std::ostream& o, const ArrayF32& n);
