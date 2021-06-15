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
		inline static spdlog::logger* GetApplicationLogger() { return s_applicationLogger.get(); }

	public:
		inline static bool init = { false };

	private:
		inline static std::shared_ptr<spdlog::logger> s_engineLogger = { nullptr };
		inline static std::shared_ptr<spdlog::logger> s_applicationLogger = { nullptr };
	};
}