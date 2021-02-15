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
		void SetParticleSystem(const std::shared_ptr<ParticleSystem3D>& particleSystem);
		void Update(const double frametime, const Vec3f& cameraPosition);

		void RenderParticleSystems(const PerspectiveCamera* camera);

		void SetCenter(const Vec3f& center);
		void SetPPS(unsigned int count);

		void SetRendering(bool b);
		bool IsRendering() const;

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:
		void UpdateModelMatrix(Mat4& model, const Particle3D& particle, const PerspectiveCamera* camera);

	private:
		std::shared_ptr<ParticleSystem3D> m_particleSystem{ nullptr };
		Entity m_this;
		bool m_render{ false };
	};
}
