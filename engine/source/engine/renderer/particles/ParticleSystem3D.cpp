#include "engine-precompiled-header.h"

#include "ParticleSystem3D.h"
#include "../../core/asset-manager/ResourceManager.h"

namespace longmarch
{
	ParticleSystem3D::ParticleSystem3D()
		: m_particlePerSecond(0.0f),
		m_avgSpeed(0.0f),
		m_gravityCompliance(0.0f),
		m_avgLifeLength(0.0f),
		m_avgScale(0.0f),
		m_randDevice(),
		m_generator(m_randDevice()),
		m_distribution(0.0, 1.0)
	{
		std::srand((unsigned)time(NULL));
	}

	ParticleSystem3D::ParticleSystem3D(float _pps, float _avgSpeed, float _gravityCompliance, float _avgLifeLength, float _avgScale, const std::string& texturename)
		: m_particlePerSecond(_pps),
		m_avgSpeed(_avgSpeed),
		m_gravityCompliance(_gravityCompliance),
		m_avgLifeLength(_avgLifeLength),
		m_avgScale(_avgScale),
		m_randDevice(),
		m_generator(m_randDevice()),
		m_distribution(0.0, 1.0)
	{
		std::srand((unsigned)time(NULL));

		m_texture = ResourceManager<Texture2D>::GetInstance()->TryGet(texturename)->Get();
	}

	void ParticleSystem3D::RandomizeRotation()
	{
		m_randomRotation = true;
	}

	void ParticleSystem3D::SetDirection(const Vec3f& _direction, const float _deviation)
	{
		m_direction = _direction;
		m_directionVariation = (float)(_deviation * std::_Pi);
	}

	void ParticleSystem3D::SetSpeedvariation(const float variation)
	{
		m_speedVariation = variation;
	}

	void ParticleSystem3D::SetLifeLengthVariation(const float variation)
	{
		m_lifeLengthVariation = variation;
	}

	void ParticleSystem3D::SetScaleVariation(const float variation)
	{
		m_scaleVariation = variation;
	}

	void ParticleSystem3D::SetCenter(const Vec3f& center)
	{
		m_center = center;
	}

	void ParticleSystem3D::Update(const float frametime, const Vec3f& cameraPosition)
	{
		EmitParticles(frametime);

		std::vector<Particle3D>::iterator itr = m_particles.begin();
		while (itr != m_particles.end())
		{
			bool stillAlive = itr->Update(frametime, cameraPosition);
			if (!stillAlive)
			{
				itr = m_particles.erase(itr);
			}
			else
			{
				++itr;
			}
		}

		Sort();
	}

	void ParticleSystem3D::EmitParticles(const float frametime)
	{
		float particlesToCreate = m_particlePerSecond * frametime;
		int count = std::floor(particlesToCreate);
		//static bool render = true;
		/*if (render)
		{
			int count = 10;
			for (int i = 0; i < count; i++)
			{
				Vec3f velocity;
				if (m_direction.x != 0.0f || m_direction.y != 0.0f || m_direction.z != 0.0f)
				{
					velocity = GenerateRandomUnitVectorWithinCone(m_direction, m_directionVariation);
				}
				else
				{
					velocity = GenerateRandomUnitVector();
				}

				velocity = glm::normalize(velocity);
				velocity *= GenerateValue(m_avgSpeed, m_speedVariation);
				float scale = GenerateValue(m_avgScale, m_scaleVariation);
				float lifeLength = GenerateValue(m_avgLifeLength, m_lifeLengthVariation);
				Particle3D particle(m_center, velocity, m_gravityCompliance, lifeLength, GenerateRotation(), scale, m_texture->GetTextureRowCount());
				m_particles.push_back(particle);
			}

			render = false;
		}*/

		for (int i = 0; i < count; i++)
		{
			Vec3f velocity;
			if (m_direction.x != 0.0f || m_direction.y != 0.0f || m_direction.z != 0.0f)
			{
				velocity = GenerateRandomUnitVectorWithinCone(m_direction, m_directionVariation);
			}
			else
			{
				velocity = GenerateRandomUnitVector();
			}

			velocity = glm::normalize(velocity);
			velocity *= GenerateValue(m_avgSpeed, m_speedVariation);
			float scale = GenerateValue(m_avgScale, m_scaleVariation);
			float lifeLength = GenerateValue(m_avgLifeLength, m_lifeLengthVariation);
			Particle3D particle(m_center, velocity, m_gravityCompliance, lifeLength, GenerateRotation(), scale, m_texture->GetTextureRowCount());
			m_particles.push_back(particle);
		}
	}

