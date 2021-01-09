// Reference https://vorbrodt.blog/2019/02/25/better-timer-class/

#pragma once

#include <thread>
#include <chrono>
#include <memory>
#include <functional>
#include <set>
#include <iterator>
#include <cassert>
#include "event.h"
#include "Lock.h"
#include "../utility/TypeHelper.h"

namespace AAAAgames
{
	class Scheduler : BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(Scheduler);
		using SchedulerHandle = std::shared_ptr<manual_event>;

		static Scheduler* GetMilliSecInstance(uint32_t _time)
		{
			if (auto it = s_intanceManager.find(_time); it != s_intanceManager.end())
			{
				return (it->second.get());
			}
			else
			{
				s_intanceManager[_time] = std::move(MemoryManager::Make_unique<Scheduler>(std::chrono::milliseconds(_time)));
				return s_intanceManager[_time].get();
			}
		}

		template<typename T>
		Scheduler(T&& tick)
			:
			m_tick(std::chrono::duration_cast<std::chrono::nanoseconds>(tick)),
			m_thread([this]()
		{
			assert(m_tick.count() > 0);
			auto start = std::chrono::high_resolution_clock::now();
			std::chrono::nanoseconds drift{ 0 };
			while (!m_event.wait_for(m_tick - drift))
			{
				{
					LOCK_GUARD_NC();
					++m_ticks;
					auto it = std::begin(m_events);
					auto end = std::end(m_events);
					while (it != end)
					{
						if (auto event = *it; ++event.elapsed == event.ticks)
						{
							if (auto remove = event.proc(); remove)
							{
								m_events.erase(it++);
								continue;
							}
							else
							{
								event.elapsed = 0;
							}
						}
						++it;
					}
				}
				std::this_thread::yield();
				auto now = std::chrono::high_resolution_clock::now();
				auto realDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
				auto fakeDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(m_tick * m_ticks);
				drift = realDuration - fakeDuration;
			}
		})
		{}

		~Scheduler()
		{
			m_event.signal();
			m_thread.join();
		}

		template<typename T, typename F, typename... Args>
		[[nodiscard]] SchedulerHandle set_timeout(T&& timeout, F f, Args&&... args)
		{
			LOCK_GUARD_NC();
			assert(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count() >= m_tick.count());
			auto _event = std::make_shared<manual_event>();
			auto proc = [=]() {
				if (_event->wait_for(std::chrono::seconds(0))) return true;
				f(args...);
				return true;
			};
			m_events.insert({ event_ctx::kNextSeqNum++, proc,
				static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count() / m_tick.count()), 0, _event });
			return _event;
		}

		template<typename T, typename F, typename... Args>
		[[nodiscard]] SchedulerHandle set_interval(T&& interval, F f, Args&&... args)
		{
			LOCK_GUARD_NC();
			assert(std::chrono::duration_cast<std::chrono::nanoseconds>(interval).count() >= m_tick.count());
			auto _event = std::make_shared<manual_event>();
			auto proc = [=]() {
				if (_event->wait_for(std::chrono::seconds(0))) return true;
				f(args...);
				return false;
			};
			m_events.insert({ event_ctx::kNextSeqNum++, proc,
				static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(interval).count() / m_tick.count()), 0, _event });
			return _event;
		}

	private:
		std::chrono::nanoseconds m_tick;
		unsigned long long m_ticks = 0;
		manual_event m_event;
		std::thread m_thread;

		struct event_ctx
		{
			bool operator < (const event_ctx& rhs) const { return seq_num < rhs.seq_num; }
			static inline unsigned long long kNextSeqNum = 0;
			unsigned long long seq_num;
			std::function<bool(void)> proc;
			unsigned long long ticks;
			mutable unsigned long long elapsed;
			std::shared_ptr<manual_event> event;
		};

		using EventSet = std::set<event_ctx>; // Should not use phmap's block set because iterators are invalidated on erase
		EventSet m_events;
	private:
		inline static A4GAMES_UnorderedMap<uint32_t, A4GAMES_Unique_ptr<Scheduler>> s_intanceManager;
	};
}