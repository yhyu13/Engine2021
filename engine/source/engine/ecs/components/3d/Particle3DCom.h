#pragma once

#include "engine/ecs/BaseComponent.h"
#include "engine/math/Geommath.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/renderer/particles/ParticleSystem3D.h"
#include "engine/renderer/camera/PerspectiveCamera.h"

namespace longmarch
{
	struct MS_ALIGN8 Particle3DCom final : public BaseComponent<Particle3DCom>
	{
		Particle3DCom() = default;
		explicit Particle3DCom(const EntityDecorator& _this);

		//! Pass in particle system wanted. Example, fire, smoke ,etc.
		void SetParticleSystem(const std::shared_ptr<ParticleSystem3D>& particleSystem);
		void Update(const double frametime, const PerspectiveCamera* camera);
		//! Prepare to draw particles with a given view matrix, call before calling Draw()
		void PrepareDrawDataWithViewMatrix(const Mat4& viewMatrix);

		void Draw();
		void Draw(const std::function<void(const Renderer3D::ParticleInstanceDrawData&)>& drawFunc);

		void SetCenter(const Vec3f& center);
		void SetPPS(unsigned int count);

		void SetRendering(bool b);
		bool IsRendering() const;

		virtual void JsonSerialize(Json::Value& value) const override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:
		Mat4 GetModelViewMatrix(const Particle3D& particle, const Mat4& viewMatrix);

	private:
		Renderer3D::ParticleInstanceDrawData m_instancedDataList;
		std::shared_ptr<ParticleSystem3D> m_particleSystem{ nullptr };
		Entity m_this;
		bool m_render{ false };
	};
}
