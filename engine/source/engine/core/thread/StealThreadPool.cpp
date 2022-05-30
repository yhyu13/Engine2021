#include "engine-precompiled-header.h"
#include "StealThreadPool.h"

longmarch::StealThreadPool::StealThreadPool(unsigned int threads)
    :
    threads(threads),
    m_queues(threads),
    m_count(threads)
{
    if (!threads)
    {
        throw std::invalid_argument("Invalid thread count!");
    }
    auto worker = [this](auto i)
    {
        while (true)
        {
            Proc task;
            for (auto n(0u); n < m_count * K; ++n)
            {
                if (m_queues[(i + n) % m_count].try_pop(task))
                {
                    break;
                }
            }
            if (!task && !m_queues[i].pop(task))
            {
                break;
            }
            task();
        }
    };
    m_threads.reserve(threads);
    for (auto i(0u); i < threads; ++i)
    {
        m_threads.emplace_back(worker, i);
    }
}

longmarch::StealThreadPool::~StealThreadPool()
{
    for (auto& queue : m_queues)
    {
        queue.done();
    }
    for (auto& thread : m_threads)
    {
        thread.join();
    }
}
