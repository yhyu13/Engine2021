// Reference: https://raw.githubusercontent.com/progschj/ThreadPool/master/ThreadPool.h

#pragma once

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
#include "../EngineCore.h"

namespace longmarch
{
    class ThreadPool
    {
    public:
        NONCOPYABLE(ThreadPool);

        static ThreadPool* GetInstance()
        {
            static ThreadPool pool;
            return &pool;
        }

        explicit ThreadPool(unsigned int threads = std::thread::hardware_concurrency() - 1);
        ~ThreadPool();

        // // add new task item to the pool
        // template <class F, class ...Args>
        // void enqueue_work(F&& f, Args&& ...args)
        // {
        //     auto work = [p = std::forward<F>(f), t = std::make_tuple(std::forward<Args>(args)...)]()
        //     {
        //         std::apply(p, t);
        //     };
        //     {
        //         std::unique_lock<std::mutex> lock(queue_mutex);
        //         // don't allow enqueueing after stopping the pool
        //         if (stop)
        //         {
        //             throw std::runtime_error("enqueue on stopped ThreadPool");
        //         }
        //         tasks.emplace(work);
        //     }
        //     condition.notify_one();
        // }

        // add new task item to the pool
        template <class F, class ...Args>
        [[nodiscard]] auto enqueue_task(F&& f, Args&& ...args) -> std::future<std::invoke_result_t<F, Args...>>
        {
            using task_return_type = std::invoke_result_t<F, Args...>;
            using task_type = std::packaged_task<task_return_type()>;

            auto task = std::make_shared<task_type>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto result = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                // don't allow enqueueing after stopping the pool
                if (stop)
                {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }
                tasks.emplace([task = std::move(task)]() { (*task)(); });
            }
            condition.notify_one();
            return result;
        }

    public:
        const unsigned int threads;
    private:
        using Proc = std::function<void(void)>;
        using Queue = std::queue<Proc>;
        using Thread = std::thread;
        using Threads = std::vector<Thread>;

        Threads workers;
        Queue tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };
}
