#pragma once
#include "engine/math/Geommath.h"
#include "VertexArray.h"
#include "Shader.h"

namespace longmarch
{
	struct Particle
	{
		Vec3f Position;
		Vec3f Speed;
		Vec3f Velocity, VelocityVariation;
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation = 0.0f;
		float SizeBegin, SizeEnd, SizeVariation;
		float LifeTime = 1.0f;
		float LifeRemaining = 1.0f;
		bool Active = false;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem();
		virtual ~ParticleSystem();

		virtual void OpenGLInit();
		virtual void Update(float ts);
		virtual void Render();
		virtual void Emit(const Particle& particle);
		void SetPoolSize(uint32_t size) { m_ParticlePoolSize = size; m_ParticlePool.resize(m_ParticlePoolSize); }

	protected:
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = 999;
		uint32_t m_ParticlePoolSize = 1000;

		struct BufferData
		{
			std::shared_ptr<VertexArray> ParticleVertexArray;
			std::shared_ptr<VertexBuffer> ParticleVertexBuffer;
			std::shared_ptr<IndexBuffer> ParticleIndexBuffer;
			std::shared_ptr<Shader> ParticleShader;
		};
		BufferData m_BufferData;
		bool m_init;
	};
}
