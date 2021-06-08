#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/exception/EngineException.h"

#define VK_TRY_CATCH(x, error, message) try { x; } catch (error err) { ENGINE_EXCEPT(str2wstr(Str("[vulkan] Exception: %s. %s", err.what(), message))); }
#define VK_CHECK_ERROR(x, message) if (auto result = x; result != 0) { ENGINE_EXCEPT(str2wstr(Str("[vulkan] Error: VkResult = %d. %s", result, message))); }

