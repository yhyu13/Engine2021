#include "engine-precompiled-header.h"
#include "Animation3D.h"

std::shared_ptr<Animation3D> longmarch::Animation3D::LoadAnimation(const aiScene* aiscene, const std::string& id)
{
	auto ret = MemoryManager::Make_shared<Animation3D>();
	ret->id = id;
	auto& animationCollection = ret->animationCollection;
	for (unsigned int i = 0; i < aiscene->mNumAnimations; ++i)
	{
		const aiAnimation* anim = aiscene->mAnimations[i];
		auto animName = std::string(anim->mName.C_Str());
		if (animName == "")
		{
			animName = Str(i);
		}
		ENGINE_EXCEPT_IF(animationCollection.contains(animName), L"Animation already exists : " + wStr(animName));

		auto& _anim = animationCollection[animName];
		_anim.Duration = anim->mDuration;
		_anim.TicksPerSecond = (anim->mTicksPerSecond != 0) ? anim->mTicksPerSecond : _anim.TicksPerSecond;
		if (_anim.TicksPerSecond == 1 && _anim.Duration > 100) //!< assimp may import duration as millisecond but tick per second as 1, we need to convert tick per second to 1000 then
		{
			_anim.TicksPerSecond = 1000;
		}
		auto& _chans = _anim.Channels;

		// The animations channels
		for (unsigned int i = 0; i < anim->mNumChannels; ++i)
		{
			const aiNodeAnim* chan = anim->mChannels[i];
			auto boneName = chan->mNodeName.C_Str();
			ENGINE_EXCEPT_IF(_chans.contains(boneName), L"Bone animation already exists : " + wStr(boneName));

			auto& _chan = _chans[boneName];
			// The position (V) keys
			if (chan->mNumPositionKeys == 0 && _chan.VKeys.empty())
			{
				_chan.VKeys.emplace_back(VKeyValue{});
			}
			else
			{
				_chan.VKeys.reserve(chan->mNumPositionKeys);
				for (unsigned int i = 0; i < chan->mNumPositionKeys; ++i)
				{
					aiVectorKey key = chan->mPositionKeys[i];
					_chan.VKeys.emplace_back(VKeyValue{ .Value = { key.mValue[0], key.mValue[1], key.mValue[2] }, .Time = (float)key.mTime });
				}
			}
			// The rotation (Q) keys
			if (chan->mNumRotationKeys == 0 && _chan.QKeys.empty())
			{
				_chan.QKeys.emplace_back(QKeyValue{});
			}
			else
			{
				_chan.QKeys.reserve(chan->mNumRotationKeys);
				for (unsigned int i = 0; i < chan->mNumRotationKeys; ++i)
				{
					aiQuatKey key = chan->mRotationKeys[i];
					_chan.QKeys.emplace_back(QKeyValue{ .Value = { key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z }, .Time = (float)key.mTime });
				}
			}
			// the scaling (S) keys
			if (chan->mNumScalingKeys == 0 && _chan.SKeys.empty())
			{
				_chan.SKeys.emplace_back(SKeyValue{});
			}
			else
			{
				_chan.SKeys.reserve(chan->mNumScalingKeys);
				for (unsigned int i = 0; i < chan->mNumScalingKeys; ++i)
				{
					aiVectorKey key = chan->mScalingKeys[i];
					_chan.SKeys.emplace_back(SKeyValue{ .Value = { key.mValue[0], key.mValue[1], key.mValue[2] }, .Time = (float)key.mTime });
				}
			}
		}
	}
	return ret;
}


longmarch::Animation3D::Animation3D(const std::shared_ptr<Skeleton>& SkeltonRef_)
	:
	skeletonRef(SkeltonRef_)
{}

void longmarch::Animation3D::SetSkeleton(const std::shared_ptr<Skeleton>& SkeltonRef_)
{
	skeletonRef = SkeltonRef_;
}

