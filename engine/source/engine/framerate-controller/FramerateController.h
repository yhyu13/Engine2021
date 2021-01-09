#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/utility/Timer.h"

namespace AAAAgames {
	class ENGINE_API FramerateController {
	public:
		static FramerateController* GetInstance();
		void FrameStart();
		void FrameEnd();
		//!Returns the frame-time in seconds
		const double GetFrameTime() const;
		//!Returns the frame-time in seconds
		const double GetTargetFrameTime() const;
		void SetMaxFrameRate(unsigned int newMax);
	private:
		FramerateController();
	private:
		Timer m_timer;
		unsigned int m_maxFramerate;
		double m_tickStart;
		double m_tickEnd;

		double m_ticksPerFrame;
		double m_frameTime;
	};
}