#include "engine-precompiled-header.h"
#include "StealThreadPool.h"
#include "engine/core/utility/Timer.h"
#include "engine/delegate/engineDelegates/EngineDelegates.h"

longmarch::StealThreadPool::StealThreadPool(int threads)
    :
    threads(threads),
    m_queues(threads),
    m_count(threads)
{
    if (!threads)
    {
        throw std::invalid_argument("Invalid thread count!");
    }

    for (auto i(0u); i < threads; ++i)
    {
        m_threads.emplace_back(
            [this, i = i]
            {
                const auto t_id = std::this_thread::get_id();
                const auto id = *(uint32_t*)&(t_id);
                Timer timer;
                for (;;)
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
                    delegates::WorkerThreadReportWait.InvokeAll(id, i, timer.MarkMilli(true));
                    task();
                    delegates::WorkerThreadReportExec.InvokeAll(id, i, timer.MarkMilli(true));
                }
            }
        );
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

void StealThreadPool::ResetStats()
{
    LOCK_GUARD_S();
    s_threadWaitMap.clear();
    s_threadExecMap.clear();
}

void StealThreadPool::ThreadReportWait(uint32_t t_id, uint32_t worker_id, double time)
{
    LOCK_GUARD_S();
    uint64_t id = (uint64_t)t_id << 32 | (uint64_t)worker_id;
    s_threadWaitMap[id] += time;
}

void StealThreadPool::ThreadReportExec(uint32_t t_id, uint32_t worker_id, double time)
{
    LOCK_GUARD_S();
    uint64_t id = (uint64_t)t_id << 32 | (uint64_t)worker_id;
    s_threadExecMap[id] += time;
}

void StealThreadPool::ReportStats()
{
    LOCK_GUARD_S();
    // for(const auto [id, time] : s_threadWaitMap)
    // {
    //     uint32_t t_id = id >> 32;
    //     uint32_t worker_id = id & 0xFFFFFFFF;
    //     Instrumentor::GetEngineInstance()->AddInstrumentorResult({Str("Idle #%u worker %u", worker_id, t_id).c_str(),time, "ms"});
    // }
    for(const auto [id, time] : s_threadExecMap)
    {
        uint32_t t_id = id >> 32;
        uint32_t worker_id = id & 0xFFFFFFFF;
        Instrumentor::GetEngineInstance()->AddInstrumentorResult({Str("Busy #%u worker %u", worker_id, t_id).c_str(),time, "ms"});
        Instrumentor::GetApplicationInstance()->AddInstrumentorResult({Str("Busy #%u worker %u", worker_id, t_id).c_str(),time, "ms"});
    }
}
