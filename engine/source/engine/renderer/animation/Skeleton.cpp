#include "engine-precompiled-header.h"
#include "Skeleton.h"

std::shared_ptr<Skeleton> longmarch::Skeleton::LoadSkeleton(const aiScene* aiscene, const std::string& id)
{
	auto ret = MemoryManager::Make_shared<Skeleton>();
	ret->id = id;
	ret->rootNode = std::move(LoadAllNodes(aiscene->mRootNode, s_SceneRootName));
	ReadHierarchy(*ret, aiscene, aiscene->mRootNode, aiMatrix4x4(), 0);
	return ret;
}

//! Utility function to load assimp file into scene node

Skeleton::Node longmarch::Skeleton::LoadAllNodes(const aiNode* node, const std::string& parent_name)
{
	Node ret{ .nodeTransform = AssimpHelper::Assimp2Glm(node->mTransformation), .parent_name = parent_name, .name = node->mName.C_Str() };
	// Recurse onto this node's children
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ret.children.emplace_back(LoadAllNodes(node->mChildren[i], ret.name));
	}
	return ret;
}

void longmarch::Skeleton::ReadHierarchy(Skeleton& s, const aiScene* aiscene, const aiNode* node, const aiMatrix4x4& parentTr, unsigned int level)
{
	// Accumulating transformations while traversing down the hierarchy.
	const aiMatrix4x4 childTr = parentTr * node->mTransformation; //TODO: whether or not to accumulate transformation
	const Mat4 inverseChildTr = Geommath::SmartInverse(AssimpHelper::Assimp2Glm(childTr));
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* aimesh = aiscene->mMeshes[node->mMeshes[i]];
#if MESH_VERTEX_DATA_FORMAT == 4
		{
			for (unsigned int i = 0; i < aimesh->mNumBones; ++i)
			{
				const aiBone* bone = aimesh->mBones[i];
				unsigned int boneIndex = 0;
				const std::string boneName(bone->mName.C_Str());
				//! Fill in bond info and bone index
				if (auto it = s.boneIndexLUT.find(boneName); it != s.boneIndexLUT.end()) [[unlikely]]
				{
					boneIndex = it->second;
				}
				else [[likely]]
				{
					boneIndex = s.bone_inverseBindTransform_LUT.size();
					s.boneIndexLUT[boneName] = boneIndex;
					s.bone_inverseBindTransform_LUT.emplace_back(AssimpHelper::Assimp2Glm(bone->mOffsetMatrix) * inverseChildTr); //!< the offset matrix should directly operator on the vertices, so we multiply the inverse of transform
					ASSERT(s.bone_inverseBindTransform_LUT.size() == (boneIndex + 1), "Bone info must match");
				}
			}
		}
#endif
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ReadHierarchy(s, aiscene, node->mChildren[i], childTr, level + 1);
	}
}

int longmarch::Skeleton::GetBoneIndex(const std::string& bone_name) const
{
	if (auto it = boneIndexLUT.find(bone_name); it != boneIndexLUT.end())
	{
		return it->second;
	}
	else
	{
		return -1;
	}
}

void longmarch::Skeleton::ApplyInverseBindTransform(const Skeleton::Bone_Transform_LUT& bone_Transform_LUT_IN, Skeleton::Bone_Transform_LUT& bone_inverseFinalTransform_LUT_OUT) const
{
	ASSERT(bone_Transform_LUT_IN.size() == bone_inverseBindTransform_LUT.size(), "Bone transform has different size!")
	if (bone_Transform_LUT_IN.size() != bone_inverseFinalTransform_LUT_OUT.size())
	{
		ResetBoneTransform(bone_inverseFinalTransform_LUT_OUT);
	}
	for (auto i(0u); i < bone_inverseBindTransform_LUT.size(); ++i)
	{
		bone_inverseFinalTransform_LUT_OUT[i] = bone_Transform_LUT_IN[i] * bone_inverseBindTransform_LUT[i];
	}
}

void longmarch::Skeleton::ResetBoneTransform(Skeleton::Bone_Transform_LUT& bone_Transform_LUT) const
{
	// Resize buffer
	bone_Transform_LUT.clear();
	bone_Transform_LUT.resize(bone_inverseBindTransform_LUT.size(), Mat4(1.0f));
}

Mat4 longmarch::Skeleton::GetBoneTransform(const std::string& bone_name, const Bone_Transform_LUT& bone_Transform_LUT) const
{
	ASSERT(bone_Transform_LUT.size() == bone_inverseBindTransform_LUT.size(), "Bone LUT has different size!");
	if (auto index = GetBoneIndex(bone_name); index != -1)
	{
		return bone_Transform_LUT[index];
	}
	else
	{
		ENGINE_EXCEPT(L"Bone does not exists in this skeleton: " + str2wstr(bone_name));
		return Mat4();
	}
}

const Skeleton::Node& longmarch::Skeleton::GetBoneNode(const std::string& bone_name) const
{
	if (!IsBoneNode(bone_name))
	{
		ENGINE_EXCEPT(L"Node is not a managed bone in this skeleton: " + str2wstr(bone_name));
	}
	static std::function<const Node&(const Node&, const std::string&)> findBoneNode = [](const Node& parent, const std::string& bone_name) -> const Node&
	{
		if (parent.name == bone_name)
		{
			return parent;
		}
		else
		{
			for (auto& child : parent.children)
			{
				auto& node = findBoneNode(child, bone_name);
				if (node.name == bone_name)
				{
					return node;
				}
			}
			return parent;
		}
	};
	auto& node = findBoneNode(rootNode, bone_name);
	if (node.name == bone_name)
	{
		return node;
	}
	else
	{
		ENGINE_EXCEPT(L"Can't find bone: " + str2wstr(bone_name));
		return Skeleton::Node();
	}
}

LongMarch_Vector<std::string> longmarch::Skeleton::GetBoneAllParentsName(const std::string& bone_name) const
{
	LongMarch_Vector<std::string> ret;
	auto node = GetBoneNode(bone_name);
	// Get all parent bones until we hit a none-bone node
	while (IsBoneNode(node.name))
	{
		ret.emplace_back(node.name);
		if (IsBoneNode(node.parent_name))
		{
			node = GetBoneNode(node.parent_name);
		}
		else
		{
			break;
		}
	}
	return ret;
}

bool longmarch::Skeleton::IsBoneNode(const std::string& node_name) const
{
	if (auto it = boneIndexLUT.find(node_name); it != boneIndexLUT.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

LongMarch_Vector<std::string> longmarch::Skeleton::GetAllBoneNames() const
{
	LongMarch_Vector<std::string> ret;
	LongMarch_MapKeyToVec(boneIndexLUT, ret);
	return ret;
}
