#pragma once

#include <chrono>

//#include "Instrumentor.h"

namespace longmarch {
	class Instrumentor;

	/*
		InstrumentingTimer class follows RAII pattern.
	*/
	class InstrumentingTimer {
	public:

		InstrumentingTimer(const char* name, Instrumentor* instrumentor);
		~InstrumentingTimer();

		void End();

	protected:

	private:

		const char* m_name;
		bool m_stopped;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_timeBegin;
		Instrumentor* m_instumentor;
	};
}
