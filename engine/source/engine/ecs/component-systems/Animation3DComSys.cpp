#include "engine-precompiled-header.h"
#include "Animation3DComSys.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/ChildrenCom.h"
#include "engine/ecs/components/ParentCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/IDNameCom.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#ifdef DEBUG_DRAW

namespace longmarch
{
	Mat4 Animation3DComSys::BoneDebugRot{ Geommath::RotationMat(Geommath::ROT_AXIS::X, PI * 0.5f) };
}

void longmarch::Animation3DComSys::DebugDraw(EntityDecorator e)
{
	auto anima = e.GetComponent<Animation3DCom>();
	if (anima->debug.showBone)
	{
		if (!anima->debug.showBoneInit)
		{
			auto node = InitDebugSkeletonRecursive(EntityDecorator{ anima->m_this, anima->m_world }, anima->m_animaRef->skeletonRef->rootNode, anima->debug.showBoneType);
			anima.Update();
			anima->debug.rootNode = node;
			anima->debug.showBoneInit = true;
		}
		else
		{
			const auto& currentAnima = anima->m_animaRef->animationCollection[anima->currentAnimName];
			auto ticks = anima->currentTime * currentAnima.TicksPerSecond;
			DrawDebugSkeletonRecursive(anima->m_animaRef, currentAnima, ticks, anima->debug.rootNode, anima->m_animaRef->skeletonRef->rootNode);
		}
	}
	else
	{
		if (anima->debug.showBoneInit)
		{
			if (anima->debug.rootNode.Valid())
			{
				auto queue = EventQueue<EngineEventType>::GetInstance();
				auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(anima->debug.rootNode);
				queue->Publish(e);
			}
			anima->debug.showBoneInit = false;
		}
	}

	if (anima->debug.showIKTarget)
	{
		if (!anima->debug.showIKTargetInit)
		{
			const auto& skeleton = anima->m_IKResolverRef->skeletonRef;
			const auto& boneAndAllParentsNames = skeleton->GetBoneAllParentsName(anima->debug.ee_bone_name);

			if (boneAndAllParentsNames.size() > anima->debug.numBones)
			{
				// Remove existing debug entity
				if (anima->debug.ikTargetNode.Valid())
				{
					auto queue = EventQueue<EngineEventType>::GetInstance();
					auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(anima->debug.ikTargetNode);
					queue->Publish(e);
				}
				auto node = InitDebugEETarget(EntityDecorator{ anima->m_this, anima->m_world });
				anima.Update();
				anima->debug.ikTargetNode = node;

				auto id = node.GetComponent<IDNameCom>();
				id->SetName("ee_target");

				const auto& skeleton = anima->m_IKResolverRef->skeletonRef;
				auto trans = node.GetComponent<Transform3DCom>();
				auto ee_mat = skeleton->GetBoneTransform(anima->debug.ee_bone_name, anima->m_IKResolverRef->bone_globalSpaceTransform_LUT);
				trans->SetRelativeToParentPos(Geommath::GetTranslation(ee_mat));
				trans->SetRelativeToParentRot(Geommath::GetRotation(ee_mat));
				trans->SetLocalScale(Vec3f(5.0f));

				// Remove existing debug entity
				if (anima->debug.ikPoleNode.Valid())
				{
					auto queue = EventQueue<EngineEventType>::GetInstance();
					auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(anima->debug.ikPoleNode);
					queue->Publish(e);
				}
				auto pole_mat = Mat4(1.0f);
				if (anima->debug.enablePole && boneAndAllParentsNames.size() > 1)
				{
					auto node = InitDebugEETarget(EntityDecorator{ anima->m_this, anima->m_world });
					anima.Update();
					anima->debug.ikPoleNode = node;

					auto id = node.GetComponent<IDNameCom>();
					id->SetName("ee_pole");

					const auto& skeleton = anima->m_IKResolverRef->skeletonRef;
					auto trans = node.GetComponent<Transform3DCom>();
					auto pole_mat = skeleton->GetBoneTransform(boneAndAllParentsNames[1], anima->m_IKResolverRef->bone_globalSpaceTransform_LUT);
					trans->SetRelativeToParentPos(Geommath::GetTranslation(pole_mat));
					trans->SetRelativeToParentRot(Geommath::GetRotation(pole_mat));
					trans->SetLocalScale(Vec3f(5.0f));

					auto scene = node.GetComponent<Scene3DCom>();
					auto mesh = scene->GetSceneData(true);
					mesh->ModifyAllMaterial([&](Material* material)
					{
						material->Kd = Vec3f(0.1, 0.8, 0.1);
					});
				}

				{
					FABRIKResolver::FABRIKData ikData;
					ikData.target = ee_mat;
					ikData.enable_pole = anima->debug.enablePole;
					ikData.pole = pole_mat;
					ikData.numBones = anima->debug.numBones;
					anima->debug.ik_root_bone_name = boneAndAllParentsNames[ikData.numBones];
					for (auto i(0u); i < ikData.numBones + 1; ++i)
					{
						const auto& bone_name = boneAndAllParentsNames[i];
						{
							ikData.eachBoneName.emplace_back(bone_name);
						}
						{
							auto transform = skeleton->GetBoneTransform(bone_name, anima->m_IKResolverRef->bone_globalSpaceTransform_LUT);
							ikData.eachBoneTransform.emplace_back(transform);
						}
					}
					{
						std::reverse(ikData.eachBoneName.begin(), ikData.eachBoneName.end());
						std::reverse(ikData.eachBoneTransform.begin(), ikData.eachBoneTransform.end());
					}
					anima->m_IKResolverRef->AddIKData(anima->debug.ee_bone_name, ikData);
				}
				anima->debug.showIKTargetInit = true;
			}
		}
		else
		{
			if (anima->debug.ikTargetNode.Valid())
			{
				auto trans = anima->debug.ikTargetNode.GetComponent<Transform3DCom>();
				auto ee_pos = trans->GetRelativeToParentPos();
				auto ee_rot = trans->GetRelativeToParentRot();
				anima->m_IKResolverRef->UpdateIKTarget(anima->debug.ee_bone_name, Geommath::ToTransformMatrix(ee_pos, ee_rot, Vec3f(1.0f)));
			}
			if (anima->debug.ikPoleNode.Valid())
			{
				auto trans = anima->debug.ikPoleNode.GetComponent<Transform3DCom>();
				auto ee_pos = trans->GetRelativeToParentPos();
				auto ee_rot = trans->GetRelativeToParentRot();
				anima->m_IKResolverRef->UpdateIKPole(anima->debug.ee_bone_name, Geommath::ToTransformMatrix(ee_pos, ee_rot, Vec3f(1.0f)));
			}
		}
	}
	else
	{
		if (anima->debug.showIKTargetInit)
		{
			if (anima->debug.ikTargetNode.Valid())
			{
				auto queue = EventQueue<EngineEventType>::GetInstance();
				auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(anima->debug.ikTargetNode);
				queue->Publish(e);
				anima->debug.ikTargetNode.Reset();
			}
			if (anima->debug.ikPoleNode.Valid())
			{
				auto queue = EventQueue<EngineEventType>::GetInstance();
				auto e = MemoryManager::Make_shared<EngineGCRecursiveEvent>(anima->debug.ikPoleNode);
				queue->Publish(e);
				anima->debug.ikPoleNode.Reset();
			}
			anima->debug.showIKTargetInit = false;
		}
	}
}