void longmarch::Animation3D::CalculateBoneTransform(const std::string& animationName, const float animationTicks, const Mat4& parentTr,
	Skeleton::Bone_Transform_LUT* bone_localSpaceTransform_LUT_OUT,
	Skeleton::Bone_Transform_LUT* bone_gobalSpaceTransform_LUT_OUT,
	Skeleton::Bone_Transform_LUT* bone_inverseFinalTransform_LUT_OUT) const
{
	if (bone_localSpaceTransform_LUT_OUT)
	{
		skeletonRef->ResetBoneTransform(*bone_localSpaceTransform_LUT_OUT);
	}
	if (bone_gobalSpaceTransform_LUT_OUT)
	{
		skeletonRef->ResetBoneTransform(*bone_gobalSpaceTransform_LUT_OUT);
	}
	if (bone_inverseFinalTransform_LUT_OUT)
	{
		skeletonRef->ResetBoneTransform(*bone_inverseFinalTransform_LUT_OUT);
	}
	if (auto anima_it = animationCollection.find(animationName); anima_it != animationCollection.end())
	{
		CalculateBoneTransformRecursive(anima_it->second, animationTicks, skeletonRef->rootNode, parentTr,
			bone_localSpaceTransform_LUT_OUT,
			bone_gobalSpaceTransform_LUT_OUT, 
			bone_inverseFinalTransform_LUT_OUT);
	}
	else
	{
		ENGINE_EXCEPT(L"Unregistered animation: " + wStr(animationName));
	}
}

bool longmarch::Animation3D::HasAnimation(const std::string& name) const
{
	return animationCollection.find(name) != animationCollection.end();
}

const Animation3D::SkeletalAnimation& longmarch::Animation3D::GetAnimation(const std::string& name) const
{
	if (auto it = animationCollection.find(name); it != animationCollection.end())
	{
		return it->second;
	}
	else
	{
		ENGINE_EXCEPT(L"Is not a valid animation : " + wStr(name));
		return SkeletalAnimation();
	}
}

LongMarch_Vector<std::string> longmarch::Animation3D::GetAllAnimationNames() const
{
	LongMarch_Vector<std::string> ret;
	LongMarch_MapKeyToVec(animationCollection, ret);
	return ret;
}

//! Using the node hierachy to calculating all bone's final inverse transform at the current animation tick
void longmarch::Animation3D::CalculateBoneTransformRecursive(const SkeletalAnimation& animation, const float animationTicks, const Skeleton::Node& node, const Mat4& parentTr, 
	Skeleton::Bone_Transform_LUT* bone_localSpaceTransform_LUT_OUT,
	Skeleton::Bone_Transform_LUT* bone_gobalSpaceTransform_LUT_OUT,
	Skeleton::Bone_Transform_LUT* bone_inverseFinalTransform_LUT_OUT) const
{
	// Find global transform
	Mat4 globalTr;
	if (const auto& anim = FindBoneAnima(animation, node.name); anim)
	{
		// Interpolations
		const auto& v = VInterpolate(animationTicks, anim);
		const auto& q = QInterpolate(animationTicks, anim);
		const auto& s = SInterpolate(animationTicks, anim);
		Mat4 nodeTr = Geommath::ToTransformMatrix(v, q, s);
		globalTr = parentTr * nodeTr;
		// Update final inverse transform
		if (auto it = skeletonRef->boneIndexLUT.find(node.name); it != skeletonRef->boneIndexLUT.end())
		{
			auto boneIndex = it->second;
			if (bone_localSpaceTransform_LUT_OUT)
			{
				(*bone_localSpaceTransform_LUT_OUT)[boneIndex] = nodeTr;
			}
			if (bone_gobalSpaceTransform_LUT_OUT)
			{
				(*bone_gobalSpaceTransform_LUT_OUT)[boneIndex] = globalTr;
			}
			if (bone_inverseFinalTransform_LUT_OUT)
			{
				(*bone_inverseFinalTransform_LUT_OUT)[boneIndex] = globalTr * skeletonRef->bone_inverseBindTransform_LUT[boneIndex];
			}
		}
		else
		{
			// There might be missing bones for no apprent reason, thus the exception is commentted out
			//ENGINE_EXCEPT(L"Unregistered bone in a animation : " + wStr(node.name));
		}
	}
	else
	{
		auto& nodeTr = node.nodeTransform;
		globalTr = parentTr * nodeTr;
		if (auto it = skeletonRef->boneIndexLUT.find(node.name); it != skeletonRef->boneIndexLUT.end())
		{
			auto boneIndex = it->second;
			if (bone_localSpaceTransform_LUT_OUT)
			{
				(*bone_localSpaceTransform_LUT_OUT)[boneIndex] = nodeTr;
			}
			if (bone_gobalSpaceTransform_LUT_OUT)
			{
				(*bone_gobalSpaceTransform_LUT_OUT)[boneIndex] = globalTr;
			}
			if (bone_inverseFinalTransform_LUT_OUT)
			{
				(*bone_inverseFinalTransform_LUT_OUT)[boneIndex] = globalTr * skeletonRef->bone_inverseBindTransform_LUT[boneIndex];
			}
		}
	}
	// Recurisve to all children
	for (const auto& child : node.children)
	{
		CalculateBoneTransformRecursive(animation, animationTicks, child, globalTr, 
			bone_localSpaceTransform_LUT_OUT,
			bone_gobalSpaceTransform_LUT_OUT,
			bone_inverseFinalTransform_LUT_OUT);
	}
}

