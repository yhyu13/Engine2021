#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/exception/EngineException.h"

#define VK_TRY_CATCH(x, error, message) try { x; } catch (error err) { ENGINE_EXCEPT(str2wstr(Str("%s. %s", err.what(), message))); }

