#pragma once
#if defined(WIN32)
#define _ENABLE_EXTENDED_ALIGNED_STORAGE // VS2017 15.8 fix on aligned allocation (for phmap to work)
#endif
#include <type_traits>
#include <vector>
#include <map>
#include <phmap/phmap.h>
#include <phmap/btree.h>

namespace longmarch
{
	template <typename Key, typename T>
	using LongMarch_Map = phmap::btree_map<Key, T>; // std::map<Key, T>

	template <typename T>
	using LongMarch_Set = phmap::btree_set<T>; // std::set<T>

	// For small set of data <= 64
	template <typename T>
	using LongMarch_UnorderedSet = phmap::flat_hash_set<T>;

	// For small set of data <= 64. All data are stable upon insertion, and use this if move is expensive or not allowed
	template <typename T>
	using LongMarch_UnorderedSet_node = phmap::node_hash_set<T>;

	// For small set of data <= 64. Might move all data on insertion
	template <typename T>
	using LongMarch_UnorderedSet_flat = phmap::flat_hash_set<T>;

	// For large set of data > 64
	template <typename T>
	using LongMarch_UnorderedSet_Par = phmap::parallel_flat_hash_set<T>;

	// For large set of data > 64. All data are stable upon insertion, and use this if move is expensive or not allowed
	template <typename T>
	using LongMarch_UnorderedSet_Par_node = phmap::parallel_node_hash_set<T>;

	// For large set of data > 64. Might move all data on insertion
	template <typename T>
	using LongMarch_UnorderedSet_Par_flat = phmap::parallel_flat_hash_set<T>;

	struct LongMarch_EnumClassHash
	{
		template <typename T> std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
	};
	template <typename Key>
	using LongMarch_HashType = typename std::conditional<std::is_enum<Key>::value, LongMarch_EnumClassHash, std::hash<Key>>::type;

	// For small set of data <= 64
	template <typename Key, typename T>
	using LongMarch_UnorderedMap = phmap::flat_hash_map<Key, T, LongMarch_HashType<Key>>;

	// For small set of data <= 64. All data are stable upon insertion, and use this if move is expensive or not allowed
	template <typename Key, typename T>
	using LongMarch_UnorderedMap_node = phmap::node_hash_map<Key, T, LongMarch_HashType<Key>>;

	// For small set of data <= 64. Might move all data on insertion
	template <typename Key, typename T>
	using LongMarch_UnorderedMap_flat = phmap::flat_hash_map<Key, T, LongMarch_HashType<Key>>;

	// For large set of data > 64
	template <typename Key, typename T>
	using LongMarch_UnorderedMap_Par = phmap::parallel_flat_hash_map<Key, T, LongMarch_HashType<Key>>;

	// For large set of data > 64. All inputs are stable upon insertion, and use this if move is expensive or not allowed
	template <typename Key, typename T>
	using LongMarch_UnorderedMap_Par_node = phmap::parallel_node_hash_map<Key, T, LongMarch_HashType<Key>>;

	// For large set of data > 64. Might move all data on insertion
	template <typename Key, typename T>
	using LongMarch_UnorderedMap_Par_flat = phmap::parallel_flat_hash_map<Key, T, LongMarch_HashType<Key>>;

	template<typename T>
	struct LongMarch_ContainerView
	{
		using _iter = typename T::const_iterator;
		explicit constexpr LongMarch_ContainerView(const _iter& _begin, const _iter& _end)
			:
			m_begin(_begin), m_end(_end)
		{}
		constexpr _iter begin() const noexcept
		{
			return m_begin;
		}
		constexpr _iter end() const noexcept
		{
			return m_begin;
		}
	private:
		_iter m_begin;
		_iter m_end;
	};

