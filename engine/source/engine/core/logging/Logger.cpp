#include "engine-precompiled-header.h"
#include "Logger.h"
#include "TerminalLogger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#ifdef _ENABLE_ASYNC_LOGGER // Project-wise macro defined in premake script
#define USE_ASYNC_LOGGER 1
#else
#define USE_ASYNC_LOGGER 1 // Use async logger or not up to you
#endif // ENABLE_ASYNC_LOGGER

namespace longmarch 
{
	void Logger::Init() 
	{
		if (!init)
		{
#if USE_ASYNC_LOGGER == 1
			spdlog::init_thread_pool(8192, 1);
#endif
			spdlog::set_pattern("%^[%T] %n: %v%$");

			auto imTermSink = TerminalLogger::GetInstance()->get_terminal_helper();
			{
				auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
				auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("eng_log.txt", 1024 * 1024 * 10, 3, false);
				std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink, imTermSink };
#if USE_ASYNC_LOGGER == 1
				s_engineLogger = std::make_shared<spdlog::async_logger>("ENGINE", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
#else
				s_engineLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
#endif
				s_engineLogger->set_level(spdlog::level::trace);
				spdlog::register_logger(s_engineLogger);

				s_engineErrorLogger = std::make_shared<spdlog::logger>("ENGINE_ERR", sinks.begin(), sinks.end());
				s_engineErrorLogger->set_level(spdlog::level::warn);
				spdlog::register_logger(s_engineErrorLogger);
			}

			{
				auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
				auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("app_log.txt", 1024 * 1024 * 10, 3, false);
				std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink, imTermSink };
#if USE_ASYNC_LOGGER == 1
				s_applicationLogger = std::make_shared<spdlog::async_logger>("APPLICATION", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
#else
				s_applicationLogger = std::make_shared<spdlog::logger>("APPLICATION", sinks.begin(), sinks.end());
#endif
				s_applicationLogger->set_level(spdlog::level::trace);
				spdlog::register_logger(s_applicationLogger);

				s_applicationErrorLogger = std::make_shared<spdlog::logger>("APPLICATION_ERR", sinks.begin(), sinks.end());
				s_applicationErrorLogger->set_level(spdlog::level::warn);
				spdlog::register_logger(s_applicationErrorLogger);
			}
			init = true;
		}
	}

	void Logger::ShutDown()
	{
		if (init)
		{
			spdlog::drop("ENGINE");
			spdlog::drop("ENGINE_ERR");
			spdlog::drop("APPLICATION");
			spdlog::drop("APPLICATION_ERR");
			s_engineLogger = nullptr;
			s_engineErrorLogger = nullptr;
			s_applicationLogger = nullptr;
			s_applicationErrorLogger = nullptr;
			init = false;
		}
	}
}