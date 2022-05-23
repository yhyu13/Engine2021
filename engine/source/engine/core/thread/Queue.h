// Reference : https://github.com/mvorbrodt/blog/blob/master/src/queue.hpp
#pragma once
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <condition_variable>
#include "Semaphore.h"

namespace longmarch
{
	template<typename T>
	class blocking_queue
	{
	public:
		template<typename Q = T>
		typename std::enable_if<std::is_copy_constructible<Q>::value, void>::type
			push(const T& item)
		{
			{
				std::unique_lock lock(m_mutex);
				m_queue.push(item);
			}
			m_ready.notify_one();
		}

		template<typename Q = T>
		typename std::enable_if<std::is_move_constructible<Q>::value, void>::type
			push(T&& item)
		{
			{
				std::unique_lock lock(m_mutex);
				m_queue.emplace(std::forward<T>(item));
			}
			m_ready.notify_one();
		}

		template<typename Q = T>
		typename std::enable_if<std::is_copy_constructible<Q>::value, bool>::type
			try_push(const T& item)
		{
			{
				std::unique_lock lock(m_mutex, std::try_to_lock);
				if (!lock)
					return false;
				m_queue.push(item);
			}
			m_ready.notify_one();
			return true;
		}

		template<typename Q = T>
		typename std::enable_if<std::is_move_constructible<Q>::value, bool>::type
			try_push(T&& item)
		{
			{
				std::unique_lock lock(m_mutex, std::try_to_lock);
				if (!lock)
					return false;
				m_queue.emplace(std::forward<T>(item));
			}
			m_ready.notify_one();
			return true;
		}

		template<typename Q = T>
		typename std::enable_if<
			std::is_copy_assignable<Q>::value &&
			!std::is_move_assignable<Q>::value, bool>::type
			pop(T& item)
		{
			std::unique_lock lock(m_mutex);
			while (m_queue.empty() && !m_done)
				m_ready.wait(lock);
			if (m_queue.empty())
				return false;
			item = m_queue.front();
			m_queue.pop();
			return true;
		}

		template<typename Q = T>
		typename std::enable_if<std::is_move_assignable<Q>::value, bool>::type
			pop(T& item)
		{
			std::unique_lock lock(m_mutex);
			while (m_queue.empty() && !m_done)
				m_ready.wait(lock);
			if (m_queue.empty())
				return false;
			item = std::move(m_queue.front());
			m_queue.pop();
			return true;
		}

		template<typename Q = T>
		typename std::enable_if<
			std::is_copy_assignable<Q>::value &&
			!std::is_move_assignable<Q>::value, bool>::type
			try_pop(T& item)
		{
			std::unique_lock lock(m_mutex, std::try_to_lock);
			if (!lock || m_queue.empty())
				return false;
			item = m_queue.front();
			m_queue.pop();
			return true;
		}

		template<typename Q = T>
		typename std::enable_if<std::is_move_assignable<Q>::value, bool>::type
			try_pop(T& item)
		{
			std::unique_lock lock(m_mutex, std::try_to_lock);
			if (!lock || m_queue.empty())
				return false;
			item = std::move(m_queue.front());
			m_queue.pop();
			return true;
		}

		void done() noexcept
		{
			{
				std::unique_lock lock(m_mutex);
				m_done = true;
			}
			m_ready.notify_all();
		}

		bool empty() const noexcept
		{
			std::scoped_lock lock(m_mutex);
			return m_queue.empty();
		}

		unsigned int size() const noexcept
		{
			std::scoped_lock lock(m_mutex);
			return m_queue.size();
		}

	private:
		CACHE_ALIGN64 mutable std::mutex m_mutex;
		CACHE_ALIGN64 std::condition_variable m_ready;
		std::queue<T> m_queue;
		bool m_done = false;
	};
}
