#include "engine-precompiled-header.h"
#include "ThreadPool.h"

// the constructor just launches some amount of workers

AAAAgames::ThreadPool::ThreadPool(unsigned int threads)
	:
	threads(threads),
	stop(false)
{
	if (!threads)
	{
		throw std::invalid_argument("Invalid thread count!");
	}
	workers.reserve(threads);
	for (auto i(0u); i < threads; ++i)
	{
		workers.emplace_back(
			[this]
		{
			while (true)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
			}
		}
		);
	}
}

// the destructor joins all threads

AAAAgames::ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (auto& worker : workers)
	{
		worker.join();
	}
}