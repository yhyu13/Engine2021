#pragma once

#if defined(WIN32) || defined(WINDOWS_APP)
#define NOMINMAX // NOMINMAX for WINDOWS
#define _ENABLE_EXTENDED_ALIGNED_STORAGE // VS2017 15.8 fix on aligned allocation (for phmap to work)
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <regex>

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <math.h>
#include <filesystem>

// Header-only externals are also included in the pch

// GL Math
//#define GLM_FORCE_MESSAGES
#define GLM_FORCE_CXX2A
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_AVX2
#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL Binding
#include <glad/glad.h>

// Assimp scene file loader
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

// JsonCPP
#include <json/json.h>

// stbi image loader
#include <stb_image.h>
#include <stb_image_write.h>

// Parrallel hash map and Block-Tree
#include <phmap/phmap.h>
#include <phmap/btree.h>

// SOL2 Lua binding
#include <lua.hpp>
#include <sol/sol.hpp>

// Fast BVH for physics and raycasting
#include <FastBVH.h>

// minizip wrapper
#include <miniz-cpp.hpp>

namespace longmarch
{
	namespace fs = std::filesystem;
}