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
		auto duration = std::chrono::duration<double>(timeEnd - m_timeBegin).count(); // in seconds
		auto timeInMilliSeconds = duration * 1000.0;
		m_instumentor->AddInstrumentorResult({ m_name, timeInMilliSeconds, "ms" });
	}
}