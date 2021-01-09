#include "engine-precompiled-header.h"
#include "FABRIKResolver.h"

longmarch::FABRIKResolver::FABRIKResolver(const std::shared_ptr<Skeleton>& skeletonRef_)
	:
	skeletonRef(skeletonRef_)
{}

void longmarch::FABRIKResolver::AddIKData(const std::string& ee_name, FABRIKData& data)
{
	LOCK_GUARD_NC();
	if (InitData(data))
	{
		m_data[ee_name] = data;
	}
}

bool longmarch::FABRIKResolver::InitData(FABRIKData& data)
{
	data.reached = false;

	ASSERT(data.numBones + 1 == data.eachBoneName.size(), "IK data corruption!");
	ASSERT(data.numBones + 1 == data.eachBoneTransform.size(), "IK data corruption!");
	data.startTargetRotation = Geommath::GetRotation(data.target);

	data.eachBonePosition.resize(data.numBones + 1);
	data.startBoneRotation.resize(data.numBones + 1);
	data.startBoneDirectionUnnormalized.resize(data.numBones + 1);
	data.eachBoneLength.resize(data.numBones);
	data.totalBoneLength = 0.f;

	for (auto i(0u); i < data.numBones + 1; ++i)
	{
		data.startBoneRotation[i] = Geommath::GetRotation(data.eachBoneTransform[i]);
		if (i == data.numBones)
		{
			// ee bone
			data.startBoneDirectionUnnormalized[i] = Geommath::GetTranslation(data.target) - Geommath::GetTranslation(data.eachBoneTransform[i]);
		}
		else
		{
			// mid bone
			data.startBoneDirectionUnnormalized[i] = Geommath::GetTranslation(data.eachBoneTransform[i + 1]) - Geommath::GetTranslation(data.eachBoneTransform[i]);
			data.eachBoneLength[i] = Geommath::Length(data.startBoneDirectionUnnormalized[i]);
			data.totalBoneLength += data.eachBoneLength[i];
		}
	}

	return true;
}

void longmarch::FABRIKResolver::RemoveIKData(const std::string& ee_name)
{
	LOCK_GUARD_NC();
	m_data.erase(ee_name);
}

void longmarch::FABRIKResolver::UpdateIKTarget(const std::string& ee_name, const Mat4& ee_target)
{
	LOCK_GUARD_NC();
	if (auto it = m_data.find(ee_name); it != m_data.end())
	{
		auto& data = it->second;
		if (data.target != ee_target)
		{
			data.target = ee_target;
			data.reached = false;
		}
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(ee_name) + L" is not managed!");
	}
}

void longmarch::FABRIKResolver::UpdateIKPole(const std::string& ee_name, const Mat4& ee_pole)
{
	LOCK_GUARD_NC();
	if (auto it = m_data.find(ee_name); it != m_data.end())
	{
		auto& data = it->second;
		if (data.pole != ee_pole)
		{
			data.pole = ee_pole;
			data.reached = false;
		}
	}
	else
	{
		ENGINE_EXCEPT(str2wstr(ee_name) + L" is not managed!");
	}
}

