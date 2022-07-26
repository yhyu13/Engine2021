#pragma once
#include "engine/core/EngineCore.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
    struct RivalGroup
    {
        uint8_t m_groupID;
    };

    template <int _NUM_GROUPS_>
    class RivalLock : private BaseAtomicClassNC
    {
    public:
        static constexpr auto NUM_GROUPS = _NUM_GROUPS_;
        static_assert(NUM_GROUPS > 1);

        NONCOPYABLE(RivalLock);

        RivalLock() = default;

        explicit RivalLock(std::initializer_list<RivalGroup> list)
        {
            ASSERT(list.size() == NUM_GROUPS, "RivalLock template does not match init argument.");
            m_groups = list;
            for (int i = 0; i < NUM_GROUPS; ++i)
            {
                const auto& g1 = m_groups[i];
                for (int j = i + 1; j < NUM_GROUPS; ++j)
                {
                    const auto& g2 = m_groups[j];
                    ASSERT(g1 != g2, "RivalLock accepts unique groups only.");
                }
            }
        }

        void Lock(RivalGroup group)
        {
            RivalGroup* groupPtr = nullptr;
            for (int i = 0; i < NUM_GROUPS; ++i)
            {
                if (group == m_groups[i])
                {
                    groupPtr = &m_groups[i];
                    break;
                }
            }

            {
                LOCK_GUARD_NC();
                if (m_currentGroupPtr == groupPtr)
                {
                    ++m_programCounter;
                    return;
                }
            }

            // RivalGroup* temp = nullptr;
            // while (!m_currentGroupPtr.compare_exchange_weak(temp, groupPtr) && temp != groupPtr)
            // {
            //     temp = nullptr;
            //     std::this_thread::yield();
            // }

            auto _address = (void**)&m_currentGroupPtr;
            while (InterlockedCompareExchangePointer(_address, groupPtr, nullptr) != groupPtr)
            {
                std::this_thread::yield();
            }
            ++m_programCounter;
        }

        void Unlock()
        {
            LOCK_GUARD_NC();
            if (--m_programCounter == 0)
            {
                m_currentGroupPtr = nullptr;
            }
        }

    private:
        // CACHE_ALIGN std:atomic<RivalGroup*> m_currentGroupPtr{nullptr};
        CACHE_ALIGN volatile RivalGroup* m_currentGroupPtr{nullptr};
        CACHE_ALIGN std::atomic_uint_fast32_t m_programCounter{0};
        RivalGroup m_groups[NUM_GROUPS];
    };

    template <int _NUM_GROUPS_>
    struct rival_lock_guard
    {
        NONCOPYABLE(rival_lock_guard);
        rival_lock_guard() = delete;

        explicit rival_lock_guard(RivalLock<_NUM_GROUPS_>& flag, RivalGroup group)
            :
            m_lock(&flag)
        {
            m_lock->Lock(group);
        }

        ~rival_lock_guard()
        {
            m_lock->Unlock();
        }

    private:
        RivalLock<_NUM_GROUPS_>* m_lock;
    };

#define LOCK_GUARD_RIVAL(lock, group) rival_lock_guard<(lock.NUM_GROUPS)> __lock_rival(lock, group)
}
