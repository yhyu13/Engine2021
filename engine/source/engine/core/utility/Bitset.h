#pragma once

#include "engine/core/exception/EngineException.h"
#include "engine/core/utility/TypeHelper.h"

namespace AAAAgames 
{
	template<typename T>
	class CACHE_ALIGN32 A4GAMES_Bitset 
	{
	public:

		inline void Reset()
		{
			m_mask = m_mask2 = m_mask3 = m_mask4 = 0ull;
		}

		inline const A4GAMES_Set<T> GetAllIndices()
		{
			A4GAMES_Set<T> ret;
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

		void AddIndex(T index) {
			uint64_t com = (uint64_t)index;
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
				ENGINE_EXCEPT(L"Index out of range!");
			}
		}

		void AddIndex(const A4GAMES_Set<T>& sets) {
			for (auto& index : sets)
			{
				AddIndex(index);
			}
		}

		void RemoveComponent(T index) {
			uint64_t com = (uint64_t)index;
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

		void RemoveIndex(const A4GAMES_Set<T>& sets) {
			for (auto& index : sets)
			{
				RemoveComponent(index);
			}
		}

		inline bool Contains(T index) const
		{
			uint64_t com = (uint64_t)index;
			if (com < 64ull)
			{
				return m_mask & (1ull << com);
			}
			else if (com < 128ull)
			{
				return m_mask2 & (1ull << (com - 64ull));
			}
			else if (com < 192ull)
			{
				return m_mask3 & (1ull << (com - 128ull));
			}
			else if (com < 256ull)
			{
				return m_mask4 & (1ull << (com - 192ull));
			}
			else
			{
				ENGINE_EXCEPT(L"Index out of range!");
			}
		}

		inline bool IsNewMatch(A4GAMES_Bitset& oldMask, A4GAMES_Bitset& target) const noexcept {
			return !oldMask.IsAMatch(target) && this->IsAMatch(target);
		}

		inline bool IsNoLongerMatched(A4GAMES_Bitset& oldMask, A4GAMES_Bitset& target) const noexcept {
			return oldMask.IsAMatch(target) && !this->IsAMatch(target);
		}

		inline bool IsAMatch(A4GAMES_Bitset& target) const noexcept {
			return
				((m_mask & target.m_mask) == target.m_mask)
				&& ((m_mask2 & target.m_mask2) == target.m_mask2)
				&& ((m_mask3 & target.m_mask3) == target.m_mask3)
				&& ((m_mask4 & target.m_mask4) == target.m_mask4);
		}

	private:
		/*
			You could create arbitrary many masks that extend the total number of available components
			256 should be enough.
		*/
		uint64_t m_mask = { 0ull };
		uint64_t m_mask2 = { 0ull };
		uint64_t m_mask3 = { 0ull };
		uint64_t m_mask4 = { 0ull };
	};
}