#include "engine-precompiled-header.h"
#include "FramerateController.h"
#include "engine/core/profiling/InstrumentorCore.h"

#include <GLFW/glfw3.h>

namespace AAAAgames {
	/*
		FramerateController will always have only one instance throughout the execution of the application.
		Default max frame rate is 60fps.
	*/
	FramerateController* FramerateController::GetInstance() {
		static FramerateController frameRateController;
		return &frameRateController;
	}

	FramerateController::FramerateController() : m_tickStart(0), m_tickEnd(0), m_ticksPerFrame(0.0f) {
		m_maxFramerate = 60;
		m_ticksPerFrame = 1000.0 / static_cast<double>(m_maxFramerate);
		m_frameTime = m_ticksPerFrame * 1e-3;
	}

	void FramerateController::FrameStart() {
		m_timer.Reset();
		m_tickStart = 0.0;// m_timer.Mark() * 1000.0; // m_timer.Mark() returns time in seconds passed
	}

	void FramerateController::FrameEnd() {
		do {
			// yield to do other work while there is time left over
			// It works well in practice, way better than sleep
			std::this_thread::yield();
			m_tickEnd = m_timer.Mark() * 1000.0; // converting seconds to milliseconds
		} while ((m_tickEnd - m_tickStart) < m_ticksPerFrame);
		double m_frameTick = (m_tickEnd - m_tickStart);
		//#ifdef _DEBUG
		Instrumentor::GetEngineInstance()->AddInstrumentorResult({ "Frame Time", m_frameTick, "ms" });
		Instrumentor::GetEngineInstance()->AddInstrumentorResult({ "FIRST_PERSON", 1000.0 / m_frameTick, "  " });
		Instrumentor::GetApplicationInstance()->AddInstrumentorResult({ "Frame Time", m_frameTick, "ms" });
		Instrumentor::GetApplicationInstance()->AddInstrumentorResult({ "FIRST_PERSON", 1000.0 / m_frameTick, "  " });
		//#endif
		m_frameTime = m_frameTick * 1e-3; // converting milliseconds to seconds
	}

	const double FramerateController::GetFrameTime() const {
		return m_frameTime;
	}

	const double AAAAgames::FramerateController::GetTargetFrameTime() const
	{
		return m_ticksPerFrame * 1e-3;;
	}

	void AAAAgames::FramerateController::SetMaxFrameRate(unsigned int newMax)
	{
		m_maxFramerate = newMax;
		m_ticksPerFrame = 1000.0 / static_cast<double>(m_maxFramerate);
		m_frameTime = m_ticksPerFrame * 1e-3;
	}
}