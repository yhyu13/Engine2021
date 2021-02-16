#pragma once

#include "engine/math/Geommath.h"
#include "engine/core/utility/TypeHelper.h"
#include "../../renderer/Texture.h"

#include "Particle3D.h"

namespace longmarch
{
	class ParticleSystem3D
	{
	public:
		ParticleSystem3D();
		explicit ParticleSystem3D(float _pps, float _avgSpeed, float _gravityCompliance, float _avgLifeLength, float _avgScale, const std::string& texturename);

		void RandomizeRotation();
		void SetDirection(const Vec3f& _direction, const float _deviation);
		void SetSpeedvariation(const float variation);
		void SetLifeLengthVariation(const float variation);
		void SetScaleVariation(const float variation);
		void SetCenterOffset(const Vec3f& center_offset);
		void SetCenter(const Vec3f& center);
		void Update(const float frametime, const Vec3f& cameraPosition);

		virtual LongMarch_Vector<Particle3D>& GetParticles();
		virtual std::shared_ptr<Texture2D> GetTexture();

	protected:
		virtual void EmitParticles(const float frametime);

	private:

		Vec3f GenerateRandomUnitVectorWithinCone(const Vec3f& coneDirection, const float angle);
		Vec3f GenerateRandomUnitVector();

		float GenerateValue(const float average, const float errorMargin);
		float NextRandomFloat();
		float GenerateRotation();
		void SortDepth();

	public:
		float m_particlePerSecond;
		float m_avgSpeed;
		float m_gravityCompliance;
		float m_avgLifeLength;
		float m_avgScale;

		float m_speedVariation{ 0.f };
		float m_lifeLengthVariation{ 0.f };
		float m_scaleVariation{ 0.f };

		bool m_randomRotation = false;
		Vec3f m_direction{ 0.f };
		float m_directionVariation{ 0.f };
		Vec3f m_center_offset{ 0.f };
		Vec3f m_center{ 0.f };

		std::random_device m_randDevice;
		std::mt19937 m_generator;
		std::uniform_real_distribution<double> m_distribution;

		std::shared_ptr<Texture2D> m_texture = { nullptr };
		LongMarch_Vector<Particle3D> m_particles;
	};
}