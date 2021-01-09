#include "engine-precompiled-header.h"
#include "Scene3DNode.h"

//! Copy a scene3DNode is preferred with new copies of all materials

std::shared_ptr<Scene3DNode> AAAAgames::Scene3DNode::Copy() const
{
	auto ret = MemoryManager::Make_shared<Scene3DNode>(*this);
	ret->meshTree.clear();
	for (auto& [level, data] : meshTree)
	{
		ret->meshTree.emplace_back(level, data->Copy());
	}
	return ret;
}

const A4GAMES_Vector<MeshData*> AAAAgames::Scene3DNode::GetAllMesh() const
{
	A4GAMES_Vector<MeshData*> ret;
	std::transform(meshTree.begin(), meshTree.end(), std::back_inserter(ret), [](const auto& item) {return item.second->meshData.get(); });
	return ret;
}

const A4GAMES_Vector<Material*> AAAAgames::Scene3DNode::GetAllMaterial() const
{
	A4GAMES_Vector<Material*> ret;
	std::transform(meshTree.begin(), meshTree.end(), std::back_inserter(ret), [](const auto& item) {return item.second->material.get(); });
	return ret;
}

void AAAAgames::Scene3DNode::ModifyAllMaterial(const std::function<void(Material*)>& callback) const
{
	for (auto& [level, data] : meshTree)
	{
		callback(data->material.get());
	}
}

void AAAAgames::Scene3DNode::SetInverseFinalBoneTransform(const Skeleton::Bone_Transform_LUT& inverseFinal)
{
	animationData.bone_inverseFinalTransform_LUT = inverseFinal;
}

const Skeleton::Bone_Transform_LUT& AAAAgames::Scene3DNode::GetInverseFinalBoneTransform() const
{
	return animationData.bone_inverseFinalTransform_LUT;
}