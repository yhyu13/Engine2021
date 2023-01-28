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
        try_push(const T& item, bool only_push_on_empty = false)
        {
            {
                std::unique_lock lock(m_mutex, std::try_to_lock);
                if (!lock)
                    return false;
                if (!only_push_on_empty || m_queue.empty())
                    m_queue.push(item);
                else
                    return false;
            }
            m_ready.notify_one();
            return true;
        }

        template <typename Q = T>
        typename std::enable_if<std::is_move_constructible<Q>::value, bool>::type
        try_push(T&& item, bool only_push_on_empty = false)
        {
            {
                std::unique_lock lock(m_mutex, std::try_to_lock);
                if (!lock)
                    return false;
                if (!only_push_on_empty || m_queue.empty())
                    m_queue.emplace(std::forward<T>(item));
                else
                    return false;
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
            m_queue.emplace_back(value);
        }

        void push(const T& value) noexcept
        {
            LOCK_GUARD_NC();
            m_queue.emplace_back(value);
        }

        T& peek_front() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queue.front();
        }

        T pop_front() noexcept
        {
            LOCK_GUARD_NC();
            auto ret = m_queue.front();
            m_queue.pop_front();
            return ret;
        }

        bool empty() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queue.empty();
        }

        size_t size() const noexcept
        {
            LOCK_GUARD_NC();
            return m_queue.size();
        }

    private:
        std::deque<T> m_queue;
    };

    /**
     * Enumerates concurrent queue modes.
     */
    enum class EConcurrentQueueMode : uint8_t
    {
        Spsc,
        Mpsc,
        Mpmc,
        Spmc
    };
    
    /**
     * Noncopyable lock-free concurrent queue, copy from Unreal Engine's TQueue
     */
    template <typename T, EConcurrentQueueMode Mode = EConcurrentQueueMode::Spsc>
    class ConcurrentQueueNC
    {
        /*
         * 
         *  Head = Tail
         *
         *  Push 1
            OldHead1=null NewNode1=SomePtr
            Push 2
            OldHead2=null NewNode2=SomePtr

            Push 2 Interlock step1
            OldHead2 = Head = Tail
            Head = NewNode2

            Push 1 Interlock step1
            OldHead1 = Head = NewNode2
            Head = NewNode1

            Push 2 Interlock step2
            OldHead2->Next = Tail->Next = NewNode2
            
            Pop
            Tail->Next is poped
            delete Tail
            Tail = Tail->Next 
            Tail->Item = Empty

            Push 1 Interlock step2
            OldHead1->Next = NewNode2->Next = Tail->Next = NewNode1
         */
    private:
        /** Structure for the internal linked list. */
        struct QueueNode
        {
            QueueNode() = default;

            explicit QueueNode(const T& InItem)
                :
                m_item(InItem)
            {}

            explicit QueueNode(T&& InItem)
                :
                m_item(std::move(InItem))
            {}

            QueueNode* volatile m_nextNode{nullptr};
            T m_item;
        };
        
    public:
        NONCOPYABLE(ConcurrentQueueNC);
        ConcurrentQueueNC()
        {
            m_head = m_tail = new QueueNode();
        }

        ~ConcurrentQueueNC()
        {
            while(auto temp = m_tail)
            {
                m_tail = m_tail->m_nextNode;
                delete temp;
            }
        }

        void push(const T& item) noexcept
        {
            push_internal(new QueueNode(item));
        }

        void push(T&& item) noexcept
        {
            push_internal(new QueueNode(item));
        }

        T& peek_front() const noexcept
        {
            return m_tail->m_nextNode->m_item;
        }

        T pop_front() noexcept
        {
            T ret;
            if constexpr (Mode == EConcurrentQueueMode::Mpsc || Mode == EConcurrentQueueMode::Spsc)
            {
                ASSERT(pop_front(ret));
            }
            else
            {
                ASSERT(false, "Multi-consumer queue should not use this method as pop_front does not guarantee to succeed!");
            }
            return ret;
        }

        bool pop_front(T& ret) noexcept
        {
            if (auto poped_node = m_tail->m_nextNode)
            {
                if constexpr (Mode == EConcurrentQueueMode::Mpsc || Mode == EConcurrentQueueMode::Spsc)
                {
                    // Step1 swap tail pointer
                    auto old_tail = m_tail;
                    m_tail = poped_node;
                    
                    // Step2 assign value
                    ret = poped_node->m_item;

                    // Step3 delete old tail
                    delete old_tail;
                    
                    m_size.fetch_add(-1, std::memory_order_relaxed);
                    return true;
                }
                else
                {
#if defined(WIN32) || defined(WINDOWS_APP)
                    auto ptr_address = (void**)&m_tail;
                    auto old_tail = m_tail;
                    // Step1 swap tail pointer
                    if (old_tail->m_nextNode == poped_node
                        && old_tail == (QueueNode*)InterlockedCompareExchangePointer(ptr_address, poped_node, old_tail))
                    {
                        // Step2 assign value
                        ret = poped_node->m_item;
                        
                        // Step3 delete old tail
                        delete old_tail;
                        
                        m_size.fetch_add(-1, std::memory_order_relaxed);
                        return true;
                    }
#else
                    static_assert(false, "Not implemented yet");
#endif
                }
            }

            return false;
        }

        bool empty() const noexcept
        {
            return m_tail->m_nextNode == nullptr;
        }

        size_t size() const noexcept
        {
            return m_size.load(std::memory_order_relaxed);
        }

    private:
        void push_internal(QueueNode* NewNode) noexcept
        {
            QueueNode* old_head;
            if constexpr (Mode == EConcurrentQueueMode::Mpsc || Mode == EConcurrentQueueMode::Mpmc)
            {
#if defined(WIN32) || defined(WINDOWS_APP)
                // Step1, swap pointer
                old_head = (QueueNode*)InterlockedExchangePointer((void**)&m_head, NewNode);
                // Step2, chain pointer
                InterlockedExchangePointer((void**)&old_head->m_nextNode, NewNode);
#else
                static_assert(false, "Not implemented yet");
#endif
            }
            else
            {
                // Step1, swap pointer
                old_head = m_head;
                m_head = NewNode;

                // Step2, chain pointer
                // Prevent compiler reordering step2 into step1
                std::atomic_thread_fence(std::memory_order_release);
                old_head->m_nextNode = NewNode;
            }

            m_size.fetch_add(1, std::memory_order_relaxed);
        }
    
    private:
        /** Holds a pointer to the head of the list. */
        CACHE_ALIGN QueueNode* volatile m_head{nullptr};
        /** Holds a pointer to the tail of the list. */
        QueueNode* m_tail{nullptr};

        std::atomic_int32_t m_size{0};
    };
}
