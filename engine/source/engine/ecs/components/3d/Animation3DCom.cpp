#include "engine-precompiled-header.h"
#include "Animation3DCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/ChildrenCom.h"
#include "engine/ecs/components/ParentCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/IDNameCom.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

AAAAgames::Animation3DCom::Animation3DCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.GetWorld()),
	m_this(_this.GetEntity())
{
}

void AAAAgames::Animation3DCom::SetAnimationCollection(const std::string& name)
{
	LOCK_GUARD2();
	if (ResourceManager<Animation3D>::GetInstance()->Has(name))
	{
		m_animaRef = ResourceManager<Animation3D>::GetInstance()->TryGet(name)->Get();
		currentAnimName = "None";
		currentTime = 0.0f;
	}
	else
	{
		ENGINE_EXCEPT(L"Is not a managed animation : " + str2wstr(name));
	}
}

void AAAAgames::Animation3DCom::SetCurrentAnimation(const AnimationSetting& anima)
{
	LOCK_GUARD2();
	if (m_animaRef->HasAnimation(anima.name))
	{
		currentAnimName = anima.name;
		if (anima.refresh)
		{
			currentTime = 0.0f;
		}
		playBackSpeed = anima.playBackSpeed;
		looping = anima.looping;
		pause = anima.pause;
	}
	else
	{
		ENGINE_EXCEPT(L"Is not a valid animation : " + str2wstr(anima.name));
	}
}

void AAAAgames::Animation3DCom::SetAnimationTickTimer(float period)
{
	LOCK_GUARD2();
	animationTickTimer.Reset();
	animationTickTimer.SetPeriod(period);
}

void AAAAgames::Animation3DCom::UpdateAnimation(double dt, Scene3DNode* sceneNode)
{
	LOCK_GUARD2();
	if (currentAnimName != "None" && !pause && m_animaRef)
	{
		currentTime += dt * playBackSpeed;
		if (animationTickTimer.Check(true))
		{
			const auto& currentAnima = m_animaRef->GetAnimation(currentAnimName);
			auto ticks = currentTime * currentAnima.TicksPerSecond;
			if (ticks > currentAnima.Duration) //!< forward playing
			{
				if (looping)
				{
					currentTime = 0;
					ticks = 0;
				}
				else
				{
					ticks = currentAnima.Duration;
				}
			}
			if (ticks < 0) //!< backward playing
			{
				if (looping)
				{
					currentTime = currentAnima.Duration / currentAnima.TicksPerSecond;
					ticks = currentAnima.Duration;
				}
				else
				{
					ticks = 0;
				}
			}
			if (m_IKResolverRef) [[unlikely]]
			{
				m_animaRef->CalculateBoneTransform(currentAnimName, ticks, Mat4(1.0f), 
					&(m_IKResolverRef->bone_localSpaceTransform_LUT),
					&(m_IKResolverRef->bone_globalSpaceTransform_LUT), 
					nullptr);
				m_IKResolverRef->ResolveIK();
				m_animaRef->skeletonRef->ApplyInverseBindTransform(m_IKResolverRef->bone_globalSpaceTransform_LUT, sceneNode->animationData.bone_inverseFinalTransform_LUT);
			}
			else [[likely]]
			{
				m_animaRef->CalculateBoneTransform(currentAnimName, ticks, Mat4(1.0f),
					nullptr,
					nullptr,
					&(sceneNode->animationData.bone_inverseFinalTransform_LUT));
			}
		}
	}
}

void AAAAgames::Animation3DCom::JsonSerialize(Json::Value& value)
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD2();
	{
		Json::Value output;
		output["id"] = "Animation3DCom";
		auto& val = output["value"];
		static const auto& _default = Animation3DCom();
		if (!m_animaRef)
		{
			goto end;
		}
		{
			val["skeleton"] = ResourceManager<Skeleton>::GetInstance()->GetName(m_animaRef->skeletonRef);
		}
		{
			val["collection"] = ResourceManager<Animation3D>::GetInstance()->GetName(m_animaRef);
		}
		{
			val["IKSetting"] = (m_IKResolverRef != nullptr);
		}
		{
			val["current"] = currentAnimName;
		}
		{
			val["playSpeed"] = playBackSpeed;
		}
		{
		end:
			value.append(std::move(output));
		}
	}
}

void AAAAgames::Animation3DCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	if (auto& val = value["collection"]; !val.isNull())
	{
		auto anima = ResourceManager<Animation3D>::GetInstance()->TryGet(val.asString())->TryGet();
		if (anima)
		{
			m_animaRef = anima;
		}
		else
		{
			ENGINE_EXCEPT(str2wstr(val.asString()) + L" animation does not eixst!");
		}
	}
	if (auto& val = value["skeleton"]; !val.isNull())
	{
		auto skeleton = ResourceManager<Skeleton>::GetInstance()->TryGet(val.asString())->TryGet();
		if (skeleton)
		{
			m_animaRef->skeletonRef = skeleton;
		}
		else
		{
			ENGINE_EXCEPT(str2wstr(val.asString()) + L" skeleton does not eixst!");
		}
	}
	if (auto& val = value["IKSetting"]; !val.isNull())
	{
		bool enable = val.asBool();
		if (enable)
		{
			m_IKResolverRef = MemoryManager::Make_shared<FABRIKResolver>(m_animaRef->skeletonRef);
		}
		else
		{
			m_IKResolverRef = nullptr;
		}
	}
	if (auto& val = value["current"]; !val.isNull())
	{
		currentAnimName = val.asString();
	}
	if (auto& val = value["playSpeed"]; !val.isNull())
	{
		playBackSpeed = val.asFloat();
	}
}

