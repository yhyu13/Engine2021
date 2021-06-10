#pragma once
#include "../EngineCore.h"
#include "../thread/Lock.h"
#include <string>
#include <exception>

namespace longmarch
{
#define ENGINE_EXCEPT(ARG) EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, ARG))
#define ENGINE_EXCEPT_IF(COND, ARG) if (COND) EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, ARG))

#define ENGINE_TRY_CATCH(X) try { X } \
	catch (EngineException& e) { EngineException::Push(std::move(e)); } \
	catch (std::exception& e) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception")); } \
	catch (...) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception")); } 

	class ENGINE_API EngineException
	{
	public:
		EngineException() = delete;
		/*
			example :
			ENGINE_EXCEPT(ARG);
			ENGINE_EXCEPT_IF(COND, ARG);
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"", L"Engine Exception");
		*/
		explicit EngineException(const wchar_t* file, unsigned int line, const std::wstring& note = L"", const std::wstring& type = L"Engine Exception")
			:
			file(file), //_CRT_WIDE(__FILE__)
			line(line), //__LINE__,
			note(note),  //L"Error!"
			exceptionType(type) //L"Engine Exception"
		{}
		const std::wstring& GetNote() const;
		const std::wstring& GetFile() const;
		unsigned int GetLine() const;
		std::wstring GetLocation() const;
		virtual std::wstring GetFullMessage() const;
		virtual std::wstring GetExceptionType() const;

		static void Push(EngineException&& e);
		static void Update();

	private:
		std::wstring exceptionType;
		std::wstring note;
		std::wstring file;
		unsigned int line;

		//! You should not use inline static for template classes
		static AtomicQueue<EngineException> m_queue;
	};

	class NotImplementedException : public std::logic_error
	{
	public:
		NotImplementedException() 
			: 
			std::logic_error("Function not yet implemented!") 
		{}
	};

	std::wstring str2wstr(const std::string& str);
	std::string wstr2str(const std::wstring& wstr);
}
