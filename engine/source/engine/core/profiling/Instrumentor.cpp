#include "engine-precompiled-header.h"
#include "Instrumentor.h"

namespace longmarch 
{
	Instrumentor::Instrumentor() 
	{
	}

	Instrumentor::~Instrumentor() 
	{
	}

	Instrumentor* Instrumentor::GetEngineInstance() 
	{
		static Instrumentor instance;
		return &instance;
	}

	Instrumentor* Instrumentor::GetApplicationInstance()
	{
		static Instrumentor instance;
		return &instance;
	}

	void Instrumentor::BeginSession() 
	{
	}

	void Instrumentor::EndSession() 
	{
	}

	void Instrumentor::AddInstrumentorResult(const InstrumentorResult& result) 
	{
		auto itr = m_results.find(result.m_name);
		if (itr != m_results.end()) 
		{
			(*itr).second.m_time = result.m_time;
		}
		else 
		{
			m_results.insert({ result.m_name , result });
		}
	}

	std::map<const char*, InstrumentorResult>& Instrumentor::GetResults() 
	{
		return m_results;
	}
	
	Remotery* RemoteryInstrumentor::GetInstance()
	{
		static Remotery* rmt;
		static std::once_flag flag;
		std::call_once(flag, []() { rmt_CreateGlobalInstance(&rmt); });
		return rmt;
	}
}