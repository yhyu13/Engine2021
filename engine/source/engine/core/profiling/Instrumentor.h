#pragma once
#include <Remotery.h>

namespace longmarch 
{
	struct InstrumentorResult 
	{
		const char* m_name;
		double m_time;
		const char* m_timeUnit;
	};

	/*
		The profiler class splitted into engine side and applications side :
		ENG_TIME("Something")
		APP_TIME("Something")
	*/
	class Instrumentor 
	{
	public:
		static Instrumentor* GetEngineInstance();
		static Instrumentor* GetApplicationInstance();
		~Instrumentor();

		void BeginSession();
		void EndSession();
		void AddInstrumentorResult(const InstrumentorResult& result);
		std::map<std::string, InstrumentorResult>& GetResults();

	private:
		Instrumentor();
		std::map<std::string, InstrumentorResult> m_results;
	};

	class RemoteryInstrumentor
	{
	public:
		static RemoteryInstrumentor* GetInstance();
		~RemoteryInstrumentor();

	public:
		RemoteryInstrumentor();
		Remotery* m_rmt_instance{ nullptr };
	};
}
