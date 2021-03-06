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

	FramerateController::FramerateController() : m_tickEnd(0), m_ticksPerFrame(0.0f) 
	{
		m_maxFramerate = 60;
		m_ticksPerFrame = 1000.0 / static_cast<double>(m_maxFramerate);
		m_frameTime = m_ticksPerFrame * 1e-3;
	}

	void FramerateController::FrameStart() 
	{
		m_timer.Reset();
	}

	void FramerateController::FrameEnd() 
	{
		m_tickEnd = m_timer.Mark() * 1000.0; // converting seconds to milliseconds
		
		// Doing all three stages of waiting would safe CPU usage from 30% to less than 10% for a 8 cores PC
		// 1. sleep
		while (m_ticksPerFrame - m_tickEnd > 6)
		{
			// sleep until has roughly +/- 1 millisecond accuracy
			std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds{ 5 });
			m_tickEnd = m_timer.Mark() * 1000.0;
		}
		while (m_ticksPerFrame - m_tickEnd > 4)
		{
			// sleep until has roughly +/- 1 millisecond accuracy
			std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds{ 3 });
			m_tickEnd = m_timer.Mark() * 1000.0;
		}
		while (m_ticksPerFrame - m_tickEnd > 2)
		{
			// sleep until has roughly +/- 1 millisecond accuracy
			std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds{ 1 });
			m_tickEnd = m_timer.Mark() * 1000.0;
		}

		// 2. yield
		while (m_tickEnd < m_ticksPerFrame * .99)
		{
			// yield has roughly +/- 30 microsecond accuracy, it could work as a more accurate sleep function at fine grain.
			std::this_thread::yield();
			m_tickEnd = m_timer.Mark() * 1000.0; // converting seconds to milliseconds
		}

		// 3. busy wait
		do {
			m_tickEnd = m_timer.Mark() * 1000.0; // converting seconds to milliseconds
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
}