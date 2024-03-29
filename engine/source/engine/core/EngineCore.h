#pragma once
#include <sstream>
#if defined(WCHAR_UTF8_CHAR_CONV)
#include <locale>
#include <codecvt>
#endif

#include "engine/core/logging/LoggingCore.h"

#if defined(WIN32) || defined(WINDOWS_APP)
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define MS_ALIGN8 __declspec(align(8))
#define MS_ALIGN16 __declspec(align(16))
#define MS_ALIGN32 __declspec(align(32))
#define MS_ALIGN64 __declspec(align(64))

#if defined(WIN32) || defined(WINDOWS_APP)
#define PLATFORM_CACHE_LINE 64
#define CACHE_ALIGN __declspec(align(PLATFORM_CACHE_LINE))
#else
#define PLATFORM_CACHE_LINE 128
#define CACHE_ALIGN alignas(PLATFORM_CACHE_LINE)
#endif

#define TWO_5 32
#define TWO_6 64
#define TWO_7 128
#define TWO_8 256
#define TWO_9 512
#define TWO_10 1024
#define TWO_11 2048
#define TWO_12 4096
#define TWO_16 65536

#define __COMBINE_(X,Y) X##Y  // yuhang : mostly used for combine variable name with line number (e.g. COMBINE(x, __LINE__))
#define COMBINE(X,Y) __COMBINE_(X,Y)

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
#define ASSERT(x, ...) do { if(!(x)) { if (Logger::init) ENGINE_CRITICAL("Assertion failed: {2} at {0} : {1}", __FILE__, __LINE__, std::string(##__VA_ARGS__)); DEBUG_BREAK(); } } while (0)
#else
    #if 1
    #define ASSERT(x, ...) do { if(!(x)) { if (Logger::init) ENGINE_CRITICAL("Assertion failed: {2} at {0} : {1}", __FILE__, __LINE__, std::string(##__VA_ARGS__)); DEBUG_BREAK(); } } while (0)
    #else
    #define ASSERT(x, ...)
    #endif // DEBUG
#endif

#ifndef _SHIPPING
#define CRITICAL_PRINT(...) do { if (Logger::init) ENGINE_CRITICAL(__VA_ARGS__); DEBUG_BREAK(); } while (0)
#define ERROR_PRINT(...) do { if (Logger::init) ENGINE_ERROR(__VA_ARGS__); DEBUG_BREAK(); } while (0)
#define WARN_PRINT(...) do { if (Logger::init) ENGINE_WARN(__VA_ARGS__); } while (0)
#define DEBUG_PRINT(...) do { if (Logger::init) ENGINE_DEBUG(__VA_ARGS__); } while (0)
#define PRINT(...) do { if (Logger::init) ENGINE_INFO(__VA_ARGS__); } while (0)
#else
#define CRITICAL_PRINT(...) do { if (Logger::init) ENGINE_CRITICAL(__VA_ARGS__); } while (0)
#define ERROR_PRINT(...) do { if (Logger::init) ENGINE_ERROR(__VA_ARGS__); } while (0)
#define WARN_PRINT(...) do { if (Logger::init) ENGINE_WARN(__VA_ARGS__); } while (0)
#define DEBUG_PRINT(...) 
#define PRINT(...) 
#endif // DEBUG

template <class T>
std::wstring wStr(const T& t)
{
    std::wstringstream ss;
    ss << t;
    return ss.str();
}

template <class T>
std::string Str(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <typename... Args>
std::string Str(const char* fmt, const Args&... args)
{
    char str[1024];
    _snprintf_s(str, 1024, fmt, args...);
    return std::string(str);
}

template <typename... Args>
std::wstring wStr(const wchar_t* fmt, const Args&... args)
{
    wchar_t str[1024];
    _snwprintf_s(str, 1024, fmt, args...);
    return std::wstring(str);
}

template <typename ...__TRIVIAL__>
std::wstring wStr(const std::string& str)
{
#if defined(WCHAR_UTF8_CHAR_CONV)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.from_bytes(str);
#else
    return std::wstring(str.begin(), str.end());
#endif
}

template <typename ...__TRIVIAL__>
std::string Str(const std::wstring& wStr)
{
#if defined(WCHAR_UTF8_CHAR_CONV)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.to_bytes(wStr);
#else
    return std::string(wStr.begin(), wStr.end());
#endif
}