void AAAAgames::Animation3DCom::ImGuiRender()
{
	if (ImGui::TreeNode("Animation3D"))
	{
		constexpr int yoffset_item = 2;
		constexpr int width_item = 100;

		// animation collection
		{
			const auto animationName = (m_animaRef) ? ResourceManager<Animation3D>::GetInstance()->GetName(m_animaRef) : "None";
			auto vs = ResourceManager<Animation3D>::GetInstance()->GetAllNames();
			vs.insert(vs.begin(), std::string("None"));
			const auto vc = A4GAMES_StrVec2ConstChar(vs);
			int index = A4GAMES_findFristIndex(vs, animationName);
			if (ImGui::Combo("Animation collection", &index, &vc[0], vc.size()))
			{
				m_animaRef = ResourceManager<Animation3D>::GetInstance()->TryGet(vc[index])->Get();
				m_animaRef->skeletonRef = ResourceManager<Skeleton>::GetInstance()->TryGet(vc[index])->Get();
			}
		}
		if (!m_animaRef)
		{
			ImGui::TreePop();
			return;
		}
		//! Skeleton
		{
			auto skeleton_name = ResourceManager<Skeleton>::GetInstance()->GetName(m_animaRef->skeletonRef);
			auto vs = ResourceManager<Skeleton>::GetInstance()->GetAllNames();
			const auto vc = A4GAMES_StrVec2ConstChar(vs);
			int index = A4GAMES_findFristIndex(vs, skeleton_name);
			if (index != -1)
			{
				ImGui::LabelText("Skeleton", vc[index]);
			}
		}
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, yoffset_item));
#ifdef DEBUG_DRAW
		//! Degbug draw
		{
			bool showBone = debug.showBone;
			if (ImGui::Checkbox("Debug bone", &showBone))
			{
				debug.showBone = showBone;
			}
		}
		{
			auto boneType = debug.showBoneType;
			static const char* boneTypes[] = { "Bone", "Gizmo" };
			ImGui::PushItemWidth(width_item);
			if (ImGui::Combo("Debug bone type", &boneType, boneTypes, IM_ARRAYSIZE(boneTypes)))
			{
				debug.showBoneType = boneType;
			}
			ImGui::PopItemWidth();
			ImGuiUtil::InlineHelpMarker("If you changed debug bone type, you need to toggle the 'Debug bone' check box again.");
		}
		{
			bool hide = debug.hideMesh;
			if (ImGui::Checkbox("Hide mesh", &hide))
			{
				debug.hideMesh = hide;
			}
			if (debug.hideMesh)
			{
				m_world->GetComponent<Scene3DCom>(m_this)->SetVisiable(false);
			}
			else
			{
				m_world->GetComponent<Scene3DCom>(m_this)->SetVisiable(true);
			}
		}
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			bool enableIKTarget = debug.enableIKTarget;
			if (ImGui::Checkbox("Enable IK Target", &enableIKTarget))
			{
				debug.enableIKTarget = enableIKTarget;
			}
			if (debug.enableIKTarget)
			{
				if (!m_IKResolverRef)
				{
					m_IKResolverRef = MemoryManager::Make_shared<FABRIKResolver>(m_animaRef->skeletonRef);
				}
				{
					auto vs = m_IKResolverRef->skeletonRef->GetAllBoneNames();
					const auto vc = A4GAMES_StrVec2ConstChar(vs);
					int index = A4GAMES_findFristIndex(vs, debug.ee_bone_name);
					if (index == -1)
					{
						debug.ee_bone_name = vs[0];
						index = 0;
					}
					if (ImGui::Combo("EE Bone", &index, &vc[0], vc.size()))
					{
						debug.ee_bone_name = vs[index];
					}
				}
				{
					bool enablePole = debug.enablePole;
					if (ImGui::Checkbox("Enable IK Pole", &enablePole))
					{
						debug.enablePole = enablePole;
					}
				}
				{
					int numBones = debug.numBones;
					if (ImGui::SliderInt("# Bones", &numBones, 2, 5))
					{
						debug.numBones = numBones;
					}
				}
				if (debug.showIKTargetInit)
				{
					if (ImGui::Button("Delete Target"))
					{
						m_IKResolverRef->RemoveIKData(debug.ee_bone_name);
						debug.showIKTarget = false;
					}
				}
				else
				{
					if (ImGui::Button("Create Target"))
					{
						// Target already exists
						m_IKResolverRef->RemoveIKData(debug.ee_bone_name);
						debug.showIKTarget = true;
					}
				}
			}
			else
			{
				if (m_IKResolverRef)
				{
					m_IKResolverRef->RemoveIKData(debug.ee_bone_name);
				}
				debug.showIKTarget = false;
			}
		}
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, yoffset_item));
#endif
		{
			bool _pause = pause;
			if (ImGui::Checkbox("Pause", &_pause))
			{
				pause = _pause;
			}
		}
		//! Play speed
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			float speed = 0.01f;
			auto playSpeed = playBackSpeed;
			if (ImGui::DragFloat("Play speed", &playSpeed, speed, playSpeed - 1.0f, playSpeed + 1.0f))
			{
				playBackSpeed = playSpeed;
			}
		}
		//! current animation 
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			auto vs = m_animaRef->GetAllAnimationNames();
			vs.insert(vs.begin(), std::string("None"));
			const auto vc = A4GAMES_StrVec2ConstChar(vs);
			int index = A4GAMES_findFristIndex(vs, currentAnimName);
			if (ImGui::Combo("Current Animation", &index, &vc[0], vc.size()))
			{
				currentAnimName = vs[index];
			}
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}