EntityDecorator longmarch::Animation3DComSys::InitDebugSkeletonRecursive(EntityDecorator parent, const Skeleton::Node& node, int debug_bone_type)
{
	auto world = parent.GetWorld();

	auto node_ = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::DEBUG_OBJ, true, false);
	auto parentChildCom = parent.GetComponent<ChildrenCom>();
	parentChildCom->AddEntity(node_);

	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	auto meshName = "";
	if (debug_bone_type == 0)
	{
		meshName = "jointBone";
	}
	else if (debug_bone_type == 1)
	{
		meshName = "jointGizmo";
	}
	auto mesh = rm->TryGet(meshName)->Get();
	auto scene = node_.GetComponent<Scene3DCom>();
	scene->SetVisiable(false);
	scene->SetCastShadow(false);
	scene->SetCastReflection(false);
	scene->SetSceneData(mesh);

	auto id = node_.GetComponent<IDNameCom>();
	id->SetName(node.name);

	auto trans = node_.GetComponent<Transform3DCom>();
	trans->m_apply_parent_rot = true;
	trans->m_apply_parent_trans = true;
	trans->m_apply_parent_scale = true;

	const auto& rtpTransform = node.nodeTransform;
	trans->SetRelativeToParentPos(Geommath::GetTranslation(rtpTransform));
	trans->SetRelativeToParentRot(Geommath::GetRotation(rtpTransform));
	trans->SetRelativeToParentScale(Geommath::GetScale(rtpTransform));
	trans->SetLocalScale(Vec3f(Geommath::Length(Geommath::GetTranslation(rtpTransform))));

	for (const auto& child : node.children)
	{
		InitDebugSkeletonRecursive(node_, child, debug_bone_type);
	}
	return node_;
}