	/*
		Use this method to get all components for an entity.
		(_TRIVIAL_ template resolves undefined class at compile time)
	 */
#ifndef __LongMarch_TRVIAL_TEMPLATE__
#define __LongMarch_TRVIAL_TEMPLATE__ template<typename ...__TRIVIAL__>
#endif

#ifndef LongMarch_ArraySize
#define LongMarch_ArraySize(x) (sizeof(x) / sizeof(*x))
#endif

	template <typename T>
	struct Identity
	{
		using Type = T;
	};

	template <typename T> 
	inline int LongMarch_sgn(T val)
	{
		return (T(0) < val) - (val < T(0));
	}

	template<typename T1, typename T2, typename... Ts>
	inline void LongMarch_HashCombine(std::size_t& s, const T1& v1, const T2& v2, const Ts&... vs) noexcept
	{
		LongMarch_HashCombine(s, v1);
		LongMarch_HashCombine(s, v2, vs...);
	}

	template <class T>
	inline void LongMarch_HashCombine(std::size_t& s, const T& v) noexcept
	{
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}

	template <typename E>
	constexpr auto LongMarch_ToUnderlying(E e) noexcept
	{
		return static_cast<std::underlying_type_t<E>>(e);
	}

	template<typename T, typename C>
	inline void LongMarch_Range(C& c, T value_low, T value_high, T stride)
	{
		while (value_low <= value_high)
		{
			c.push_back(value_low);
			value_low += stride;
		}
	}

	template <typename M, typename V>
	inline void LongMarch_MapInv(const  M& m, V& v) {
		for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) 
		{
			v.emplace(it->second, it->first);
		}
	}

	template <typename M, typename V>
	inline void LongMarch_MapValueToVec(const  M& m, V& v) {
		for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) 
		{
			v.push_back(it->second);
		}
	}

	template <typename M, typename V>
	inline void LongMarch_MapKeyToVec(const  M& m, V& v) {
		for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) 
		{
			v.push_back(it->first);
		}
	}

	/*
		c++ 20 has unified element find method by calling contains for all containers
	*/
#ifndef LongMarch_contains
#define LongMarch_contains(c, x) (c.contains(x))
#endif

	template<typename Func, typename T>
	void LongMarch_ForEach(Func callback, std::initializer_list<std::vector<T>> list) 
	{
		for (auto& vec : list) 
		{
			std::for_each(vec.begin(), vec.end(), callback);
		}
	}

	template <typename TValue>
	inline bool LongMarch_Contains(const std::vector<TValue>& c, const TValue& x)
	{
		return std::find(c.begin(), c.end(), x) != c.end();
	}

	template <typename TValue>
	inline void LongMarch_EraseRemove(std::vector<TValue>& c, const TValue& x)
	{
		c.erase(std::remove(c.begin(), c.end(), x), c.end());
	}

	template <typename TValue>
	inline int LongMarch_EraseFirst(std::vector<TValue>& c, const TValue& x)
	{
		int index = LongMarch_findFristIndex(c, x);
		if (index != -1)
		{
			c.erase(c.begin() + index);
		}
		return index;
	}

	template <typename TValue>
	inline int LongMarch_findFristIndex(const std::vector<TValue>& c, const TValue& x)
	{
		int index = -1;
		if (auto it = std::find(c.begin(), c.end(), x); it != c.end())
		{
			index = std::distance(c.begin(), it);
		}
		return index;
	}

	template <typename TValue>
	inline const std::vector<uint32_t> LongMarch_findAllIndices(const std::vector<TValue>& c, const TValue& x, size_t _estimate_reserve = 64)
	{
		std::vector<uint32_t> indices;
		indices.reserve(_estimate_reserve);
		auto it = c.begin();
		auto& _begin = c.begin();
		auto& _end = c.end();
		auto& _check = [&x](const TValue& _x) {return _x == x; };
		while ((it = std::find_if(it, _end, _check)) != _end)
		{
			indices.emplace_back(std::distance(_begin, it++));
		}
		return indices;
	}

	//! Find the first index i such that x<=v[i]
	template <typename TValue>
	inline int LongMarch_lowerBoundFindIndex(std::vector<TValue> v, const TValue& x)
	{
		auto it = std::lower_bound(v.begin(), v.end(), x);
		if (it == v.end() || !(x <= (*it)))
		{
			return -1;
		}
		else 
		{
			int index = std::distance(v.begin(), it);
			return index;
		}
	}

	__LongMarch_TRVIAL_TEMPLATE__
		inline const std::vector<const char*> LongMarch_StrVec2ConstChar(const std::vector<std::string>& vs)
	{
		std::vector<const char*> vc;
		std::transform(vs.begin(), vs.end(), std::back_inserter(vc), [](const std::string& s)->const char* {return s.c_str(); });
		return vc;
	}
}

