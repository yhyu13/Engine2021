#pragma once
#include "engine/renderer/animation/3d/Animation3D.h"
#include "engine/renderer/mesh/Mesh.h"
#include "engine/scene-graph/assimp/AssimpHelper.h"

namespace AAAAgames
{
	class Scene3DManager;
	struct Animation3DCom;

	/**
	 * @brief Representation of a scene node in the overall scene (a scene node could be consistent of multiple mesh, animations and skeletons)
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class Scene3DNode
	{
		friend Scene3DManager;
		friend Animation3DCom;

	private:
		using MeshHierarchy = A4GAMES_Vector<std::pair<uint32_t, std::shared_ptr<Mesh>>>; //!< In-order hierarchy tree of all mesh in a scene node, stands for a mesh and its height in the tree

	public:
		Scene3DNode() = default;
		Scene3DNode(std::string_view _name)
			:
			sceneNodeName(_name)
		{}

		//! Copy a scene3DNode is preferred with new copies of all materials
		std::shared_ptr<Scene3DNode> Copy() const;

		const A4GAMES_Vector<MeshData*> GetAllMesh() const;

		const A4GAMES_Vector<Material*> GetAllMaterial() const;

		void ModifyAllMaterial(const std::function<void(Material*)>& callback) const;

		void SetInverseFinalBoneTransform(const Skeleton::Bone_Transform_LUT& inverseFinal);

		const Skeleton::Bone_Transform_LUT& GetInverseFinalBoneTransform() const;

		std::string Name() const { return sceneNodeName; }
		auto empty() const { return meshTree.empty(); }
		auto begin() { return meshTree.begin(); }
		auto end() { return meshTree.end(); }
		auto begin() const { return meshTree.begin(); }
		auto end() const { return meshTree.end(); }
		auto cbegin() const { return meshTree.cbegin(); }
		auto cend() const { return meshTree.cend(); }

	private:
		std::string sceneNodeName{ "None" };
		MeshHierarchy meshTree; //!< In-order hierarchy tree of all mesh in a scene node, stands for a mesh and its height in the tree

		struct AnimationData
		{
			Skeleton::Bone_Transform_LUT bone_inverseFinalTransform_LUT; //!< the inverse transform matrix at animation pose, calculated every frame
		}animationData;
	};
}