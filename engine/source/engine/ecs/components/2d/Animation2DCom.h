#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/core/asset-manager/ResourceManager.h"
#include "engine/renderer/Animation/2d/Animation2D.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/allocator/MemoryManager.h"

namespace AAAAgames
{
	/*
	Data class that stores references to the current animation and maintain an animation state map
	*/
	struct CACHE_ALIGN32 Animation2DCom final : BaseComponent<Animation2DCom>{
		Animation2DCom()
			:m_animationState("")
		{
		}

		void Add(const std::string & name, const std::string & stateName)
		{
			m_animationStateMap[stateName] = MemoryManager::Make_shared<Animation2D>(*ResourceManager<Animation2D>::GetInstance()->TryGet(name)->Get());
		}
		std::shared_ptr<Animation2D> GetCurrentAnimation()
		{
			if (auto temp = m_animation.lock())
			{
				return temp;
			}
			else
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Animation2D state " + str2wstr(m_animationState) + L"has failed to be getted!");
			}
		}
		std::string GetCurrentAnimationState()
		{
			return m_animationState;
		}
		void SetCurrentAnimationState(const std::string & state)
		{
			if (m_animationState == state)
			{
				return;
			}
			if (m_animationStateMap.find(state) != m_animationStateMap.end())
			{
				m_animationState = state;
				m_animation = m_animationStateMap[state];
				if (auto temp = m_animation.lock())
				{
					temp->Reset();
				}
				else
				{
					throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Animation2D state " + str2wstr(m_animationState) + L"has failed to be set!");
				}
			}
			else
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Animation2D state " + str2wstr(m_animationState) + L"has not been managed!");
			}
		}
	private:
		std::weak_ptr<Animation2D> m_animation;
		std::string m_animationState;
		A4GAMES_UnorderedMap<std::string, std::shared_ptr<Animation2D>> m_animationStateMap;
	};
}
