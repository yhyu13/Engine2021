#pragma once

#include <map>
#include <vector>
#include <functional>
#include <spdlog/spdlog.h>

namespace longmarch 
{
	class Logger 
	{
	public:

		Logger() = delete; ~Logger() = delete; Logger(const Logger&) = delete; Logger(const Logger&&) = delete; \
			Logger& operator=(const Logger&) = delete; Logger& operator=(const Logger&&) = delete;

		static void Init();
		static void ShutDown();

		inline static spdlog::logger* GetEngineLogger() { return s_engineLogger.get(); }
		inline static spdlog::logger* GetEngineErrorLogger() { return s_engineErrorLogger.get(); }
		inline static spdlog::logger* GetApplicationLogger() { return s_applicationLogger.get(); }
		inline static spdlog::logger* GetApplicationErrorLogger() { return s_applicationErrorLogger.get(); }

	public:
		inline static bool init = { false };

	private:
		inline static std::shared_ptr<spdlog::logger> s_engineLogger = { nullptr }; // Could be an async logger
		inline static std::shared_ptr<spdlog::logger> s_engineErrorLogger = { nullptr }; // Sync logger
		inline static std::shared_ptr<spdlog::logger> s_applicationLogger = { nullptr }; // Could be an async logger
		inline static std::shared_ptr<spdlog::logger> s_applicationErrorLogger = { nullptr }; // Sync logger
	};
}