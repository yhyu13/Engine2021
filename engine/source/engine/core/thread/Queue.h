// Reference : https://github.com/mvorbrodt/blog/blob/master/src/queue.hpp
#pragma once
#include <mutex>
#include <queue>
#include <utility>
#include <type_traits>
#include <condition_variable>

namespace longmarch
{
    /*
     * Atomic queue, using mutex underneath
     */
    template <typename T>
    class blocking_queue
    {
    public:
        template <typename Q = T>
        typename std::enable_if<std::is_copy_constructible<Q>::value, void>::type
        push(const T& item)
        {
            {
                std::unique_lock lock(m_mutex);
                m_queue.push(item);
            }
            m_ready.notify_one();
        }

        template <typename Q = T>
        typename std::enable_if<std::is_move_constructible<Q>::value, void>::type
        push(T&& item)
        {
            {
                std::unique_lock lock(m_mutex);
                m_queue.emplace(std::forward<T>(item));
            }
            m_ready.notify_one();
        }

        template <typename Q = T>
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

        template <typename Q = T>
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

        template <typename Q = T>
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

        template <typename Q = T>
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

        template <typename Q = T>
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

        template <typename Q = T>
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
        mutable std::mutex m_mutex;
        std::condition_variable m_ready;
        std::queue<T> m_queue;
        bool m_done = false;
    };

    /*
     * Atomic queue, noncopyable, using spin lock underneath
     */
    template <typename T>
    class AtomicQueueNC final : private BaseAtomicClassNC
    {
    public:
        NONCOPYABLE(AtomicQueueNC);
        AtomicQueueNC() = default;

        void push(T&& value) noexcept
        {
            LOCK_GUARD_NC();
            m_queque.emplace_back(value);
        }

        void push(const T& value) noexcept
        {
            LOCK_GUARD_NC();
            m_queque.emplace_back(value);
        }

        T& front() noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.front();
        }

        const T& front() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.front();
        }

        void pop() noexcept
        {
            LOCK_GUARD_NC();
            m_queque.pop_front();
        }

        bool empty() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.empty();
        }

        bool contains(const T& value) const noexcept
        {
            LOCK_GUARD_NC();
            return std::find(m_queque.begin(), m_queque.end(), value) != m_queque.end();
        }

        size_t size() noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.size();
        }

        auto begin() noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.begin();
        }

        auto end() noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.end();
        }

        auto begin() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.begin();
        }

        auto end() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.end();
        }

        auto cbegin() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.cbegin();
        }

        auto cend() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queque.cend();
        }

    private:
        std::deque<T> m_queque;
    };
}
