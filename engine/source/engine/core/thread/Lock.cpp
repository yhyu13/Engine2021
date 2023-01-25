#include "engine-precompiled-header.h"
#include "engine/core/utility/Timer.h"
#include "Lock.h"

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
        int spin_count = SPIN_COUNT;
        do
        {
            THREAD_PAUSE();
        }
        while (m_lock->test_and_set(std::memory_order_acq_rel) && --spin_count);
        if (spin_count)
        {
            break;
        }
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
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
    Timer timer;
    int64_t _marked_nano = 0ull;
    const auto _period_nano = *m_period_nano * 2;
    while (!m_lock->try_lock())
    {
        THREAD_PAUSE();
        _marked_nano = timer.Mark<std::nano, int64_t>();
        if (_marked_nano >= _period_nano)
        {
            m_lock->lock();
            break;
        }
        ASSERT_DEADLOCK_TIMER();
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
        int spin_count = SPIN_COUNT;
        do
        {
            THREAD_PAUSE();
        }
        while (sc_flag.test_and_set(std::memory_order_acq_rel) && --spin_count);
        if (spin_count)
        {
            break;
        }
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
    }
}

void longmarch::BaseAtomicClassStatic::UnLockS() noexcept
{
    sc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNI::LockNI() noexcept
{
    SET_DEADLOCK_TIMER();
    while (ni_flag.test_and_set(std::memory_order_acq_rel))
    {
        int spin_count = SPIN_COUNT;
        do
        {
            THREAD_PAUSE();
        }
        while (ni_flag.test_and_set(std::memory_order_acq_rel) && --spin_count);
        if (spin_count)
        {
            break;
        }
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
    }
}

void longmarch::BaseAtomicClassNI::UnLockNI() noexcept
{
    ni_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNC::LockNC() const noexcept
{
    SET_DEADLOCK_TIMER();
    while (nc_flag.test_and_set(std::memory_order_acq_rel))
    {
        int spin_count = SPIN_COUNT;
        do
        {
            THREAD_PAUSE();
        }
        while (nc_flag.test_and_set(std::memory_order_acq_rel) && --spin_count);
        if (spin_count)
        {
            break;
        }
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
    }
}

void longmarch::BaseAtomicClassNC::UnLockNC() const noexcept
{
    nc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClass::Lock() const noexcept
{
    SET_DEADLOCK_TIMER();
    while (m_flag.test_and_set(std::memory_order_acq_rel))
    {
        int spin_count = SPIN_COUNT;
        do
        {
            THREAD_PAUSE();
        }
        while (m_flag.test_and_set(std::memory_order_acq_rel) && --spin_count);
        if (spin_count)
        {
            break;
        }
        ASSERT_DEADLOCK_TIMER();
        THREAD_YIELD();
    }
}

void longmarch::BaseAtomicClass::UnLock() const noexcept
{
    m_flag.clear(std::memory_order_release);
}

void AdaptiveAtomicClassNC::LockAdaptiveNC() const noexcept
{
    SET_DEADLOCK_TIMER();
    Timer timer;
    int64_t _marked_nano = 0ull;
    const auto _period_nano = nc_period_nano * 2;
    while (!nc_mutex.try_lock())
    {
        THREAD_PAUSE();
        _marked_nano = timer.Mark<std::nano, int64_t>();
        if (_marked_nano >= _period_nano)
        {
            nc_mutex.lock();
            break;
        }
        ASSERT_DEADLOCK_TIMER();
    }
    nc_period_nano += (_marked_nano - nc_period_nano) / 2;
}

void AdaptiveAtomicClassNC::UnLockAdaptiveNC() const noexcept
{
    nc_mutex.unlock();
}
