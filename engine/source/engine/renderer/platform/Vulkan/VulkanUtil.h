#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/exception/EngineException.h"

#ifndef _SHIPPING
#define VK_CHECK_RESULT(x, ...) { auto _ = x; ASSERT(_ == 0, Str("[Vulkan] Error: VkResult = %d. %s", _, __VA_ARGS__)); }
#else
#define VK_CHECK_RESULT(x, ...)
#endif

// Custom define for better code readability
#ifndef VK_FLAGS_NONE
#define VK_FLAGS_NONE 0
#endif // !VK_FLAGS_NONE

// Default fence timeout in nanoseconds
#ifndef VK_DEFAULT_FENCE_TIMEOUT
#define VK_DEFAULT_FENCE_TIMEOUT 100000000000
#endif // !VK_DEFAULT_FENCE_TIMEOUT

