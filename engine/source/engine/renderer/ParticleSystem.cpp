#include "engine-precompiled-header.h"
#include "ParticleSystem.h"
#include "Renderer2D.h"
#include "engine/core/utility/Random.h"
#include <glm/gtx/compatibility.hpp>

namespace longmarch {
	ParticleSystem::ParticleSystem()
	{
		m_ParticlePool.resize(m_ParticlePoolSize);
		m_init = false;
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	void ParticleSystem::OpenGLInit()
	{
		if (!m_init)
		{
			m_init = true;
			m_BufferData.ParticleVertexArray = VertexArray::Create();

			float vertices[] =
			{
				-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f,	1.0f, 0.0f,
				 0.5f,  0.5f, 0.0f,	1.0f, 1.0f,
				-0.5f,  0.5f, 0.0f,	0.0f, 1.0f
			};

			m_BufferData.ParticleVertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
			m_BufferData.ParticleVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
				});
			m_BufferData.ParticleVertexArray->AddVertexBuffer(m_BufferData.ParticleVertexBuffer);

			uint8_t quadIndices[6] = { 0, 1, 2, 2, 3, 0 };
			m_BufferData.ParticleIndexBuffer = IndexBuffer::Create(quadIndices, sizeof(quadIndices), sizeof(uint8_t));
			m_BufferData.ParticleVertexArray->SetIndexBuffer(m_BufferData.ParticleIndexBuffer);
		}
	}

	void ParticleSystem::Update(float ts)
	{
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.Active)
				continue;

			if (particle.LifeRemaining <= 0.0f)
			{
				particle.Active = false;
				continue;
			}

			particle.LifeRemaining -= ts;
			particle.Position += particle.Velocity * ts;
			particle.Rotation += 0.01f * ts;
		}
	}

	void ParticleSystem::Render()
	{
		OpenGLInit();

		if (!m_BufferData.ParticleVertexArray)
			return;

		for (auto& particle : m_ParticlePool)
		{
			if (!particle.Active)
				continue;

			float life = particle.LifeRemaining / particle.LifeTime;
			glm::vec4 color = glm::lerp(particle.ColorBegin, particle.ColorEnd, life);
			float size = glm::lerp(particle.SizeBegin, particle.SizeEnd, life);

			//Renderer2D::DrawQuad(m_BufferData.ParticleVertexArray, m_BufferData.ParticleIndexBuffer, particle.Position, glm::vec2(size), particle.Rotation, color);
			Renderer2D::AddBatch(particle.Position, glm::vec2(size), particle.Rotation, color, std::vector<glm::vec3>(), std::vector<glm::vec2>());
		}
	}

	void ParticleSystem::Emit(const Particle& particleProps)
	{
		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.Active = true;
		particle.Position = particleProps.Position;
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		particle.Velocity = particleProps.Velocity;
		particle.Velocity += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

		particle.Speed = particleProps.Speed;

		particle.ColorBegin = particleProps.ColorBegin;
		particle.ColorEnd = particleProps.ColorEnd;

		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;
		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;

		m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	}
}