EntityDecorator longmarch::Animation3DComSys::InitDebugEETarget(EntityDecorator parent)
{
	auto world = parent.GetWorld();

	auto node_ = world->GenerateEntity3DNoCollision((EntityType)EngineEntityType::DEBUG_OBJ, true, false);
	auto parentChildCom = parent.GetComponent<ChildrenCom>();
	parentChildCom->AddEntity(node_);

	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	auto meshName = "unit_sphere";
	auto mesh = rm->TryGet(meshName)->Get()->Copy();
	auto scene = node_.GetComponent<Scene3DCom>();
	scene->SetVisiable(true);
	scene->SetCastShadow(false);
	scene->SetCastReflection(false);
	mesh->ModifyAllMaterial([&](Material* material)
	{
		material->Kd = Vec3f(0.8, 0.1, 0.1);
	});
	scene->SetSceneData(mesh);

	auto trans = node_.GetComponent<Transform3DCom>();
	trans->m_apply_parent_rot = true;
	trans->m_apply_parent_trans = true;
	trans->m_apply_parent_scale = true;
	return node_;
}

void longmarch::Animation3DComSys::DrawDebugSkeletonRecursive(const std::shared_ptr<Animation3D>& anima, const Animation3D::SkeletalAnimation& animation, const float animationTicks, EntityDecorator e, const Skeleton::Node& node)
{
	auto world = e.GetWorld();
	if (const auto& anim = Animation3D::FindBoneAnima(animation, node.name); anim)
	{
		// Interpolations
		const auto& v = Animation3D::VInterpolate(animationTicks, anim);
		const auto& q = Animation3D::QInterpolate(animationTicks, anim);
		const auto& s = Animation3D::SInterpolate(animationTicks, anim);
		Mat4 nodeTr = Geommath::ToTransformMatrix(v, q, s);
		if (auto it = anima->skeletonRef->boneIndexLUT.find(node.name); it != anima->skeletonRef->boneIndexLUT.end())
		{
			auto scene = e.GetComponent<Scene3DCom>();
			scene->SetVisiable(true);
			auto trans = e.GetComponent<Transform3DCom>();
			const auto& rtpTransform = nodeTr;
			trans->SetRelativeToParentPos(Geommath::GetTranslation(rtpTransform));
			trans->SetRelativeToParentRot(Geommath::GetRotation(rtpTransform));
			trans->SetRelativeToParentScale(Geommath::GetScale(rtpTransform));
		}
		else
		{
			// There might be missing bones for no apprent reason, thus the exception is commentted out
			//ENGINE_EXCEPT(L"Unregistered bone in a animation : " + str2wstr(node.name));
		}
	}
	else
	{
		auto scene = e.GetComponent<Scene3DCom>();
		if (auto it = anima->skeletonRef->boneIndexLUT.find(node.name); it != anima->skeletonRef->boneIndexLUT.end())
		{
			scene->SetVisiable(true);
			auto trans = e.GetComponent<Transform3DCom>();
			const auto& rtpTransform = node.nodeTransform;
			trans->SetRelativeToParentPos(Geommath::GetTranslation(rtpTransform));
			trans->SetRelativeToParentRot(Geommath::GetRotation(rtpTransform));
			trans->SetRelativeToParentScale(Geommath::GetScale(rtpTransform));
		}
		else
		{
			scene->SetVisiable(false);
		}
	}
	auto childCom = e.GetComponent<ChildrenCom>();
	const auto& children = childCom->GetChildren();
	for (auto i(0u); i < children.size(); ++i)
	{
		DrawDebugSkeletonRecursive(anima, animation, animationTicks, EntityDecorator{ children[i] ,world }, node.children[i]);
	}
}
#endif