	LongMarch_Vector<Particle3D>& ParticleSystem3D::GetParticles()
	{
		return m_particles;
	}

	std::shared_ptr<Texture2D> ParticleSystem3D::GetTexture()
	{
		return m_texture;
	}

	/*
		Get the next pseudo-random float, uniformly distributed float value between 0.0 and 1.0
	*/
	float ParticleSystem3D::NextRandomFloat()
	{
		return m_distribution(m_generator);
	}

	float ParticleSystem3D::GenerateValue(const float average, const float errorMargin)
	{
		float offset = (NextRandomFloat() - 0.5f) * 2.0f * errorMargin;
		return average + offset;
	}

	Vec3f ParticleSystem3D::GenerateRandomUnitVectorWithinCone(const Vec3f& coneDirection, const float angle)
	{
		float cosAngle = (float)std::cosf(angle);

		std::random_device randDevice;
		std::mt19937 generator(randDevice());
		std::uniform_real_distribution<double> distribution(0.0f, 1.0f);

		float theta = (float)(distribution(generator) * 2.0f * std::_Pi);
		float y = cosAngle + (distribution(generator) * (1.0 - cosAngle));
		float rootOneMinusZSquarred = (float)(std::sqrtf(1.0f - (y * y)));
		float x = (float)(rootOneMinusZSquarred * std::cosf(theta));
		float z = (float)(rootOneMinusZSquarred * std::sinf(theta));

		Vec4f direction(x, y, z, 1.0f);
		if (coneDirection.x != 0 || coneDirection.z != 0 || (coneDirection.y != 1 && coneDirection.y != -1))
		{
			Vec3f rotateAxis = glm::cross(coneDirection, Vec3f(0.0f, 1.0f, 0.0f));
			rotateAxis = glm::normalize(rotateAxis);
			float rotateAngle = (float)std::acosf(glm::dot(coneDirection, Vec3f(0.0f, 1.0f, 0.0f)));
			Mat4 rotationMatrix(1.0f);
			rotationMatrix = glm::rotate(rotationMatrix, -rotateAngle, rotateAxis);
			direction = rotationMatrix * direction;
		}
		else if (coneDirection.y == -1)
		{
			direction.y *= -1;
		}
		return Vec3f(direction);
	}

	Vec3f ParticleSystem3D::GenerateRandomUnitVector()
	{
		float theta = (float)(NextRandomFloat() * 2.0f * std::_Pi);
		float z = (NextRandomFloat() * 2.0f) - 1.0f;
		float rootOneMinusZSquarred = (float)(std::sqrtf(1.0f - (z * z)));
		float x = (float)(rootOneMinusZSquarred * std::cosf(theta));
		float y = (float)(rootOneMinusZSquarred * std::sinf(theta));
		return Vec3f(x, y, z);
	}

	float ParticleSystem3D::GenerateRotation()
	{
		if (m_randomRotation)
		{
			return NextRandomFloat() * 360.0f;
		}
		else
		{
			return 0.0f;
		}
	}

	void ParticleSystem3D::Sort()
	{
		struct Particle3DObj_ComparatorLesser // used in priority queue that puts objects in greater distances at front
		{
			bool operator()(const Particle3D& lhs, const Particle3D& rhs) noexcept
			{
				return lhs.m_distance < rhs.m_distance;
			}
		};

		std::priority_queue<Particle3D, LongMarch_Vector<Particle3D>, Particle3DObj_ComparatorLesser> depth_sorted_particle_obj;
		for (auto& particle : m_particles)
		{
			depth_sorted_particle_obj.emplace(particle);
		}
		m_particles.clear();
		while (!depth_sorted_particle_obj.empty())
		{
			m_particles.push_back(depth_sorted_particle_obj.top());
			depth_sorted_particle_obj.pop();
		}
	}
}