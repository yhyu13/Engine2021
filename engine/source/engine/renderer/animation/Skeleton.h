#pragma once
#include "engine/scene-graph/assimp/AssimpHelper.h"
#include "engine/renderer/mesh/MeshData.h"
#include "engine/core/utility/TypeHelper.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	//! Entity agnostic skeleton class. Store bone info : transform matrices, name-index LUT, and skeleton hierarchy
	struct Skeleton
	{
		using Bone_Transform_LUT = LongMarch_Vector<Mat4>; //!< the inverse transform matrix at bind pose (from bone space to model space), calculated once.
		using Bone_nameIndex_LUT = LongMarch_UnorderedMap_flat<std::string, uint32_t>; //!< look up table to retrive a bone index by bone name

		//! Node struct for the whole scene (a node might be bone or not for assimp)
		struct Node
		{
			Mat4 nodeTransform; //!< relative to parent transform
			std::string parent_name; //!< name of the parent node
			std::string name; //!< name of the node
			LongMarch_Vector<Node> children; //!< node's children nodes
		};

	public:
		//! Helper method for querying transform for a specific bone
		int GetBoneIndex(const std::string& bone_name) const;
		
		//! Helper method that calculates inverse final transform from global bone transform
		void ApplyInverseBindTransform(const Skeleton::Bone_Transform_LUT& bone_globalTransform_LUT, Skeleton::Bone_Transform_LUT& bone_inverseFinalTransform_LUT) const;

		//! Reset all bone transform to identity matrix and resize the LUT to be the same dimension as the inverse bind LUT
		void ResetBoneTransform(Skeleton::Bone_Transform_LUT& bone_Transform_LUT) const;

		//! Helper method for querying transform for a specific bone
		Mat4 GetBoneTransform(const std::string& bone_name, const Bone_Transform_LUT& bone_Transform_LUT) const;

		//! Helper method for querying node data
		const Node& GetBoneNode(const std::string& bone_name) const;

		//! Helper method for get all parents of a bone, from leaf to root
		LongMarch_Vector<std::string> GetBoneAllParentsName(const std::string& bone_name) const;

		//! Get names of bones
		LongMarch_Vector<std::string> GetAllBoneNames() const;

		//! Load one and only one skeleton from an assimp scene
		static std::shared_ptr<Skeleton> LoadSkeleton(const aiScene* aiscene, const std::string& id);

	private:

		//! Helper method for determine if a bone name is valid
		bool IsBoneNode(const std::string& node_name) const;

		//! Utility function to load all nodes from an assimp file
		static Node LoadAllNodes(const aiNode* node, const std::string& parent_name);

		//! Utility function to load nodes hierarchy from ab assimp file
		static void ReadHierarchy(Skeleton& s, const aiScene* aiscene, const aiNode* node, const aiMatrix4x4& parentTr, unsigned int level);

	public:
		std::string id;
		Bone_Transform_LUT bone_inverseBindTransform_LUT; //!< the inverse transform matrix at bind pose (from bone space to model space), calculated once!
		Bone_nameIndex_LUT boneIndexLUT; //!< look up table to retrive a bone index by bone name
		Node rootNode; //!< Node struct for the whole scene (a node might be bone or a scene in terms of assimp)

	private:
		inline static constexpr const char* s_SceneRootName{ "Scene_Root" };
	};
}
