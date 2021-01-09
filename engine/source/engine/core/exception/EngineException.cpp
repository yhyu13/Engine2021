#include "engine-precompiled-header.h"
#include "EngineException.h"

namespace longmarch
{
    AtomicQueue<EngineException> EngineException::m_queue;
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
	throw e;
}

void longmarch::EngineException::Update()
{
	while (!m_queue.empty())
	{
		throw m_queue.front();
		m_queue.pop();
	}
}

std::wstring longmarch::str2wstr(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string longmarch::wstr2str(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}