#include "../allocator/MemoryManager.h"

namespace longmarch
{
#if CUSTOM_ALLOCATOR == 1
	template<class T>
	using LongMarch_Vector = std::vector<T, longmarch::Mallocator<T>>;

	template<typename Func, typename T>
	inline void LongMarch_ForEach(Func callback, std::initializer_list<LongMarch_Vector<T>> list)
	{
		for (auto& vec : list) 
		{
			std::for_each(vec.begin(), vec.end(), callback);
		}
	}

	template <typename TValue>
	inline bool LongMarch_Contains(const LongMarch_Vector<TValue>& c, const TValue& x)
	{
		return std::find(c.begin(), c.end(), x) != c.end();
	}

	template <typename TValue>
	inline void LongMarch_EraseRemove(LongMarch_Vector<TValue>& c, const TValue& x)
	{
		c.erase(std::remove(c.begin(), c.end(), x), c.end());
	}

	template <typename TValue>
	inline int LongMarch_EraseFirst(LongMarch_Vector<TValue>& c, const TValue& x)
	{
		int index = LongMarch_findFristIndex(c, x);
		if (index != -1)
		{
			c.erase(c.begin() + index);
		}
		return index;
	}

	template <typename TValue>
	inline int LongMarch_findFristIndex(const LongMarch_Vector<TValue>& c, const TValue& x)
	{
		int index = -1;
		if (auto it = std::find(c.begin(), c.end(), x); it != c.end())
		{
			index = std::distance(c.begin(), it);
		}
		return index;
	}

	template <typename TValue>
	inline const LongMarch_Vector<uint32_t> LongMarch_findAllIndices(const LongMarch_Vector<TValue>& c, const TValue& x, size_t _estimate_reserve = 64)
	{
		LongMarch_Vector<uint32_t> indices;
		indices.reserve(_estimate_reserve);
		auto it = c.begin();
		auto& _begin = c.begin();
		auto& _end = c.end();
		auto& _check = [&x](const TValue& _x) {return _x == x; };
		while ((it = std::find_if(it, _end, _check)) != _end)
		{
			indices.emplace_back(std::distance(_begin, it++));
		}
		return indices;
	}

	//! Find the first index i such that x<=v[i]
	template <typename TValue>
	inline int LongMarch_lowerBoundFindIndex(LongMarch_Vector<TValue> v, const TValue& x)
	{
		auto it = std::lower_bound(v.begin(), v.end(), x);
		if (it == v.end() || !(x<=(*it))) 
		{
			return -1;
		}
		else 
		{
			int index = std::distance(v.begin(), it);
			return index;
		}
	}

	__LongMarch_TRVIAL_TEMPLATE__
		inline const LongMarch_Vector<const char*> LongMarch_StrVec2ConstChar(const LongMarch_Vector<std::string>& vs)
	{
		LongMarch_Vector<const char*> vc;
		std::transform(vs.begin(), vs.end(), std::back_inserter(vc), [](const std::string& s)->const char* {return s.c_str(); });
		return vc;
	}
#else
	template<class T>
	using LongMarch_Vector = std::vector<T>;
#endif // CUSTOM_ALLOCATOR
}

#include "Bitset.h"
#include "Timer.h"