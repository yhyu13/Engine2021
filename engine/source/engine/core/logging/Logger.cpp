#include "engine-precompiled-header.h"
#include "Logger.h"
#include "TerminalLogger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace AAAAgames {
	void Logger::Init() {
		if (!init)
		{
			spdlog::init_thread_pool(8192, 1);
			spdlog::set_pattern("%^[%T] %n: %v%$");

			auto imTermSink = TerminalLogger::GetInstance()->get_terminal_helper();
			{
				auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
				auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("eng_log.txt", 1024 * 1024 * 10, 3, false);
				std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink, imTermSink };
				s_engineLogger = std::make_shared<spdlog::async_logger>("ENGINE", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
				s_engineLogger->set_level(spdlog::level::trace);
				spdlog::register_logger(s_engineLogger);
			}

			{
				auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
				auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("app_log.txt", 1024 * 1024 * 10, 3, false);
				std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink, imTermSink };
				s_applicationLogger = std::make_shared<spdlog::async_logger>("APPLICATION", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
				s_applicationLogger->set_level(spdlog::level::trace);
				spdlog::register_logger(s_applicationLogger);
			}
			init = true;
		}
	}

	void Logger::ShutDown()
	{
		spdlog::drop("ENGINE");
		spdlog::drop("APPLICATION");
		s_engineLogger.reset();
		s_applicationLogger.reset();
		init = false;
	}
}