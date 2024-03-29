#include "engine-precompiled-header.h"
#include "FramerateController.h"
#include "engine/core/profiling/InstrumentorCore.h"
#include "engine/core/utility/Timer.h"

#if defined(WIN32) || defined(WINDOWS_APP)
#include <timeapi.h>
#endif

namespace longmarch
{
    /*
        FramerateController will always have only one instance throughout the execution of the application.
        Default max frame rate is 60fps.
    */
    FramerateController* FramerateController::GetInstance()
    {
        static FramerateController frameRateController;
        return &frameRateController;
    }

    FramerateController::FramerateController()
        :
        m_maxFramerate(60),
        m_tickStart(0),
        m_tickEnd(0),
        m_targetFrameTimeMilli(0.0f),
        m_highPrecisionMode(true)
    {
        m_targetFrameTimeMilli = 1000.0 / static_cast<double>(m_maxFramerate);
        m_frameTimeSec = m_targetFrameTimeMilli * 1e-3;
    }

    void FramerateController::FrameStart()
    {
        m_timer.Reset();
    }

    void FramerateController::FrameEnd()
    {
        m_tickEnd = m_timer.Mark<std::milli, double>();

        // Doing all 3 stages of waiting would safe total CPU usage from 30% to less than 10% for on a 8 cores PC
        // 1. thread sleep has roughly +/- 1 millisecond accuracy
        if (!m_highPrecisionMode)
        {
            auto k = static_cast<int>(m_targetFrameTimeMilli);
            while (--k > 2)
            {
                while (m_targetFrameTimeMilli - m_tickEnd > static_cast<double>(k))
                {
                    // yuhang : timeBeginPeriod/timeEndPeriod is windows only,
#if defined(WIN32) || defined(WINDOWS_APP)
                    timeBeginPeriod(1);
#endif
                    std::this_thread::sleep_for(std::chrono::milliseconds{k - 1});
#if defined(WIN32) || defined(WINDOWS_APP)
                    timeEndPeriod(1);
#endif
                    m_tickEnd = m_timer.Mark<std::milli, double>();
                }
            }
        }

        // 2. thread yield has roughly +/- 30 microsecond accuracy, it could work as a more accurate sleep function at fine grain.
        while (m_tickEnd < m_targetFrameTimeMilli * .995)
        {
            std::this_thread::yield();
            m_tickEnd = m_timer.Mark<std::milli, double>();
        }


#ifndef _SHIPPING
        Instrumentor::GetEngineInstance()->AddInstrumentorResult({"Frame Time", m_tickEnd, "ms"});
        Instrumentor::GetEngineInstance()->AddInstrumentorResult({"FPS", 1.0 / m_frameTimeSec, "  "});
        Instrumentor::GetApplicationInstance()->AddInstrumentorResult({"Frame Time", m_tickEnd, "ms"});
        Instrumentor::GetApplicationInstance()->AddInstrumentorResult({"FPS", 1.0 / m_frameTimeSec, "  "});
#endif

        // 3. busy wait
        do
        {
            m_tickEnd = m_timer.Mark<std::milli, double>();
        }
        while (m_tickEnd < m_targetFrameTimeMilli * .99995);

        m_frameTimeSec = m_tickEnd * 1e-3; // converting milliseconds to seconds
    }

    const double FramerateController::GetFrameTime() const
    {
        return m_frameTimeSec;
    }

    const double longmarch::FramerateController::GetTargetFrameTime() const
    {
        return m_targetFrameTimeMilli * 1e-3;
    }

    void longmarch::FramerateController::SetMaxFrameRate(unsigned int newMax)
    {
        m_maxFramerate = newMax;
        m_targetFrameTimeMilli = 1000.0 / static_cast<double>(m_maxFramerate);
        m_frameTimeSec = m_targetFrameTimeMilli * 1e-3;
    }

    void longmarch::FramerateController::SetHighPrecisionMode(bool enable)
    {
        m_highPrecisionMode = enable;
    }

    bool FramerateController::GetHighPrecisionMode() const
    {
        return m_highPrecisionMode;
    }
}
