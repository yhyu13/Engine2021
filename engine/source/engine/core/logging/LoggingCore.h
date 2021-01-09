#pragma once

#include "Logger.h"

using namespace AAAAgames;

//////// ENGINE LOGGING ////////
#define ENGINE_TRACE(...)			Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)			Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ENGINE_DEBUG(...)			Logger::GetEngineLogger()->debug(__VA_ARGS__)
#define ENGINE_WARN(...)			Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)			Logger::GetEngineLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...)		Logger::GetEngineLogger()->critical(__VA_ARGS__)

//////// APPLICATION LOGGING ////////
#define APP_TRACE(...)		Logger::GetApplicationLogger()->trace(__VA_ARGS__)
#define APP_INFO(...)		Logger::GetApplicationLogger()->info(__VA_ARGS__)
#define APP_DEBUG(...)		Logger::GetApplicationLogger()->debug(__VA_ARGS__)
#define APP_WARN(...)		Logger::GetApplicationLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...)		Logger::GetApplicationLogger()->error(__VA_ARGS__)
#define APP_CRITICAL(...)	Logger::GetApplicationLogger()->critical(__VA_ARGS__)