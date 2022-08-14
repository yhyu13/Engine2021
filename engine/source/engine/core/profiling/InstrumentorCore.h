#pragma once

#include "Instrumentor.h"
#include "InstumentingTimer.h"

#ifndef _SHIPPING
#define ENG_TIME(name) InstrumentingTimer eng_timer##__LINE__(name, longmarch::Instrumentor::GetEngineInstance())
#else
#define ENG_TIME(name) 
#endif

#ifndef _SHIPPING
#define APP_TIME(name) InstrumentingTimer app_timer##__LINE__(name, longmarch::Instrumentor::GetApplicationInstance())
#else
#define APP_TIME(name) 
#endif

#ifndef _SHIPPING
#define GPU_TIME(name) {longmarch::RemoteryInstrumentor::GetInstance();} rmt_ScopedOpenGLSample(name)
#else
#define GPU_TIME(name) 
#endif
