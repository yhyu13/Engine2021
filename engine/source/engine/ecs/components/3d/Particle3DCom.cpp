#include "engine-precompiled-header.h"
#include "Particle3DCom.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/audio/AudioManager.h"

namespace longmarch
{
	Particle3DCom::Particle3DCom(const EntityDecorator& _this)
		:
		BaseComponent(_this.GetVolatileWorld()),
		m_this(_this.GetEntity())
	{
	}

	void Particle3DCom::SetParticleSystem(const std::shared_ptr<ParticleSystem3D>& particleSystem)
	{
		LOCK_GUARD2();
		m_particleSystem = particleSystem;
	}

	void Particle3DCom::Update(const double frametime, const PerspectiveCamera* camera)
	{
		LOCK_GUARD2();
		
		m_particleSystem->Update(frametime, camera->GetWorldPosition());

		m_instancedDataList.clear();
		// Draw calls for particle systems
		// Collect data for all particles in a map [texture, vector of particle instanced data]	
		Renderer3D::ParticleInstanceData_CPU instanceData;
		for (auto& particle : m_particleSystem->GetParticles())
		{
			instanceData.models.push_back(GetModelViewMatrix(particle, camera));

			Vec4f textureOffsets(particle.m_currentTextureOffset.xy, particle.m_nextTextureOffset.xy);
			instanceData.textureOffsets.push_back(textureOffsets);

			instanceData.blendFactors.push_back(particle.m_blendFactor);
		}
		auto texture = m_particleSystem->GetTexture();
		instanceData.textureRows = texture->GetTextureRowCount();
		instanceData.entity = m_this;

		m_instancedDataList.emplace_back(texture, instanceData);
	}

	void Particle3DCom::Draw()
	{
		LOCK_GUARD2();
		if (m_render)
		{
			Renderer3D::DrawParticles(m_instancedDataList);
		}
	}

	void Particle3DCom::Draw(const std::function<void(const Renderer3D::ParticleInstanceDrawData&)>& drawFunc)
	{
		LOCK_GUARD2();
		if (m_render)
		{
			drawFunc(m_instancedDataList);
		}
	}

	void Particle3DCom::SetCenter(const Vec3f& center)
	{
		LOCK_GUARD2();
		m_particleSystem->SetCenter(center);
	}

	void Particle3DCom::SetRendering(bool b)
	{
		LOCK_GUARD2();
		m_render = b;
	}

	bool Particle3DCom::IsRendering() const
	{
		LOCK_GUARD2();
		return m_render;
	}

	void Particle3DCom::SetPPS(unsigned int count)
	{
		LOCK_GUARD2();
		m_particleSystem->m_particlePerSecond = count;
	}

	void Particle3DCom::JsonSerialize(Json::Value& value)
	{
	}

	void Particle3DCom::JsonDeserialize(const Json::Value& value)
	{
		if (value.isNull())
		{
			return;
		}

		Json::Value particleSystems = value["particle-systems"];
		auto& data = particleSystems;
		m_render = data["enable"].asBool();
		std::string type = data["type"].asString();

		float pps = data["pps"].asFloat();
		float speed = data["avg_speed"].asFloat();
		float gravity = data["gravity_compliance"].asFloat();
		float life = data["avg_life"].asFloat();
		float scale = data["avg_scale"].asFloat();
		float speedVariation = data["speed_variation"].asFloat();
		float lifeVariation = data["life_variation"].asFloat();
		float scaleVariation = data["scale_variation"].asFloat();
		auto& centerValue = data["center_offset"];
		Vec3f center(centerValue[0].asFloat(), centerValue[1].asFloat(), centerValue[2].asFloat());
		auto& dirValue = data["direction"];
		Vec3f direction(dirValue[0].asFloat(), dirValue[1].asFloat(), dirValue[2].asFloat());
		float dirVariation = data["direction_variation"].asFloat();
		std::string texturename = data["texture"].asString();
		bool randomRotation = data["randomize-rotation"].asBool();

		auto particleSystem = MemoryManager::Make_shared<ParticleSystem3D>(pps, speed, gravity, life, scale, texturename);
		particleSystem->SetCenter(center);
		particleSystem->SetDirection(direction, dirVariation);
		particleSystem->SetLifeLengthVariation(lifeVariation);
		particleSystem->SetScaleVariation(scaleVariation);
		particleSystem->SetSpeedvariation(speedVariation);
		if (randomRotation)
		{
			particleSystem->RandomizeRotation();
		}

		SetParticleSystem(particleSystem);
	}

	void Particle3DCom::ImGuiRender()
	{
		if (ImGui::TreeNode("Particle"))
		{
			ImGui::PushItemWidth(200);
			ImGui::InputFloat("##pps", &m_particleSystem->m_particlePerSecond, 10.0f, 50.0f, "pps = %.2f");

			ImGui::InputFloat("##life", &m_particleSystem->m_avgLifeLength, 0.01, 0.1, "life = %.2f");
			ImGui::SliderFloat("##life_deviation", &m_particleSystem->m_lifeLengthVariation, 0.0f, 1.0f, "life deviation = %.2f");

			ImGui::SliderFloat("##gravity", &m_particleSystem->m_gravityCompliance, 0.0, 1.0, "gravity compliance = %.2f");

			ImGui::InputFloat("##speed", &m_particleSystem->m_avgSpeed, 0.01, 0.1, "speed = %.3f");
			ImGui::SliderFloat("##speed_deviation", &m_particleSystem->m_speedVariation, 0.0f, 1.0f, "speed deviation = %.3f");

			ImGui::InputFloat("##scale", &m_particleSystem->m_avgScale, 0.01, 0.1, "scale = %.3f");
			ImGui::SliderFloat("##scale_deviation", &m_particleSystem->m_scaleVariation, 0.0f, 1.0f, "scale deviation = %.3f");

			ImGui::Text("direction");
			ImGui::SameLine();
			ImGui::PushItemWidth(39);
			ImGui::DragFloat("##direction_x", &(m_particleSystem->m_direction.x), 0.01f, 0.0, 1.0, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(39);
			ImGui::DragFloat("##direction_y", &(m_particleSystem->m_direction.y), 0.01f, 0.0, 1.0, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(39);
			ImGui::DragFloat("##direction_z", &(m_particleSystem->m_direction.z), 0.01f, 0.0, 1.0, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SliderFloat("##direction_deviation", &m_particleSystem->m_directionVariation, 0.0f, 1.0f, "direction deviation = %.3f");

			ImGui::Checkbox("Randomize rotation", &m_particleSystem->m_randomRotation);

			ImGui::PopItemWidth();

			ImGui::TreePop();
		}
	}

	Mat4 Particle3DCom::GetModelViewMatrix(const Particle3D& particle, const PerspectiveCamera* camera)
	{
		Mat4 view = camera->GetViewMatrix();
		Mat4 model(1.0f);
		model[0][0] = view[0][0];
		model[0][1] = view[1][0];
		model[0][2] = view[2][0];

		model[1][0] = view[0][1];
		model[1][1] = view[1][1];
		model[1][2] = view[2][1];

		model[2][0] = view[0][2];
		model[2][1] = view[1][2];
		model[2][2] = view[2][2];

		Geommath::SetTranslation(model, particle.m_position);
		model = view * model;
		Geommath::SetRotation(model, Geommath::FromAxisRot(glm::radians(particle.m_rotation), Vec3f(0.0, 0.0, 1.0)));
		Geommath::SetScale(model, Vec3f(particle.m_scale)); 
		return model;
	}
}