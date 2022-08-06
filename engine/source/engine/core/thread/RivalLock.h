#pragma once
#include "engine/core/EngineCore.h"
#include "engine/core/utility/TypeHelper.h"

#define USE_ATOMIC_RIVAL_PTR 0

#ifndef _SHIPPING
#define DEADLOCK_TIMER 1 // Debug break unfriendly, disabled unless you need to debug deak lock
#else
#define DEADLOCK_TIMER 0
#endif // !_SHIPPING

#if DEADLOCK_TIMER
#define SET_DEADLOCK_TIMER() Timer __timer
#define ASSERT_DEADLOCK_TIMER() ASSERT(__timer.Mark() < 1.0f, "Dead lock?")
#else
#define SET_DEADLOCK_TIMER()
#define ASSERT_DEADLOCK_TIMER()
#endif

#define THREAD_YIELD() //std::this_thread::yield()

namespace longmarch
{
    // yuhang : id of a lock group
    struct RivalGroup
    {
        uint8_t m_groupID;

        friend bool operator==(const RivalGroup& lhs, const RivalGroup& rhs)
        {
            return lhs.m_groupID == rhs.m_groupID;
        }
    };

    /**
     * @brief Define N rival groups, threads from only one rival group could enter the critical section
              This is useful for the parallelism of ECS reading actions (Getter) where it can be interrupted by storing actions (Setter) in between w/o
              much caring about the correctness of data. This way, we can eliminate tetris programming of defining explicit storing or reading stages to avoid reading/writing data in the same time.
              The programmer can free to use any Getter and Setter w/o caring about atomicity
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    template <int _NUM_GROUPS_>
    class RivalLock : private BaseAtomicClassNC
    {
    public:
        static constexpr auto NUM_GROUPS = _NUM_GROUPS_;
        static_assert(NUM_GROUPS > 1);

        NONCOPYABLE(RivalLock);

        explicit RivalLock(std::initializer_list<RivalGroup> group_list)
        {
            ASSERT(group_list.size() == NUM_GROUPS, "RivalLock template does not match init argument.");
            for (int i = 0; i < NUM_GROUPS; ++i)
            {
                m_groups[i] = *(group_list.begin() + i);
                const auto& g1 = m_groups[i];
                for (int j = 0; j < i; ++j)
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

            SET_DEADLOCK_TIMER();
#if USE_ATOMIC_RIVAL_PTR
            RivalGroup* temp = nullptr;
            while (!m_currentGroupPtr.compare_exchange_weak(temp, groupPtr) && temp != groupPtr)
            {
                ASSERT_DEADLOCK_TIMER();
                temp = nullptr;
                THREAD_YIELD();
            }
#else
            auto _address = (void**)&m_currentGroupPtr;

            // TODO @yuhang : InterlockedCompareExchangePointer is limited to windows platform, implement a generic platform interface for this
            while (InterlockedCompareExchangePointer(_address, groupPtr, nullptr) != groupPtr)
            {
                ASSERT_DEADLOCK_TIMER();
                THREAD_YIELD();
            }
#endif
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
#if USE_ATOMIC_RIVAL_PTR
        CACHE_ALIGN std:atomic<RivalGroup*> m_currentGroupPtr{nullptr};
#else
        CACHE_ALIGN volatile RivalGroup* m_currentGroupPtr{nullptr};
#endif
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

#define LOCK_GUARD_RIVAL(lock, group) rival_lock_guard<(lock).NUM_GROUPS> __lock_rival(lock, group)
}

#undef USE_ATOMIC_RIVAL_PTR
#undef DEADLOCK_TIMER
#undef SET_DEADLOCK_TIMER
#undef ASSERT_DEADLOCK_TIMER
#undef THREAD_YIELD
