#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/math/Geommath.h"
#include "engine/ecs/EntityDecorator.h"
#include "../../../renderer/particles/ParticleSystem3D.h"
#include "engine/renderer/camera/PerspectiveCamera.h"
#include <lobject.h>

namespace longmarch
{
	struct CACHE_ALIGN32 Particle3DCom final : BaseComponent<Particle3DCom>
	{
		Particle3DCom() = default;
		explicit Particle3DCom(const EntityDecorator& _this);

		/*
			Pass in particle system wanted. Example, fire, smoke ,etc.
		*/
		void SetParticleSystem(std::shared_ptr<ParticleSystem3D> particleSystem);
		void Update(const double& frametime, const glm::vec3 cameraPosition);

		void RenderParticleSystems(PerspectiveCamera* camera);

		void SetCenter(glm::vec3 center);
		void EnableRendering();
		void SetPPS(unsigned int count);

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:

		void UpdateModelMatrix(glm::mat4& model, const Particle3D& particle, PerspectiveCamera* camera);

	private:

	std::shared_ptr<ParticleSystem3D> m_particleSystem;
	Entity m_this;

	bool m_render = false;
	};
}
