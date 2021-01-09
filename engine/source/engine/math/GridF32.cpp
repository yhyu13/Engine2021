#include "engine-precompiled-header.h"
#include "GridF32.h"

longmarch::GridF32Proxy::GridF32Proxy(float* array, size_t size)
	:
	m_size(size)
{
	m_array = array;
}

longmarch::GridF32::GridF32()
	:
	m_x(0),
	m_y(0),
	m_grid(nullptr)
{}

longmarch::GridF32::GridF32(size_t x, size_t y)
	:
	m_x(x),
	m_y(y)
{
	m_grid = (float**)MemoryManager::Allocate(x * sizeof(float*));
	for (size_t i = 0; i < x; ++i)
	{
		m_grid[i] = (float*)MemoryManager::Allocate(y * sizeof(float));
		for (size_t j = 0; j < y; ++j)
		{
			m_grid[i][j] = 0.0f;
		}
	}
}

longmarch::GridF32::GridF32(const GridF32& rhs)
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

longmarch::GridF32::GridF32(GridF32&& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_grid = other.m_grid;
	other.m_grid = nullptr;
}

longmarch::GridF32::~GridF32()
{
	if (m_grid)
	{
		for (size_t i = 0; i < m_x; ++i)
		{
			MemoryManager::Free(m_grid[i], sizeof(float) * m_y);
		}
		MemoryManager::Free(m_grid, sizeof(float*) * m_x);
	}
}

size_t longmarch::GridF32::X()
{
	return m_x;
}

size_t longmarch::GridF32::Y()
{
	return m_y;
}

longmarch::ArrayF32::ArrayF32()
	:
	m_x(0),
	m_grid(nullptr)
{
}

longmarch::ArrayF32::ArrayF32(size_t x)
{
	m_x = x;
	m_grid = (float*)MemoryManager::Allocate(x * sizeof(float));
	for (size_t i = 0; i < x; ++i)
	{
		m_grid[i] = 0;
	}
}

longmarch::ArrayF32::ArrayF32(const std::initializer_list<float>& elements)
{
	m_x = elements.size();
	m_grid = (float*)MemoryManager::Allocate(m_x * sizeof(float));
	for (size_t i = 0; i < m_x; ++i)
	{
		float v = *(elements.begin() + i);
		m_grid[i] = v;
	}
}

longmarch::ArrayF32::ArrayF32(const ArrayF32& rhs)
{
	m_x = rhs.m_x;
	m_grid = (float*)MemoryManager::Allocate(m_x * sizeof(float));
	for (size_t i = 0; i < m_x; ++i)
	{
		m_grid[i] = rhs.m_grid[i];
	}
}

longmarch::ArrayF32::ArrayF32(ArrayF32&& rhs)
{
	m_x = rhs.m_x;
	m_grid = rhs.m_grid;
	rhs.m_grid = nullptr;
}

longmarch::ArrayF32::~ArrayF32()
{
	if (m_grid)
	{
		MemoryManager::Free(m_grid, sizeof(float) * m_x);
	}
	m_grid = nullptr;
}

size_t longmarch::ArrayF32::X() const
{
	return m_x;
}

ArrayF32 longmarch::ArrayF32::Inv() const
{
	ArrayF32 ret(*this);
	for (size_t i(0); i < ret.X(); ++i)
	{
		ret[i] = 1.0f / ret[i];
	}
	return (ret);
}

std::ostream& operator<<(std::ostream& o, const ArrayF32& n)
{
	o.precision(6);
	o << "(";
	for (size_t i = 0; i < n.X()-1; ++i)
	{
		o << n[i] << ",";
	}
	o << n[n.X() - 1] << ") ";
	return o;
}
