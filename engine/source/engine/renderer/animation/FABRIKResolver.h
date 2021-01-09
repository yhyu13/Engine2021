#pragma once
#include "Skeleton.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"
#include "engine/math/Geommath.h"
#include "engine/core/exception/EngineException.h"

namespace longmarch
{
	//! FABRIK IK resolver for skeleton
	class FABRIKResolver : BaseAtomicClassNC
	{
	public:
		struct FABRIKData
		{
			uint32_t iterationBudget{ 10u };
			float eplisonStop{ 1e-3f };
			float snapBackStrength{ .1f };
			uint32_t numBones{ 2u };

			LongMarch_Vector<Mat4> eachBoneTransform; //!< Start at root, end at ee. Size equal to numBones + 1.
			LongMarch_Vector<std::string> eachBoneName; //!< Start at root, end at ee. Size equal to numBones + 1.

			Mat4 target; //!< end-effector's target
			Mat4 pole; //!< end-effector's parent's target
			bool enable_pole{ false };

		private:
			friend FABRIKResolver;

			LongMarch_Vector<float> eachBoneLength; //!< Start at root, end at parent of ee. Size equal to numBones.
			LongMarch_Vector<Vec3f> eachBonePosition; //!< Start at root, end at ee. Size equal to numBones + 1.
			LongMarch_Vector<Vec3f> startBoneDirectionUnnormalized; //!< Start at root, end at ee. Size equal to numBones + 1.
			LongMarch_Vector<Quaternion> startBoneRotation; //!< Start at root, end at ee. Size equal to numBones + 1.
			Quaternion startTargetRotation;

			float totalBoneLength{ 0.f };

			bool reached{ false };
			bool unreachable{ false };
		};

	public:
		NONCOPYABLE(FABRIKResolver);
		FABRIKResolver() = delete;
		explicit FABRIKResolver(const std::shared_ptr<Skeleton>& skeletonRef_);

		void AddIKData(const std::string& ee_name, FABRIKData& data);

		void RemoveIKData(const std::string& ee_name);

		void UpdateIKTarget(const std::string& ee_name, const Mat4& ee_target);

		void UpdateIKPole(const std::string& ee_name, const Mat4& ee_pole);

		void ResolveIK();

	private:
		bool InitData(FABRIKData& data);

		void UpdateBoneTransform(const FABRIKData& data);

	public:
		Skeleton::Bone_Transform_LUT bone_globalSpaceTransform_LUT; //!< even though the variable is named in gloabl space, the bone transform could all be in model space, it does not really matter.
		Skeleton::Bone_Transform_LUT bone_localSpaceTransform_LUT;
		std::shared_ptr<Skeleton> skeletonRef;

	private:
		LongMarch_UnorderedMap_flat<std::string, FABRIKData> m_data;
	};
}