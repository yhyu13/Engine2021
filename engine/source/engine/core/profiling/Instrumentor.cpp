#include "engine-precompiled-header.h"
#include "Instrumentor.h"

//! If use Nvidia Nsight, turn remotery off
//#define USE_REMOTERY_PROFILING

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
	
	RemoteryInstrumentor* RemoteryInstrumentor::GetInstance()
	{
		static RemoteryInstrumentor rmt;
		return &rmt;
	}

	RemoteryInstrumentor::RemoteryInstrumentor()
	{
#ifdef USE_REMOTERY_PROFILING
		rmt_CreateGlobalInstance(&m_rmt_instance);
		rmt_BindOpenGL(); // Remotery is built with OpenGL backend
#endif
	}

	RemoteryInstrumentor::~RemoteryInstrumentor()
	{
#ifdef USE_REMOTERY_PROFILING
		rmt_DestroyGlobalInstance(m_rmt_instance); 
#endif
	}
}