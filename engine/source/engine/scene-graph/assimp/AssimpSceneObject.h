#pragma once

#include "AssimpHelper.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/allocator/MemoryManager.h"

namespace longmarch
{
	class AssimpSceneObject
	{
	public:

		inline static std::shared_ptr<AssimpSceneObject> LoadFromFile(const fs::path& path)
		{
			return MemoryManager::Make_shared<AssimpSceneObject>(path);
		}

		AssimpSceneObject() = delete;
		explicit AssimpSceneObject(const fs::path& path);

		~AssimpSceneObject();

		const aiScene* GetScene() const noexcept;

		const aiNode* GetRoot() const noexcept;

	private:
		const aiScene* m_aiscene{ nullptr };
	};
}
