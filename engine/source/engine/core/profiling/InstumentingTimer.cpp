#include "engine-precompiled-header.h"
#include "InstumentingTimer.h"
#include "Instrumentor.h"

namespace AAAAgames {
	InstrumentingTimer::InstrumentingTimer(const char* name, Instrumentor* instrumentor) :m_name(name), m_stopped(false), m_instumentor(instrumentor) {
		m_timeBegin = std::chrono::high_resolution_clock::now();
	}

	InstrumentingTimer::~InstrumentingTimer() {
		if (!m_stopped) {
			End();
		}
	}

	void InstrumentingTimer::End() {
		decltype(auto) timeEnd = std::chrono::high_resolution_clock::now();

		decltype(auto) begin = std::chrono::time_point_cast<std::chrono::microseconds>(m_timeBegin).time_since_epoch().count();
		decltype(auto) end = std::chrono::time_point_cast<std::chrono::microseconds>(timeEnd).time_since_epoch().count();
		decltype(auto) duration = end - begin;
		decltype(auto) timeInMilliSeconds = duration * 0.001;

		m_stopped = true;

		m_instumentor->AddInstrumentorResult({ m_name, timeInMilliSeconds, "ms" });
	}
}