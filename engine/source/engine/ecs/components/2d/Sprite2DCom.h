#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/core/allocator/MemoryManager.h"
#include "engine/core/asset-manager/ResourceManager.h"
#include "engine/renderer/Sprite.h"

namespace longmarch
{
	/* Data class of sprite */
	struct MS_ALIGN16 Sprite2DCom final : BaseComponent<Sprite2DCom>
	{
		Sprite2DCom()
		{
			m_sprite = MemoryManager::Make_shared<Sprite>();
		}

		void SetTexture(std::shared_ptr<Texture2D> texture)
		{
			m_sprite->SetSpriteTexture(texture);
		}

		void SetTexture(const std::string & name)
		{
			m_sprite->SetSpriteTexture(ResourceManager<Texture2D>::GetInstance()->TryGet(name)->TryGet());
			m_name = name;
		}

		const std::string & GetTextureName()
		{
			return m_name;
		}

		void SetScale(const glm::vec2 & t)
		{
			m_sprite->SetSpriteScale(t);
		}

		void SetAlpha(float a)
		{
			m_sprite->SetSpriteAlpha(a);
		}

		std::shared_ptr<Sprite> Get()
		{
			return m_sprite;
		}
	private:
		std::shared_ptr<Sprite> m_sprite;
		std::string m_name;
	};
}
