#pragma once

#include "Logger.h"

using namespace longmarch;

//////// ENGINE LOGGING ////////
#define ENGINE_TRACE(...)			Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)			Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ENGINE_DEBUG(...)			Logger::GetEngineLogger()->debug(__VA_ARGS__)
#define ENGINE_WARN(...)			Logger::GetEngineErrorLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)			Logger::GetEngineErrorLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...)		Logger::GetEngineErrorLogger()->critical(__VA_ARGS__)

//////// APPLICATION LOGGING ////////
#define APP_TRACE(...)		Logger::GetApplicationLogger()->trace(__VA_ARGS__)
#define APP_INFO(...)		Logger::GetApplicationLogger()->info(__VA_ARGS__)
#define APP_DEBUG(...)		Logger::GetApplicationLogger()->debug(__VA_ARGS__)
#define APP_WARN(...)		Logger::GetApplicationErrorLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...)		Logger::GetApplicationErrorLogger()->error(__VA_ARGS__)
#define APP_CRITICAL(...)	Logger::GetApplicationErrorLogger()->critical(__VA_ARGS__)