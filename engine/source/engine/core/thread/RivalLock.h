#pragma once
#include "engine/core/EngineCore.h"
#include "engine/core/utility/TypeHelper.h"

// Use std atomic pointer or use platform specific interlock exchange method
#if defined(WIN32) || defined(WINDOWS_APP)
#define USE_ATOMIC_RIVAL_PTR 0 
#else
#define USE_ATOMIC_RIVAL_PTR 1
#endif

#ifndef _SHIPPING
#define DEADLOCK_TIMER 1 // Debug break unfriendly, disabled unless you need to debug dead lock
#else
#define DEADLOCK_TIMER 0
#endif // !_SHIPPING

#if DEADLOCK_TIMER
#define SET_DEADLOCK_TIMER() LazyInitializedTimer _timer
#define ASSERT_DEADLOCK_TIMER() ASSERT(_timer.Mark() < 1.f, "Dead lock?")
#else
#define SET_DEADLOCK_TIMER()
#define ASSERT_DEADLOCK_TIMER()
#endif

#define SPIN_COUNT 16
#define THREAD_PAUSE() _mm_pause() // pause for ~75 clocks on intel 11th-i7 11700
#define THREAD_YIELD() std::this_thread::yield() // pause for ~400 clocks on intel 11th-i7 11700

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
    template <int ARG_NUM_GROUPS>
    class RivalLock
    {
    public:
        static constexpr auto NUM_GROUPS = ARG_NUM_GROUPS;
        static_assert(NUM_GROUPS > 1);

        NONCOPYABLE(RivalLock);

        explicit RivalLock()
        {
            for (int i = 0; i < NUM_GROUPS; ++i)
            {
                m_groups[i].m_groupID = i;
            }
        }

        void Lock(RivalGroup group)
        {
            ASSERT(group.m_groupID < NUM_GROUPS);
            RivalGroup* desired_groupPtr = &m_groups[group.m_groupID];
#if USE_ATOMIC_RIVAL_PTR
            if (m_currentGroupPtr.load(std::memory_order_relaxed) == desired_groupPtr)
#else
            if (m_currentGroupPtr == desired_groupPtr)
#endif
            {
                // If already held by the same rival group, try add to program counter
                if (m_programCounter.fetch_add(1, std::memory_order_relaxed) > 0)
                {
                    return;
                }
                // But if UnLock happens after 'if (m_currentGroupPtr == groupPtr)' and before above fetch_add statement,
                // There is a chance that fetch_add returns 0, which allow other rival group to compete for lock, so we need to compete for the lock as 
                // And Before competing for the lock, we need to reset m_programCounter by subtracting 1
                {
                    m_programCounter.fetch_add(-1, std::memory_order_relaxed);
                }
            }

            SET_DEADLOCK_TIMER();
#if USE_ATOMIC_RIVAL_PTR
            RivalGroup* current_held = nullptr;
            while (!m_currentGroupPtr.compare_exchange_weak(current_held, desired_groupPtr) && current_held != desired_groupPtr)
            {
                int spin_count = SPIN_COUNT;
                do
                {
                    current_held = nullptr;
                    THREAD_PAUSE();
                } while ((!m_currentGroupPtr.compare_exchange_weak(current_held, desired_groupPtr) && current_held != desired_groupPtr) && --spin_count);
                if (spin_count)
                {
                    break;
                }
                ASSERT_DEADLOCK_TIMER();
                current_held = nullptr;
                THREAD_YIELD();
            }
#else
            auto _address = (void**)&m_currentGroupPtr;
            // TODO @yuhang : InterlockedCompareExchangePointer is limited to windows platform, implement a generic platform interface for this
            while (InterlockedCompareExchangePointer(_address, desired_groupPtr, nullptr) != desired_groupPtr)
            {
                int spin_count = SPIN_COUNT;
                do
                {
                    THREAD_PAUSE();
                }
                while ((InterlockedCompareExchangePointer(_address, desired_groupPtr, nullptr) != desired_groupPtr) && --spin_count);
                if (spin_count)
                {
                    break;
                }
                ASSERT_DEADLOCK_TIMER();
                THREAD_YIELD();
            }
#endif
            m_programCounter.fetch_add(1, std::memory_order_relaxed);
        }

        void UnLock()
        {
            if (m_programCounter.fetch_add(-1, std::memory_order_relaxed) == 1)
            {
#if USE_ATOMIC_RIVAL_PTR
                m_currentGroupPtr.store(nullptr, std::memory_order_relaxed);
#else
                m_currentGroupPtr = nullptr;
#endif
            }
        }

    private:
#if USE_ATOMIC_RIVAL_PTR
        CACHE_ALIGN std::atomic<RivalGroup*> m_currentGroupPtr{nullptr};
#else
        CACHE_ALIGN volatile RivalGroup* m_currentGroupPtr{nullptr};
#endif
        std::atomic_int_fast32_t m_programCounter{0};
        RivalGroup m_groups[NUM_GROUPS];
    };
    
    // Conditionally apply rival lock. If not enabled, rival lock would not take actual effect.
    template <int ARG_NUM_GROUPS>
    struct rival_lock_guard_cond
    {
        NONCOPYABLE(rival_lock_guard_cond);
        rival_lock_guard_cond() = delete;

        explicit rival_lock_guard_cond(RivalLock<ARG_NUM_GROUPS>& flag, RivalGroup group, bool enabled = true)
            :
            m_lock(&flag),
            m_enabled(enabled)
        {
            if (m_enabled)
            {
                m_lock->Lock(group);
            }
        }

        ~rival_lock_guard_cond()
        {
            if (m_enabled)
            {
                m_lock->UnLock();
            }
        }

    private:
        RivalLock<ARG_NUM_GROUPS>* m_lock;
        bool m_enabled;
    };
#define LOCK_GUARD_RIVAL(lock, group, ...) rival_lock_guard_cond<(lock).NUM_GROUPS> __lock_rival_cond(lock, group, __VA_ARGS__);ATOMIC_THREAD_FENCE
}

#undef USE_ATOMIC_RIVAL_PTR
#undef DEADLOCK_TIMER
#undef SET_DEADLOCK_TIMER
#undef ASSERT_DEADLOCK_TIMER

#undef SPIN_COUNT
#undef THREAD_PAUSE
#undef THREAD_YIELD
