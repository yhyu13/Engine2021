#include "engine-precompiled-header.h"
#include "engine/core/utility/Timer.h"
#include "Lock.h"

#define DEBUG_TIMER 1

namespace longmarch
{
	std::atomic_flag stbi_hdr_write_lock;
	std::atomic_flag stbi_png_write_lock;
}

longmarch::atomic_flag_guard::atomic_flag_guard(std::atomic_flag& flag) noexcept
	:
	m_lock(&flag)
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	while (m_lock->test_and_set(std::memory_order_acq_rel))
	{
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (m_lock->test(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

longmarch::atomic_flag_guard::~atomic_flag_guard() noexcept
{
	m_lock->clear(std::memory_order_release);
}

longmarch::atomic_bool_guard::atomic_bool_guard(std::atomic_bool& flag) noexcept
	:
	m_lock(&flag)
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	bool expected = false;
	while (!m_lock->compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
		expected = false;
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (m_lock->load(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

longmarch::atomic_bool_guard::~atomic_bool_guard() noexcept
{
	m_lock->store(false, std::memory_order_release);
}

void longmarch::BaseAtomicClassStatic::LockS() noexcept
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	while (sc_flag.test_and_set(std::memory_order_acq_rel))
	{
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (sc_flag.test(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

void longmarch::BaseAtomicClassStatic::UnlockS() noexcept
{
	sc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNI::LockNI() noexcept
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	while (ni_flag.test_and_set(std::memory_order_acq_rel))
	{
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (ni_flag.test(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

void longmarch::BaseAtomicClassNI::UnlockNI() noexcept
{
	ni_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClassNC::LockNC() const noexcept
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	while (nc_flag.test_and_set(std::memory_order_acq_rel))
	{
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (nc_flag.test(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

void longmarch::BaseAtomicClassNC::UnlockNC() const noexcept
{
	nc_flag.clear(std::memory_order_release);
}

void longmarch::BaseAtomicClass2::Lock2() const noexcept
{
#if DEBUG_TIMER == 1
	Timer timer;
#endif
	while (m_flag.test_and_set(std::memory_order_acq_rel))
	{
#if DEBUG_TIMER == 1
		ASSERT(timer.Mark() < 1.0, "Dead lock?");
#else
		while (m_flag.test(std::memory_order_relaxed))
		{
			std::this_thread::yield();
		}
#endif
	}
}

void longmarch::BaseAtomicClass2::Unlock2() const noexcept
{
	m_flag.clear(std::memory_order_release);
}