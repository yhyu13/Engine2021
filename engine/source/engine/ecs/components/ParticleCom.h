#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/renderer/ParticleSystem.h"

namespace longmarch
{
	/* Data class of sprite */
	struct CACHE_ALIGN16 ParticleCom : BaseComponent<ParticleCom> 
	{
		ParticleCom()
			:
			m_ParticleActive(true),
			m_ParticleSystem(nullptr)
		{
		}

		template<typename ParticleType>
		void Init()
		{
			m_ParticleSystem = MemoryManager::Make_shared<ParticleType>();
		}

		bool IsActive()
		{
			return m_ParticleActive;
		}

		void SetActive(bool b)
		{
			LOCK_GUARD();
			m_ParticleActive = b;
		}

		std::shared_ptr<ParticleSystem> Get()
		{
			return m_ParticleSystem;
		}

	private:

		std::shared_ptr<ParticleSystem> m_ParticleSystem;
		bool m_ParticleActive;
	};
}
