#pragma once

#include "BaseComponent.h"
#include "engine/core/exception/EngineException.h"
#include <set>

#define USE_LARGE_COMPONENT_BIT_MASK 0

namespace longmarch
{
	/**
	 * @brief This class holds bit-masking functionality.
	 *
	 * @detail These signatures (bit-masks) serves two chief purposes :
	 *
	 *		1#	Each entity maintains its signature that helps identify what are the
	 *			components the entity owns.
	 *
	 *		2#	Each system maintains its signature that helps identify which components
	 *			the system cares about, and thus which entities the system is interested in.
	 *
	 *	For details on systems, please see BaseComponentSystem.h
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	struct CACHE_ALIGN16 BitMaskSignature
	{
	public:
		inline void Reset()
		{
#if USE_LARGE_COMPONENT_BIT_MASK == 0
			m_mask = 0ull;
#else
			m_mask = m_mask2 = 0ull;
#endif
		}

		inline const std::set<uint32_t> GetAllComponentIndex() const
		{
			std::set<uint32_t> ret;
			if (m_mask != 0ull)
			{
				for (uint32_t i(0u); i < 64u; ++i)
				{
					if ((m_mask) & (1ull << i))
					{
						ret.insert(i);
					}
				}
			}
#if USE_LARGE_COMPONENT_BIT_MASK
			if (m_mask2 != 0ull)
			{
				for (uint32_t i(0u); i < 64u; ++i)
				{
					if ((m_mask2) & (1ull << i))
					{
						ret.insert(i + 64u);
					}
				}
			}
#endif
			return ret;
		}

		template<typename ComponentType>
		void AddComponent() 
		{
			const uint64_t com = static_cast<uint64_t>(GetComponentTypeIndex<ComponentType>());
			if (com < 64ull)
			{
				m_mask |= (1ull << com);
			}
#if USE_LARGE_COMPONENT_BIT_MASK
			else if (com < 128ull)
			{
				m_mask2 |= (1ull << (com - 64ull));
			}
#endif
			else
			{
				ENGINE_EXCEPT(L"Component Index out of range!");
			}
		}

		template<typename C1, typename C2, typename... ComponentTypes>
		void AddComponent() {
			AddComponent<C1>();
			AddComponent<C2, ComponentTypes...>();
		}

		template<typename ComponentType>
		void RemoveComponent() 
		{
			const uint64_t com = static_cast<uint64_t>(GetComponentTypeIndex<ComponentType>());
			if (com < 64ull)
			{
				m_mask &= ~(1ull << com);
			}
#if USE_LARGE_COMPONENT_BIT_MASK
			else if (com < 128ull)
			{
				m_mask2 &= ~(1ull << (com - 64ull));
			}
#endif
			else
			{
				ENGINE_EXCEPT(L"Component Index out of range!");
			}
		}

		template<typename C1, typename C2, typename... ComponentTypes>
		void RemoveComponent() 
		{
			RemoveComponent<C1>();
			RemoveComponent<C2, ComponentTypes...>();
		}

		// return true if old mask does not contain all 1s in target but this mask does
		inline bool IsNewMatch(const BitMaskSignature& oldMask, const BitMaskSignature& target) const noexcept
		{
			return !oldMask.IsAMatch(target) && this->IsAMatch(target);
		}

		// return true if old mask does contain all 1s in target but this mask does not
		inline bool IsNoLongerMatched(const BitMaskSignature& oldMask, const BitMaskSignature& target) const noexcept
		{
			return oldMask.IsAMatch(target) && !this->IsAMatch(target);
		}

		// return true if this mask contain all bits in target
		inline bool IsAMatch(const BitMaskSignature& target) const noexcept 
		{
#if USE_LARGE_COMPONENT_BIT_MASK == 0
			return ((m_mask & target.m_mask) == target.m_mask);
#else
			return ((m_mask & target.m_mask) == target.m_mask) && ((m_mask2 & target.m_mask2) == target.m_mask2);
#endif
		}

		friend inline bool operator==(const BitMaskSignature& lhs, const BitMaskSignature& rhs)
		{
#if USE_LARGE_COMPONENT_BIT_MASK == 0
			return lhs.m_mask == rhs.m_mask;
#else
			return lhs.m_mask == rhs.m_mask && lhs.m_mask2 == rhs.m_mask2;
#endif
		}

		friend inline bool operator<(const BitMaskSignature& lhs, const BitMaskSignature& rhs)
		{
#if USE_LARGE_COMPONENT_BIT_MASK == 0
			return lhs.m_mask < rhs.m_mask;
#else
			return lhs.m_mask < rhs.m_mask && lhs.m_mask2 < rhs.m_mask2;
#endif
		}

		/*
			You could create arbitrary many masks that extend the total number of available components
			64 should be enough.
		*/
		uint64_t m_mask{ 0ull };
#if USE_LARGE_COMPONENT_BIT_MASK
		uint64_t m_mask2{ 0ull };
#endif
	};
}

/*
	Custum hash function for Entity
*/
namespace std
{
	template <>
	struct hash<longmarch::BitMaskSignature>
	{
		std::size_t operator()(const longmarch::BitMaskSignature& e) const
		{
#if USE_LARGE_COMPONENT_BIT_MASK == 0
			return hash<uint64_t>()(e.m_mask);
#else
			return hash<uint64_t>()(e.m_mask) ^ hash<uint64_t>()(e.m_mask2);
#endif
		}
	};
}

#undef USE_LARGE_COMPONENT_BIT_MASK
