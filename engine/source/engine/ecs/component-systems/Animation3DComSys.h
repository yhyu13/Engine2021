#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Animation3DCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"

namespace longmarch
{
	class Animation3DComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Animation3DComSys);
		COMSYS_DEFAULT_COPY(Animation3DComSys);

		Animation3DComSys()
		{
			m_systemSignature.AddComponent<Animation3DCom>();
			m_systemSignature.AddComponent<Scene3DCom>();
		}

		virtual void PreRenderUpdate(double dt) override
		{
			ForEach(
				[dt, this](EntityDecorator e)
			{
#ifdef DEBUG_DRAW
				DebugDraw(e);
#endif
			}
			);
		}

		virtual void PostRenderUpdate(double dt) override
		{
			ParEach(
				[dt](EntityDecorator e)
			{
				auto anima = e.GetComponent<Animation3DCom>();
				auto scene = e.GetComponent<Scene3DCom>();
				auto sceneNode = scene->GetSceneData(false);
				if (sceneNode)
				{
					anima->UpdateAnimation(dt, sceneNode.get());
				}
			}
			).wait();
		}

#ifdef DEBUG_DRAW
	private:
		//! Debug drawing for transform component
		void DebugDraw(EntityDecorator e);

		EntityDecorator InitDebugSkeletonRecursive(EntityDecorator parent, const Skeleton::Node& node, int debug_bone_type);
		EntityDecorator InitDebugEETarget(EntityDecorator parent);

		void DrawDebugSkeletonRecursive(const std::shared_ptr<Animation3D>& anima, const Animation3D::SkeletalAnimation& animation, const float animationTicks, EntityDecorator e, const Skeleton::Node& node);
	
	private:
		static Mat4 BoneDebugRot;
#endif
	};
}