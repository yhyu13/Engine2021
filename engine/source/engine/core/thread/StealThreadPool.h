#pragma once

// Reference: https://github.com/mvorbrodt/blog/blob/master/src/pool.hpp
#include <vector>
#include <memory>
#include <thread>
#include <future>
#include <functional>
#include <stdexcept>
#include "Queue.h"

namespace longmarch
{
    class StealThreadPool : public BaseAtomicClassStatic
    {
    public:
        NONCOPYABLE(StealThreadPool);

        static StealThreadPool* GetInstance()
        {
            static StealThreadPool pool;
            return &pool;
        }

        explicit StealThreadPool(int threads = std::thread::hardware_concurrency() - 1);
        ~StealThreadPool();

        // template <typename F, typename... Args>
        // void enqueue_work(F&& f, Args&&... args)
        // {
        //     auto work = [p = std::forward<F>(f), t = std::make_tuple(std::forward<Args>(args)...)]()
        //     {
        //         std::apply(p, t);
        //     };
        //     const auto i = (m_index.fetch_add(1, std::memory_order_relaxed) % m_count);
        //
        //     for (auto n(0u); n < m_count * K; ++n)
        //     {
        //         if (m_queues[(i + n) % m_count].try_push(work))
        //         {
        //             return;
        //         }
        //     }
        //
        //     m_queues[i % m_count].push(std::move(work));
        // }

        template <typename F, typename... Args>
        [[nodiscard]] auto enqueue_task(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
        {
            using task_return_type = std::invoke_result_t<F, Args...>;
            using task_type = std::packaged_task<task_return_type()>;

            auto task = std::make_shared<task_type>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto result = task->get_future();
            auto work = [task = std::move(task)]() { (*task)(); };
            const auto i = (m_index.fetch_add(1, std::memory_order_relaxed) % m_count);

            for (auto n(0); n < m_count * K; ++n)
            {
                if (m_queues[(i + n) % m_count].try_push(work, true))
                {
                    return result;
                }
            }
            m_queues[i].push(std::move(work));
            return result;
        }

    public:
        static void ResetStats();

        static void ThreadReportWait(uint32_t t_id, uint32_t worker_id, double time);
        static void ThreadReportExec(uint32_t t_id, uint32_t worker_id, double time);

        static void ReportStats();

    private:
        inline static std::unordered_map<uint64_t, double> s_threadWaitMap;
        inline static std::unordered_map<uint64_t, double> s_threadExecMap;
        
    public:
        int threads;

    private:
        using Proc = std::function<void(void)>;
        using Queue = blocking_queue<Proc>;
        using Queues = std::vector<Queue>;
        using Thread = std::thread;
        using Threads = std::vector<Thread>;

        constexpr inline static int K = {2};

        Queues m_queues;
        Threads m_threads;
        int m_count;
        std::atomic_int_fast32_t m_index = {0};
    };
}
