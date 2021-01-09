#pragma once

#include "Instrumentor.h"
#include "InstumentingTimer.h"

#ifdef _DEBUG
#define ENG_TIME(name) InstrumentingTimer timer##__LINE__(name, longmarch::Instrumentor::GetEngineInstance())
#else
#define ENG_TIME(name) InstrumentingTimer timer##__LINE__(name, longmarch::Instrumentor::GetEngineInstance())
#endif

#ifdef _DEBUG
#define APP_TIME(name) InstrumentingTimer timer##__LINE__(name, longmarch::Instrumentor::GetApplicationInstance())
#else
#define APP_TIME(name) InstrumentingTimer timer##__LINE__(name, longmarch::Instrumentor::GetApplicationInstance())
#endif