const Animation3D::SkeletalKeyFrames* longmarch::Animation3D::FindBoneAnima(const SkeletalAnimation& animation, const std::string& nodeName)
{
	if (auto it = animation.Channels.find(nodeName); it != animation.Channels.end())
	{
		return &(it->second);
	}
	else
	{
		return nullptr;
	}
}

const Vec3f longmarch::Animation3D::VInterpolate(float animationTicks, const SkeletalKeyFrames* anim)
{
	const auto& keys = anim->VKeys;
	if (keys.size() > 1)
	{
		// Using std::lower_bound is actually slower than the plain old for loop
		//auto next = std::lower_bound(keys.begin() + 1, keys.end() - 1, animationTicks, [](const auto& key, float animationTicks)->bool {return key.Time < animationTicks; });
		//int nextIndex = std::distance(keys.begin(), next);
		//int index = nextIndex - 1;
		int index = keys.size() - 2;
		for (auto i(0u); i < keys.size() - 1; ++i)
		{
			if (animationTicks < keys[i + 1].Time)
			{
				index = i;
				break;
			}
		}
		int nextIndex = index + 1;

		float deltaTime = keys[nextIndex].Time - keys[index].Time;
		float factor = (animationTicks - keys[index].Time) / deltaTime;
		const auto& start = keys[index].Value;
		const auto& end = keys[nextIndex].Value;
		return Geommath::Lerp(start, end, factor);
	}
	else if (keys.size() == 1)
	{
		return keys[0].Value;
	}
	else
	{
		ENGINE_EXCEPT(L"V keys is empty!");
		return Vec3f();
	}
}

const Quaternion longmarch::Animation3D::QInterpolate(float animationTicks, const SkeletalKeyFrames* anim)
{
	const auto& keys = anim->QKeys;
	if (keys.size() > 1)
	{
		// Using std::lower_bound is actually slower than the plain old for loop
		//auto next = std::lower_bound(keys.begin() + 1, keys.end() - 1, animationTicks, [](const auto& key, float animationTicks)->bool {return key.Time < animationTicks; });
		//int nextIndex = std::distance(keys.begin(), next);
		//int index = nextIndex - 1;
		int index = keys.size() - 2;
		for (auto i(0u); i < keys.size() - 1; ++i)
		{
			if (animationTicks < keys[i + 1].Time)
			{
				index = i;
				break;
			}
		}
		int nextIndex = index + 1;

		float deltaTime = keys[nextIndex].Time - keys[index].Time;
		float factor = (animationTicks - keys[index].Time) / deltaTime;
		const auto& start = keys[index].Value;
		const auto& end = keys[nextIndex].Value;
		return Geommath::Slerp(start, end, factor);
	}
	else if (keys.size() == 1)
	{
		return keys[0].Value;
	}
	else
	{
		ENGINE_EXCEPT(L"Q keys is empty!");
		return Quaternion();
	}
}

const Vec3f longmarch::Animation3D::SInterpolate(float animationTicks, const SkeletalKeyFrames* anim)
{
	const auto& keys = anim->SKeys;
	if (keys.size() > 1)
	{
		// Using std::lower_bound is actually slower than the plain old for loop
		//auto next = std::lower_bound(keys.begin() + 1, keys.end() - 1, animationTicks, [](const auto& key, float animationTicks)->bool {return key.Time < animationTicks; });
		//int nextIndex = std::distance(keys.begin(), next);
		//int index = nextIndex - 1;
		int index = keys.size() - 2;
		for (auto i(0u); i < keys.size() - 1; ++i)
		{
			if (animationTicks < keys[i + 1].Time)
			{
				index = i;
				break;
			}
		}
		int nextIndex = index + 1;

		float deltaTime = keys[nextIndex].Time - keys[index].Time;
		float factor = (animationTicks - keys[index].Time) / deltaTime;
		const auto& start = keys[index].Value;
		const auto& end = keys[nextIndex].Value;
		return Geommath::Lerp(start, end, factor);
	}
	else if (keys.size() == 1)
	{
		return keys[0].Value;
	}
	else
	{
		ENGINE_EXCEPT(L"S keys is empty!");
		return Vec3f();
	}
}
