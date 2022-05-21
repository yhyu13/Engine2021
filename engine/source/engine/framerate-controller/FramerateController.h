#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/utility/Timer.h"

namespace longmarch
{
    class ENGINE_API FramerateController
    {
    public:
        static FramerateController* GetInstance();
        void FrameStart();
        void FrameEnd();
        //!Returns the frame-time in seconds
        const double GetFrameTime() const;
        //!Returns the frame-time in seconds
        const double GetTargetFrameTime() const;
        void SetMaxFrameRate(unsigned int newMax);
        //! Enable high precision mode to obtain more stable framerate at the cost of higher CPU usage
        void SetHighPrecisionMode(bool enable);
    private:
        FramerateController();
    private:
        Timer m_timer;
        unsigned int m_maxFramerate;
        double m_tickStart;
        double m_tickEnd;

        double m_targetFrameTimeMilli; // this value equals 1000 / m_maxFramerate
        double m_frameTimeSec;

        // High precision mode would disable the attempt to thread sleep
        bool m_highPrecisionMode;
    };
}
