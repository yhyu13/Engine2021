#pragma once
#include "../Skeleton.h"
#include "../FABRIKResolver.h"
#include "engine/core/exception/EngineException.h"

namespace AAAAgames
{
	class Animation3DComSys;

	//! Entity agnostic 3D Skeletal animation class. Store a collection of animations that corresponds to a certain skeleton
	struct Animation3D
	{
	public:
		struct VKeyValue
		{
			Vec3f Value{ 0 };
			float Time; // Time in ticks
		};
		struct QKeyValue
		{
			Quaternion Value{ Geommath::UnitQuat };
			float Time; // Time in ticks
		};
		struct SKeyValue
		{
			Vec3f Value{ 1.0f };
			float Time; // Time in ticks
		};

		struct SkeletalKeyFrames
		{
			A4GAMES_Vector<VKeyValue> VKeys;
			A4GAMES_Vector<QKeyValue> QKeys;
			A4GAMES_Vector<SKeyValue> SKeys;
		};

		struct SkeletalAnimation
		{
			float Duration; //!< Total ticks
			float TicksPerSecond{ 30 }; //!< Ticks per second default as 30 fps
			A4GAMES_UnorderedMap_flat<std::string, SkeletalKeyFrames> Channels; //!< Bone name->animation key frames LUT
		};

		//! The name of the animation -> bone name -> all key frames for this bone for this animation
		using AnimationCollection = A4GAMES_UnorderedMap_flat<std::string, SkeletalAnimation>;

	public:
		Animation3D() = default;
		explicit Animation3D(const std::shared_ptr<Skeleton>& SkeltonRef_);

		//! Set skeleton reference to the current animation
		void SetSkeleton(const std::shared_ptr<Skeleton>& SkeltonRef_);

		//! Calculate model space and local space bone transform from animation track
		void CalculateBoneTransform(const std::string& animationName, const float animationTicks, const Mat4& parentTr,
			Skeleton::Bone_Transform_LUT* bone_localSpaceTransform_LUT_OUT,
			Skeleton::Bone_Transform_LUT* bone_gobalSpaceTransform_LUT_OUT, 
			Skeleton::Bone_Transform_LUT* bone_inverseFinalTransform_LUT_OUT) const;

		bool HasAnimation(const std::string& name) const;
		const SkeletalAnimation& GetAnimation(const std::string& name) const;

		//! Get names of animations
		A4GAMES_Vector<std::string> GetAllAnimationNames() const;

		//! Load animation from assimp scene
		static std::shared_ptr<Animation3D> LoadAnimation(const aiScene* aiscene, const std::string& id);

	private:
		friend Animation3DComSys;

		//! Using the node hierachy to calculating all bone's model and local space transform at the current animation tick
		void CalculateBoneTransformRecursive(const SkeletalAnimation& animation, const float animationTicks, const Skeleton::Node& node, const Mat4& parentTr, 
			Skeleton::Bone_Transform_LUT* bone_localSpaceTransform_LUT_OUT,
			Skeleton::Bone_Transform_LUT* bone_modelSpaceTransform_LUT_OUT,
			Skeleton::Bone_Transform_LUT* bone_inverseFinalTransform_LUT_OUT) const;

		static const SkeletalKeyFrames* FindBoneAnima(const SkeletalAnimation& animation, const std::string& nodeName);

		static const Vec3f VInterpolate(float animationTicks, const SkeletalKeyFrames* anim);

		static const Quaternion QInterpolate(float animationTicks, const SkeletalKeyFrames* anim);

		static const Vec3f SInterpolate(float animationTicks, const SkeletalKeyFrames* anim);

	public:
		std::string id;
		AnimationCollection animationCollection; //!< The name of the animation -> bone name -> all key frames for this bone for this animation
		std::shared_ptr<Skeleton> skeletonRef{ nullptr };
	};
}
