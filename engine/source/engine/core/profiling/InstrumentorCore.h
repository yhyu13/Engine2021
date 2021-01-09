#pragma once

#include "Instrumentor.h"
#include "InstumentingTimer.h"

#ifdef _DEBUG
#define ENG_TIME(name) InstrumentingTimer timer##__LINE__(name, AAAAgames::Instrumentor::GetEngineInstance())
#else
#define ENG_TIME(name) InstrumentingTimer timer##__LINE__(name, AAAAgames::Instrumentor::GetEngineInstance())
#endif

#ifdef _DEBUG
#define APP_TIME(name) InstrumentingTimer timer##__LINE__(name, AAAAgames::Instrumentor::GetApplicationInstance())
#else
#define APP_TIME(name) InstrumentingTimer timer##__LINE__(name, AAAAgames::Instrumentor::GetApplicationInstance())
#endif
