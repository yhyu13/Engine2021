#pragma once
#include <sstream>
#include "engine/core/logging/LoggingCore.h"

#define CACHE_ALIGN8 __declspec(align(8))
#define CACHE_ALIGN16 __declspec(align(16))
#define CACHE_ALIGN32 __declspec(align(32))
#define CACHE_ALIGN64 __declspec(align(64))

#define TWO_5 32
#define TWO_6 64
#define TWO_7 128
#define TWO_8 256
#define TWO_9 512
#define TWO_10 1024
#define TWO_11 2048
#define TWO_12 4096
#define TWO_16 65536

#ifdef ENGINE_BUILD_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API
#endif

#define NONCOPYABLE(Class) Class( const Class& ) = delete; Class( const Class&& ) = delete; \
							Class& operator=(const Class&) = delete; Class& operator=(const Class&&) = delete;

#define NONINSTANTIABLE(Class) Class() = delete; ~Class() = delete; Class( const Class& ) = delete; Class( const Class&& ) = delete; \
							Class& operator=(const Class&) = delete; Class& operator=(const Class&&) = delete;

#ifndef _SHIPPING
#define ASSERT(x, ...) { if(!(x)) { if (Logger::init) ENGINE_CRITICAL("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define ASSERT(x, ...)
#endif // DEBUG

#ifndef _SHIPPING
#define ERROR_PRINT(...) { if (Logger::init) ENGINE_ERROR(__VA_ARGS__); }
#define DEBUG_PRINT(...) { if (Logger::init) ENGINE_DEBUG(__VA_ARGS__); }
#define PRINT(...) { if (Logger::init) ENGINE_INFO(__VA_ARGS__); }
#else
#define ERROR_PRINT(...) 
#define DEBUG_PRINT(...) 
#define PRINT(...) 
#endif // DEBUG

template<class T>
std::wstring wStr(const T& t)
{
	std::wstringstream ss;
	ss << t;
	return ss.str();
}

template<class T>
std::string Str(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

template<typename... Args>
std::string Str(const char* fmt, const Args &... args)
{
	char str[256];
	std::snprintf(str, 256, fmt, args...);
	return std::string(str);
}

template<typename... Args>
std::wstring wStr(const wchar_t* fmt, const Args &... args)
{
	wchar_t str[256];
	std::snprintf(str, 256, fmt, args...);
	return std::wstring(str);
}
