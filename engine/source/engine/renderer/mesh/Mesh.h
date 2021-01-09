#pragma once

#include "MeshData.h"
#include "../material/Material.h"

namespace AAAAgames
{
	struct Mesh
	{
	public:
		std::string meshName{ "None" };
		std::shared_ptr<MeshData> meshData{ nullptr };
		std::shared_ptr<Material> material{ nullptr };

	public:
		Mesh() = default;
		explicit Mesh(const std::string& _name)
			:
			meshName(_name)
		{}

		//! Copy a Mesh is preferred with new copies of all materials
		inline std::shared_ptr<Mesh> Copy() const
		{
			auto ret = MemoryManager::Make_shared<Mesh>(meshName);
			ret->meshData = meshData;
			// Make a copy of material as we will often need to modify settings of mateirals for each individual entity
			// We don't make a copy of mesh as we will never need to modify it for each individual entity
			ret->material = MemoryManager::Make_shared<Material>(*material);
			return ret;
		}

		inline void Draw()
		{
			meshData->Draw();
		}
	};
}