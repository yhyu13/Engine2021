#include "engine-precompiled-header.h"
#include "InstumentingTimer.h"
#include "Instrumentor.h"

namespace longmarch 
{
	InstrumentingTimer::InstrumentingTimer(const char* name, Instrumentor* instrumentor) 
		:
		m_name(name), 
		m_stopped(false), 
		m_instumentor(instrumentor) 
	{
		m_timeBegin = std::chrono::high_resolution_clock::now();
	}

	InstrumentingTimer::~InstrumentingTimer() 
	{
		if (!m_stopped) 
		{
			End();
		}
	}

	void InstrumentingTimer::End() 
	{
		m_stopped = true;
		auto timeEnd = std::chrono::high_resolution_clock::now();

		auto begin = std::chrono::time_point_cast<std::chrono::microseconds>(m_timeBegin).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(timeEnd).time_since_epoch().count();
		auto duration = end - begin;
		auto timeInMilliSeconds = duration * 0.001;

		m_instumentor->AddInstrumentorResult({ m_name, timeInMilliSeconds, "ms" });
	}
}