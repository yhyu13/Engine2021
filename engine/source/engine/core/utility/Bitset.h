#pragma once

#include "engine/core/exception/EngineException.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
    // A bitset that can hold 256 bits.
    template <typename T>
    class MS_ALIGN16 LongMarch_Bitset256
    {
    public:
        inline void Reset()
        {
            m_mask = m_mask2 = m_mask3 = m_mask4 = 0ull;
        }

        [[nodiscard]] const LongMarch_Set<T> GetAllIndices() const
        {
            LongMarch_Set<T> ret;
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

        void AddIndex(T index)
        {
            uint64_t com = static_cast<uint64_t>(index);
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

        void AddIndex(const LongMarch_Set<T>& sets)
        {
            for (auto& index : sets)
            {
                AddIndex(index);
            }
        }

        void RemoveComponent(T index)
        {
            uint64_t com = static_cast<uint64_t>(index);
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

        void RemoveIndex(const LongMarch_Set<T>& sets)
        {
            for (auto& index : sets)
            {
                RemoveComponent(index);
            }
        }

        [[nodiscard]] bool Contains(T index) const
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
                return false;
            }
        }

        // return true if old mask does not contain all 1s in target but this mask does
        [[nodiscard]] bool IsNewMatch(const LongMarch_Bitset256& oldMask, const LongMarch_Bitset256& target) const noexcept
        {
            return !oldMask.IsAMatch(target) && this->IsAMatch(target);
        }

        // return true if old mask does contain all 1s in target but this mask does not
        [[nodiscard]] bool IsNoLongerMatched(const LongMarch_Bitset256& oldMask,
                                             const LongMarch_Bitset256& target) const noexcept
        {
            return oldMask.IsAMatch(target) && !this->IsAMatch(target);
        }

        // return true if this mask contain all 1s in target
        [[nodiscard]] bool IsAMatch(const LongMarch_Bitset256& target) const noexcept
        {
            return
                ((m_mask & target.m_mask) == target.m_mask)
                && ((m_mask2 & target.m_mask2) == target.m_mask2)
                && ((m_mask3 & target.m_mask3) == target.m_mask3)
                && ((m_mask4 & target.m_mask4) == target.m_mask4);
        }

        friend inline bool operator==(const LongMarch_Bitset256& lhs, const LongMarch_Bitset256& rhs)
        {
            return lhs.m_mask == rhs.m_mask && lhs.m_mask2 == rhs.m_mask2 && lhs.m_mask3 == rhs.m_mask3 && lhs.m_mask4
                == rhs.m_mask4;
        }

        friend inline bool operator<(const LongMarch_Bitset256& lhs, const LongMarch_Bitset256& rhs)
        {
            return lhs.m_mask < rhs.m_mask || lhs.m_mask2 < rhs.m_mask2 || lhs.m_mask3 < rhs.m_mask3 || lhs.m_mask4 <
                rhs.m_mask4;
        }

    private:
        /*
            You could create arbitrary many masks that extend the total number of available components
            256 should be enough.
        */
        uint64_t m_mask{0ull};
        uint64_t m_mask2{0ull};
        uint64_t m_mask3{0ull};
        uint64_t m_mask4{0ull};
    };
}
