#include "engine-precompiled-header.h"
#include "EngineException.h"

namespace longmarch
{
    AtomicQueueNC<EngineException> EngineException::m_queue;
}

const std::wstring& longmarch::EngineException::GetNote() const
{
    return note;
}

const std::wstring& longmarch::EngineException::GetFile() const
{
    return file;
}

unsigned int longmarch::EngineException::GetLine() const
{
    return line;
}

std::wstring longmarch::EngineException::GetLocation() const
{
    return std::wstring( L"Line [" ) + std::to_wstring( line ) + L"] in " + file;
}

std::wstring longmarch::EngineException::GetFullMessage() const
{
#if 1
    return GetNote() + L"\nAt: " + GetLocation();
#else
	return GetNote();
#endif
}

std::wstring longmarch::EngineException::GetExceptionType() const
{
    return exceptionType;
}

void longmarch::EngineException::Push(EngineException&& e)
{
    m_queue.push(e);
    ASSERT(false, "EngineException::Push throw!");
	throw e;
}

void longmarch::EngineException::Update()
{
	while (!m_queue.empty())
	{
	    ASSERT(false, "EngineException::Update throw!");
		throw m_queue.pop_front();
	}
}
