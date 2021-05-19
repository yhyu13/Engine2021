#pragma once

#include "BaseComponent.h"
#include "engine/core/exception/EngineException.h"
#include <set>

namespace longmarch {
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
	class CACHE_ALIGN16 BitMaskSignature 
	{
	public:
		inline void Reset()
		{
			m_mask = m_mask2 = m_mask3 = m_mask4 = 0ull;
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
			if (m_mask3 != 0ull)
			{
				for (uint32_t i(0u); i < 64u; ++i)
				{
					if ((m_mask3) & (1ull << i))
					{
						ret.insert(i + 128u);
					}
				}
			}
			if (m_mask4 != 0ull)
			{
				for (uint32_t i(0u); i < 64u; ++i)
				{
					if ((m_mask4) & (1ull << i))
					{
						ret.insert(i + 192u);
					}
				}
			}
			return ret;
		}

		template<typename ComponentType>
		void AddComponent() 
		{
			uint64_t com = static_cast<uint64_t>(GetComponentTypeIndex<ComponentType>());
			if (com < 64ull)
			{
				m_mask |= (1ull << com);
			}
			else if (com < 128ull)
			{
				m_mask2 |= (1ull << (com - 64ull));
			}
			else if (com < 192ull)
			{
				m_mask3 |= (1ull << (com - 128ull));
			}
			else if (com < 256ull)
			{
				m_mask4 |= (1ull << (com - 192ull));
			}
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
			uint64_t com = static_cast<uint64_t>(GetComponentTypeIndex<ComponentType>());
			if (com < 64ull)
			{
				m_mask &= ~(1ull << com);
			}
			else if (com < 128ull)
			{
				m_mask2 &= ~(1ull << (com - 64ull));
			}
			else if (com < 192ull)
			{
				m_mask3 &= ~(1ull << (com - 128ull));
			}
			else if (com < 256ull)
			{
				m_mask4 &= ~(1ull << (com - 192ull));
			}
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

		// return true if this mask contain all 1s in target
		inline bool IsAMatch(const BitMaskSignature& target) const noexcept 
		{
			return
				((m_mask & target.m_mask) == target.m_mask)
				&& ((m_mask2 & target.m_mask2) == target.m_mask2)
				&& ((m_mask3 & target.m_mask3) == target.m_mask3)
				&& ((m_mask4 & target.m_mask4) == target.m_mask4);
		}

		friend inline bool operator==(const BitMaskSignature& lhs, const BitMaskSignature& rhs)
		{
			return lhs.m_mask == rhs.m_mask && lhs.m_mask2 == rhs.m_mask2 && lhs.m_mask3 == rhs.m_mask3 && lhs.m_mask4 == rhs.m_mask4;
		}

		friend inline bool operator<(const BitMaskSignature& lhs, const BitMaskSignature& rhs) 
		{
			return lhs.m_mask < rhs.m_mask || lhs.m_mask2 < rhs.m_mask2 || lhs.m_mask3 < rhs.m_mask3 || lhs.m_mask4 < rhs.m_mask4;
		}

	private:
		/*
			You could create arbitrary many masks that extend the total number of available components
			256 should be enough.
		*/
		uint64_t m_mask{ 0ull };
		uint64_t m_mask2{ 0ull };
		uint64_t m_mask3{ 0ull };
		uint64_t m_mask4{ 0ull };
	};
}