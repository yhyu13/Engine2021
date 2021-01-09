#include "engine-precompiled-header.h"
#include "EngineException.h"

namespace AAAAgames
{
    AtomicQueue<EngineException> EngineException::m_queue;
}

const std::wstring& AAAAgames::EngineException::GetNote() const
{
    return note;
}

const std::wstring& AAAAgames::EngineException::GetFile() const
{
    return file;
}

unsigned int AAAAgames::EngineException::GetLine() const
{
    return line;
}

std::wstring AAAAgames::EngineException::GetLocation() const
{
    return std::wstring( L"Line [" ) + std::to_wstring( line ) + L"] in " + file;
}

std::wstring AAAAgames::EngineException::GetFullMessage() const
{
#if 1
    return GetNote() + L"\nAt: " + GetLocation();
#else
	return GetNote();
#endif
}

std::wstring AAAAgames::EngineException::GetExceptionType() const
{
    return exceptionType;
}

void AAAAgames::EngineException::Push(EngineException&& e)
{
    m_queue.push(e);
	throw e;
}

void AAAAgames::EngineException::Update()
{
	while (!m_queue.empty())
	{
		throw m_queue.front();
		m_queue.pop();
	}
}

std::wstring AAAAgames::str2wstr(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string AAAAgames::wstr2str(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}