void longmarch::FABRIKResolver::ResolveIK()
{
	LOCK_GUARD_NC();
	for (auto& [ee_name, data] : m_data)
	{
		auto targetPosition = Geommath::GetTranslation(data.target);
		auto targetRotation = Geommath::GetRotation(data.target);

		auto& Transforms = data.eachBoneTransform;
		auto& Positions = data.eachBonePosition;
		const auto& BonesLength = data.eachBoneLength;
		const auto& StartDirectionSucc = data.startBoneDirectionUnnormalized;
		const auto& StartRotationBone = data.startBoneRotation;

		if (!data.reached)
		{
			// Set bone position
			for (auto i(0u); i < data.numBones + 1; ++i)
			{
				Positions[i] = Geommath::GetTranslation(Transforms[i]);
			}

			// Check reachable
			data.unreachable = (Geommath::DistanceSquare(targetPosition, Positions[0]) >= data.totalBoneLength * data.totalBoneLength);
			if (data.unreachable)
			{
				// Unreachable
				// Just strech all bones
				auto direction = Geommath::Normalize(targetPosition - Positions[0]);
				for (auto i(1u); i < data.numBones + 1; ++i)
				{
					Positions[i] = Positions[i - 1] + direction * BonesLength[i - 1];
				}
			}
			else
			{
				// Reachable
				// Snap back to starting position
				for (auto i(0u); i < data.numBones; ++i)
				{
					Positions[i + 1] = Geommath::Lerp(Positions[i + 1], Positions[i] + StartDirectionSucc[i], data.snapBackStrength);
				}
				// Start iteration
				for (auto iter(0u); iter < data.iterationBudget; ++iter)
				{
					if (data.reached)
					{
						break;
					}
					else
					{
						// Back : ee to parent of root
						Positions[data.numBones] = targetPosition;
						for (int i = data.numBones - 1; i > 0; --i)
						{
							auto direction = Geommath::Normalize(Positions[i] - Positions[i + 1]);
							Positions[i] = Positions[i + 1] + direction * BonesLength[i];
						}
						// Forward : parent of root to ee
						for (auto i(1u); i < data.numBones + 1; ++i)
						{
							auto direction = Geommath::Normalize(Positions[i] - Positions[i - 1]);
							Positions[i] = Positions[i - 1] + direction * BonesLength[i - 1];
						}
					}
					data.reached = (Geommath::DistanceSquare(Positions[data.numBones], targetPosition) <= data.eplisonStop);
				}
			}

			if (data.enable_pole)
			{
				auto polePosition = Geommath::GetTranslation(data.pole);
				for (auto i(1u); i < data.numBones; ++i)
				{
					auto plane = Geommath::Plane::FromNP(Positions[i + 1] - Positions[i - 1], Positions[i - 1]);
					auto plane_normal = Vec3f(plane.xyz);
					auto projectedPole = Geommath::Plane::ProjectedPointOnPlane(plane, polePosition);
					auto projectedBone = Geommath::Plane::ProjectedPointOnPlane(plane, Positions[i]);
					auto angle = Geommath::SignedAngle(projectedBone - Positions[i - 1], projectedPole - Positions[i - 1], plane_normal);
					Positions[i] = Geommath::FromAxisRot(angle, plane_normal) * (Positions[i] - Positions[i - 1]) + Positions[i - 1];
				}
			}
		}
		// Update IK bone transform
		for (auto i(0u); i < data.numBones + 1; ++i)
		{
			if (i == data.numBones)
			{
				Geommath::SetRotation(Transforms[i], Geommath::Inverse(targetRotation) * data.startTargetRotation * StartRotationBone[i]);
			}
			else
			{
				Geommath::SetRotation(Transforms[i], Geommath::FromVectorPair(StartDirectionSucc[i], Positions[i + 1] - Positions[i]) * StartRotationBone[i]);
			}
			Geommath::SetTranslation(Transforms[i], Positions[i]);
		}
		// Update overall bone transform
		UpdateBoneTransform(data);
	}
}

void longmarch::FABRIKResolver::UpdateBoneTransform(const FABRIKData& data)
{
	static std::function<void(FABRIKResolver*, const FABRIKData&, const std::string&)> update_children_node = [](FABRIKResolver* res, const FABRIKData& data, const std::string& parent_bone_name)
	{
		// Recursively update all non-ik children bones
		if (auto parent_index = res->skeletonRef->GetBoneIndex(parent_bone_name); parent_index != -1)
		{
			auto node = res->skeletonRef->GetBoneNode(parent_bone_name);
			for (const auto& child : node.children)
			{
				auto bone_name = child.name;
				if (!LongMarch_Contains(data.eachBoneName, bone_name))
				{
					if (auto index = res->skeletonRef->GetBoneIndex(bone_name); index != -1)
					{
						res->bone_globalSpaceTransform_LUT[index] = res->bone_globalSpaceTransform_LUT[parent_index] * res->bone_localSpaceTransform_LUT[index];
						update_children_node(res, data, bone_name);
					}
				}
			}
		}
	};
	for (auto i(0u); i < data.numBones + 1; ++i)
	{
		auto bone_name = data.eachBoneName[i];
		if (auto index = skeletonRef->GetBoneIndex(bone_name); index != -1)
		{
			// Update bone transform with ik result
			bone_globalSpaceTransform_LUT[index] = data.eachBoneTransform[i];
			// Also update children bones
			update_children_node(this, data, bone_name);
		}
		else
		{
			ENGINE_EXCEPT(L"IK Bone does not exists: " + str2wstr(bone_name));
		}
	}
}