#include "engine-precompiled-header.h"
#include "engine/core/utility/Timer.h"
#include "Lock.h"

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
    // define extern variables
    std::atomic_flag stbi_hdr_write_lock;
    std::atomic_flag stbi_png_write_lock;
}

longmarch::atomic_flag_guard::atomic_flag_guard(std::atomic_flag& flag) noexcept
    :
    m_lock(&flag)
{
    SET_DEADLOCK_TIMER();
    while (m_lock->test_and_set(std::memory_order_acq_rel))
    {
        ASSERT_DEADLOCK_TIMER();
        while (m_lock->test(std::memory_order_relaxed))
        {
            ASSERT_DEADLOCK_TIMER();
            THREAD_YIELD();
        }
    }
}

longmarch::atomic_flag_guard::~atomic_flag_guard() noexcept
{
    m_lock->clear(std::memory_order_release);
}

longmarch::adaptive_atomic_guard::adaptive_atomic_guard(std::mutex& flag, int64_t& period) noexcept
    :
    m_lock(&flag),
    m_period_nano(&period)
{
    SET_DEADLOCK_TIMER();
    Timer _timer;
    int64_t _marked_nano = 0ull;
    const auto _period_nano = *m_period_nano * 2;
    while (!m_lock->try_lock())
    {
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
        _marked_nano = _timer.Mark<std::nano, int64_t>();
        if (_marked_nano >= _period_nano)
        {
            m_lock->lock();
            break;
        }
    }
    *m_period_nano += (_marked_nano - *m_period_nano) / 2;
}

longmarch::adaptive_atomic_guard::~adaptive_atomic_guard() noexcept
{
    m_lock->unlock();
}

void longmarch::BaseAtomicClassStatic::LockS() noexcept
{
    SET_DEADLOCK_TIMER();
    while (sc_flag.test_and_set(std::memory_order_acq_rel))
    {
        ASSERT_DEADLOCK_TIMER();
        while (sc_flag.test(std::memory_order_relaxed))
        {
            ASSERT_DEADLOCK_TIMER();
            THREAD_YIELD();
        }
    }
}

void longmarch::BaseAtomicClassStatic::UnlockS() noexcept
{
    sc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNI::LockNI() noexcept
{
    SET_DEADLOCK_TIMER();
    while (ni_flag.test_and_set(std::memory_order_acq_rel))
    {
        ASSERT_DEADLOCK_TIMER();
        while (ni_flag.test(std::memory_order_relaxed))
        {
            ASSERT_DEADLOCK_TIMER();
            THREAD_YIELD();
        }
    }
}

void longmarch::BaseAtomicClassNI::UnlockNI() noexcept
{
    ni_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNC::LockNC() const noexcept
{
    SET_DEADLOCK_TIMER();
    while (nc_flag.test_and_set(std::memory_order_acq_rel))
    {
        ASSERT_DEADLOCK_TIMER();
        while (nc_flag.test(std::memory_order_relaxed))
        {
            ASSERT_DEADLOCK_TIMER();
            THREAD_YIELD();
        }
    }
}

void longmarch::BaseAtomicClassNC::UnlockNC() const noexcept
{
    nc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClass::Lock() const noexcept
{
    SET_DEADLOCK_TIMER();
    while (m_flag.test_and_set(std::memory_order_acq_rel))
    {
        ASSERT_DEADLOCK_TIMER();
        while (m_flag.test(std::memory_order_relaxed))
        {
            ASSERT_DEADLOCK_TIMER();
            THREAD_YIELD();
        }
    }
}

void longmarch::BaseAtomicClass::UnLock() const noexcept
{
    m_flag.clear(std::memory_order_release);
}

void AdaptiveAtomicClassNC::LockAdaptiveNC() const noexcept
{
    SET_DEADLOCK_TIMER();
    Timer _timer;
    int64_t _marked_nano = 0ull;
    const auto _period_nano = nc_period_nano * 2;
    while (!nc_mutex.try_lock())
    {
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
        _marked_nano = _timer.Mark<std::nano, int64_t>();
        if (_marked_nano >= _period_nano)
        {
            nc_mutex.lock();
            break;
        }
    }
    nc_period_nano += (_marked_nano - nc_period_nano) / 2;
}

void AdaptiveAtomicClassNC::UnlockAdaptiveNC() const noexcept
{
    nc_mutex.unlock();
}
