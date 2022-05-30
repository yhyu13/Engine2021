#pragma once

// Reference: https://github.com/mvorbrodt/blog/blob/master/src/pool.hpp
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "ThreadUtil.h"
#include "Lock.h"
#include "Queue.h"

namespace longmarch
{
	class StealThreadPool
	{
	public:
		NONCOPYABLE(StealThreadPool);

		static StealThreadPool* GetInstance()
		{
			static StealThreadPool pool;
			return &pool;
		}

		explicit StealThreadPool(unsigned int threads = std::thread::hardware_concurrency());
		~StealThreadPool();

		template<typename F, typename... Args>
		void enqueue_work(F&& f, Args&&... args)
		{
			auto work = [p = std::forward<F>(f), t = std::make_tuple(std::forward<Args>(args)...)]() { std::apply(p, t); };
			const auto i = (m_index = ++m_index % m_count);

			for (auto n(0u); n < m_count * K; ++n)
			{
				if (m_queues[(i + n) % m_count].try_push(work))
				{
					return;
				}
			}

			m_queues[i % m_count].push(std::move(work));
		}

		template<typename F, typename... Args>
		[[nodiscard]] auto enqueue_task(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
		{
			using task_return_type = std::invoke_result_t<F, Args...>;
			using task_type = std::packaged_task<task_return_type()>;

			auto task = std::make_shared<task_type>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
			auto result = task->get_future();
			auto work = [task = std::move(task)]() { (*task)(); };
			const auto i = (m_index = ++m_index % m_count);

			for (auto n(0u); n < m_count * K; ++n)
			{
				if (m_queues[(i + n) % m_count].try_push(work))
				{
					return result;
				}
			}
			m_queues[i % m_count].push(std::move(work));
			return result;
		}
	public:
		const unsigned int threads;
	private:
		using Proc = std::function<void(void)>;
		using Queue = blocking_queue<Proc>;
		using Queues = std::vector<Queue>;
		using Thread = std::thread;
		using Threads = std::vector<Thread>;

		Queues m_queues;
		Threads m_threads;
		std::atomic_uint m_index = { 0u };
		const unsigned int m_count;
		constexpr inline static const unsigned int K = { 2u };
	};
}
