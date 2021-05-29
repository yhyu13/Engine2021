#include "engine-precompiled-header.h"
#include "FramerateController.h"
#include "engine/core/profiling/InstrumentorCore.h"

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
		m_ticksPerFrame(0.0f),
		m_highPrecisionMode(false)
	{
		m_ticksPerFrame = 1000.0 / static_cast<double>(m_maxFramerate);
		m_frameTime = m_ticksPerFrame * 1e-3;
	}

	void FramerateController::FrameStart() 
	{
		m_timer.Reset();
	}

	void FramerateController::FrameEnd()
	{
		m_tickEnd = m_timer.Mark<double, std::milli>();

		// Doing all three stages of waiting would safe CPU usage from 30% to less than 10% for a 8 cores PC
		// 1. sleep
		if (!m_highPrecisionMode)
		{
			int k = static_cast<int>(m_ticksPerFrame);
			while (--k >= 2)
			{
				while (m_ticksPerFrame - m_tickEnd > static_cast<double>(k))
				{
					std::this_thread::sleep_for(std::chrono::milliseconds{ k - 1 });
					m_tickEnd = m_timer.Mark<double, std::milli>();
				}
			}
		}

		// 2. yield
		while (m_tickEnd < m_ticksPerFrame * .99)
		{
			// yield has roughly +/- 30 microsecond accuracy, it could work as a more accurate sleep function at fine grain.
			std::this_thread::yield();
			m_tickEnd = m_timer.Mark<double, std::milli>();
		}

		// 3. busy wait
		do {
			m_tickEnd = m_timer.Mark<double, std::milli>();
		} while (m_tickEnd < m_ticksPerFrame * .9999);

		m_frameTime = m_tickEnd * 1e-3; // converting milliseconds to seconds

#ifndef _SHIPPING
		Instrumentor::GetEngineInstance()->AddInstrumentorResult({ "Frame Time", m_tickEnd, "ms" });
		Instrumentor::GetEngineInstance()->AddInstrumentorResult({ "FPS", 1.0 / m_frameTime, "  " });
		Instrumentor::GetApplicationInstance()->AddInstrumentorResult({ "Frame Time", m_tickEnd, "ms" });
		Instrumentor::GetApplicationInstance()->AddInstrumentorResult({ "FPS", 1.0 / m_frameTime, "  " });
#endif
	}

	const double FramerateController::GetFrameTime() const 
	{
		return m_frameTime;
	}

	const double longmarch::FramerateController::GetTargetFrameTime() const
	{
		return m_ticksPerFrame * 1e-3;;
	}

	void longmarch::FramerateController::SetMaxFrameRate(unsigned int newMax)
	{
		m_maxFramerate = newMax;
		m_ticksPerFrame = 1000.0 / static_cast<double>(m_maxFramerate);
		m_frameTime = m_ticksPerFrame * 1e-3;
	}
	void longmarch::FramerateController::SetHighPrecisionMode(bool enable)
	{
		m_highPrecisionMode = enable;
	}
}