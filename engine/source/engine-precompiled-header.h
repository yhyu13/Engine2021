#pragma once

#if defined(WINDOWS_APP)
#define NOMINMAX // NOMINMAX for WINDOWS
#define _ENABLE_EXTENDED_ALIGNED_STORAGE // VS2017 15.8 fix on aligned allocation (for phmap to work)
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
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <concepts>
#include <ranges> // cpp 20

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <math.h>
#include <filesystem>

// GL Math
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_INLINE
#define GLM_FORCE_AVX2
#define GLM_RIGHT_HANDED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp >
#include <glm/gtc/random.hpp>
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

// Blaze Math
#define BLAZE_MPI_PARALLEL_MODE 1
#include <blaze/Blaze.h>

// qu3e Simple AABB Physics
#include <qu3e/q3.h>

// Parrallel hash map and Block-Tree
#include <phmap/phmap.h>
#include <phmap/btree.h>

// Lua binding
#include <lua.hpp>
#include <sol/sol.hpp>

// Fast BVH
#include <FastBVH.h>

namespace AAAAgames
{
	namespace fs = std::filesystem;
}