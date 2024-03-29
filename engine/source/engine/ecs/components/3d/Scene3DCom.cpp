#include "engine-precompiled-header.h"
#include "Scene3DCom.h"
#include "engine/scene-graph/Scene3DManager.h"
#include "engine/ui/ImGuiUtil.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Animation3DCom.h"

longmarch::Scene3DCom::Scene3DCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.Volatile().GetWorld()),
	m_this(_this.GetEntity())
{
	//m_atomic_lock_enabled = true;
}

void longmarch::Scene3DCom::SetSceneData(const Scene3DCom::SceneDataRef& obj)
{
	LOCK_GUARD();
	if (obj)
	{
		m_objDatasRef = obj->Copy();
	}
	else
	{
		m_objDatasRef = nullptr;
	}
}

void longmarch::Scene3DCom::SetSceneData(const ResourceManager<Scene3DNode>::ResourceHandle& handle)
{
	LOCK_GUARD();
	m_objDatasHandle = handle;
}

Scene3DCom::SceneDataRef& longmarch::Scene3DCom::GetSceneData(bool waitOnHandle)
{
	LOCK_GUARD();
	if (m_objDatasHandle && m_objDatasHandle->IsFutureValid())
	{
		if (waitOnHandle)
		{
			if (auto ptr = m_objDatasHandle->Get(); ptr)
			{
				m_objDatasRef = ptr->Copy();
			}
		}
		else
		{
			if (auto ptr = m_objDatasHandle->TryGet(); ptr)
			{
				m_objDatasRef = ptr->Copy();
			}
		}
	}
	return m_objDatasRef;
}

void longmarch::Scene3DCom::SetShaderName(const std::string& name)
{
	LOCK_GUARD();
	m_shaderName = name;
}

bool longmarch::Scene3DCom::IsVisible() const
{
	LOCK_GUARD();
	return m_visible;
}

void longmarch::Scene3DCom::SetVisiable(bool b)
{
	LOCK_GUARD();
	m_visible = b;
}

bool longmarch::Scene3DCom::IsHideInGame() const
{
	LOCK_GUARD();
	return m_hideInGame;
}

void longmarch::Scene3DCom::SetHideInGame(bool b)
{
	LOCK_GUARD();
	m_hideInGame = b;
}

bool longmarch::Scene3DCom::IsCastShadow() const
{
	LOCK_GUARD();
	return m_castShadow;
}

void longmarch::Scene3DCom::SetCastShadow(bool b)
{
	LOCK_GUARD();
	m_castShadow = b;
}

bool longmarch::Scene3DCom::IsCastReflection() const
{
	LOCK_GUARD();
	return m_castReflection;
}

void longmarch::Scene3DCom::SetCastReflection(bool b)
{
	LOCK_GUARD();
	m_castReflection = b;
}

bool longmarch::Scene3DCom::IsParticleRenderType() const
{
	LOCK_GUARD();
	return m_renderType == Renderer3D::RENDER_TYPE::PARTICLE;
}

void longmarch::Scene3DCom::SetParticleRenderType(bool v)
{
	LOCK_GUARD();
	if (v)
	{
		m_renderType = Renderer3D::RENDER_TYPE::PARTICLE;
	}
}

bool longmarch::Scene3DCom::IsTranslucenctRenderType() const
{
	LOCK_GUARD();
	return m_renderType == Renderer3D::RENDER_TYPE::TRANSLUCENT;
}

void longmarch::Scene3DCom::SetTranslucenctRenderType(bool v)
{
	LOCK_GUARD();
	if (v)
	{
		m_renderType = Renderer3D::RENDER_TYPE::TRANSLUCENT;
	}
}

int longmarch::Scene3DCom::GetTranslucencySortPriority() const
{
	LOCK_GUARD();
	return m_translucencySortPriority;
}

void longmarch::Scene3DCom::SetTranslucencySortPriority(int v)
{
	LOCK_GUARD();
	m_translucencySortPriority = v;
}

void longmarch::Scene3DCom::SetShouldDraw(bool b, bool override)
{
	LOCK_GUARD();
	(override) ? m_shoudlDraw = b : m_shoudlDraw &= b;
}

bool longmarch::Scene3DCom::GetShouldDraw() const
{
	LOCK_GUARD();
	return m_shoudlDraw;
}

void longmarch::Scene3DCom::Draw()
{
	GetSceneData(false);
	{
		LOCK_GUARD();
		if (m_shoudlDraw && m_objDatasRef)
		{
			const auto trans = EntityDecorator{ m_this , m_world }.GetComponent<Transform3DCom>();
			const auto& shaderName = m_shaderName;
			Renderer3D::Draw(m_this, m_objDatasRef, trans->GetModelTr(), trans->GetPrevModelTr(), shaderName);
		}
		// Automatically reset the drawable flag on render completion (always set shouldDraw before Draw)
		m_shoudlDraw = true;
	}
}

void longmarch::Scene3DCom::Draw(const std::function<void(const Renderer3D::RenderData_CPU&)>& drawFunc)
{
	GetSceneData(false);
	{
		LOCK_GUARD();
		if (m_shoudlDraw && m_objDatasRef)
		{
			auto trans = EntityDecorator{ m_this , m_world }.GetComponent<Transform3DCom>();
			const auto& _sceneData = *m_objDatasRef;
			for (const auto& [level, data] : _sceneData)
			{
				auto& mesh = data->meshData;
				auto& mat = data->material;
				drawFunc(Renderer3D::RenderData_CPU{ (Entity)m_this, mesh, mat, trans->GetModelTr(), trans->GetPrevModelTr() });
			}
		}
		// Automatically reset the drawable flag on render completion (always set shouldDraw before Render)
		m_shoudlDraw = true;
	}
}

void longmarch::Scene3DCom::JsonSerialize(Json::Value& value) const
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD();
	{
		Json::Value output;
		output["id"] = "Scene3DCom";
		auto& val = output["value"];
		static const auto& _default = Scene3DCom();

		if (m_visible != _default.m_visible)
		{
			val["visible"] = m_visible;
		}
		if (m_hideInGame != _default.m_hideInGame)
		{
			val["hide_in_game"] = m_hideInGame;
		}
		if (m_castShadow != _default.m_castShadow)
		{
			val["castShadow"] = m_castShadow;
		}
		if (m_castReflection != _default.m_castReflection)
		{
			val["castReflection"] = m_castReflection;
		}
		if (m_renderType != _default.m_renderType)
		{
			val["render-type"] = LongMarch_ToUnderlying(m_renderType);
		}
		if (m_translucencySortPriority != _default.m_translucencySortPriority)
		{
			val["translucency_order"] = m_translucencySortPriority;
		}
		if (m_objDatasRef)
		{
			const auto& sceneData = *m_objDatasRef;
			if (!sceneData.empty())
			{
				// Serialize mesh name
				auto mesh_name = sceneData.Name();
				val["mesh"] = mesh_name;
				// Only serialize material for non prefabs
				if (mesh_name.find("prefab") == std::string::npos)
				{
					const auto& allMat = sceneData.GetAllMaterial();
					if (allMat.size() > 0)
					{
						if (allMat.size() > 1)
						{
							PRINT("Could only serialize non-prefab scene object with one material!");
						}
						auto& val2 = val["mat"];
						const auto& mat = allMat[0];
						static const auto& _default = Material();

						val2[(mat->emissive) ? "Kd" : "albedo"] = LongMarch_ArrayToJsonValue(mat->Kd, 3);
						if (mat->alpha != _default.alpha)
						{
							val["alpha"] = mat->alpha;
						}
						if (mat->emissive != _default.emissive)
						{
							val2["emissive"] = mat->emissive;
						}
						if (mat->metallic != _default.metallic)
						{
							val2["metallic"] = mat->metallic;
						}
						if (mat->roughness != _default.roughness)
						{
							val2["roughness"] = mat->roughness;
						}
						auto rm_texture2D = ResourceManager<Texture2D>::GetInstance();
						if (mat->textures.has_albedo())
						{
							val2["albedo_map"] = rm_texture2D->GetName(mat->textures.albedo_texture);
						}
						if (mat->textures.has_normal())
						{
							val2["normal_map"] = rm_texture2D->GetName(mat->textures.normal_texture);
						}
						if (mat->textures.has_metallic())
						{
							val2["metallic_map"] = rm_texture2D->GetName(mat->textures.metallic_texture);
						}
						if (mat->textures.has_roughness())
						{
							val2["roughness_map"] = rm_texture2D->GetName(mat->textures.roughness_texture);
						}
						if (mat->textures.has_ao())
						{
							val2["ao_map"] = rm_texture2D->GetName(mat->textures.ao_texture);
						}
					}
				}
			}
		}
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::Scene3DCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	std::string mesh_name;
	if (auto& val = value["mesh"]; !val.isNull())
	{
		mesh_name = val.asString();
		Scene3DManager::GetInstance()->LoadSceneNodeToEntity(EntityDecorator{ m_this, m_world }, mesh_name);
	}
	{
		if (auto& val = value["visible"]; !val.isNull())
		{
			m_visible = val.asBool();
		}
		if (auto& val = value["hide_in_game"]; !val.isNull())
		{
			m_hideInGame = val.asBool();
		}
		if (auto& val = value["castShadow"]; !val.isNull())
		{
			m_castShadow = val.asBool();
		}
		if (auto& val = value["castReflection"]; !val.isNull())
		{
			m_castReflection = val.asBool();
		}
		if (auto& val = value["render-type"]; !val.isNull())
		{
			m_renderType = Renderer3D::RENDER_TYPE(val.asInt());
		}
		if (auto& val = value["translucency_order"]; !val.isNull())
		{
			m_translucencySortPriority = val.asBool();
		}
		if (auto& val = value["mat"]; !val.isNull() && mesh_name.find("prefab") == std::string::npos)
		{
			const auto& sceneData = *(GetSceneData(true));
			if (sceneData.empty())
			{
				return;
			}
			bool isLight = false;
			if (auto& val2 = val["Kd"]; !val2.isNull())
			{
				isLight = true;
				ASSERT(val2.size() == 3, "Light intensity must be a vec3!");
				auto value = (glm::max)(Vec3f(val2[0].asFloat(), val2[1].asFloat(), val2[2].asFloat()), 0.01f); //  clamp luminance

				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->Kd = value;
				});
			}
			if (auto& val2 = val["albedo"]; !val2.isNull())
			{
				ASSERT(!isLight, "Albedo color does not apply to light!");
				ASSERT(val2.size() == 3, "albedo must be a vec3!");
				auto value = glm::clamp(Vec3f(val2[0].asFloat(), val2[1].asFloat(), val2[2].asFloat()), 0.05f, 0.92f); // NTSC safe rgb range
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->Kd = value;
				});
			}
			if (auto& val2 = val["alpha"]; !val2.isNull())
			{
				auto value = glm::clamp(val2.asFloat(), 0.f, 1.f);
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->alpha = value;
				});
			}
			if (auto& val2 = val["metallic"]; !val2.isNull())
			{
				auto value = glm::clamp(val2.asFloat(), 0.01f, .99f);
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->metallic = value;
				});
			}
			if (auto& val2 = val["roughness"]; !val2.isNull())
			{
				auto value = glm::clamp(val2.asFloat(), 0.01f, .99f);
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->roughness = value;
				});
			}
			if (auto& val2 = val["emissive"]; !val2.isNull())
			{
				ASSERT(isLight, "Emissive material requires 'Kd' to be set!");
				auto value = val2.asBool();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->emissive = value;
				});
			}
			if (auto& val2 = val["albedo_map"]; !val2.isNull())
			{
				auto value = val2.asString();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->SetTexture(value, value, Material::MAT_TEXTURE_TYPE::ALBEDO);
				});
			}
			if (auto& val2 = val["normal_map"]; !val2.isNull())
			{
				auto value = val2.asString();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->SetTexture(value, value, Material::MAT_TEXTURE_TYPE::NORMAL);
				});
			}
			if (auto& val2 = val["metallic_map"]; !val2.isNull())
			{
				auto value = val2.asString();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->SetTexture(value, value, Material::MAT_TEXTURE_TYPE::METALLIC);
				});
			}
			if (auto& val2 = val["roughness_map"]; !val2.isNull())
			{
				auto value = val2.asString();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->SetTexture(value, value, Material::MAT_TEXTURE_TYPE::ROUGHNESS);
				});
			}
			if (auto& val2 = val["ao_map"]; !val2.isNull())
			{
				auto value = val2.asString();
				sceneData.ModifyAllMaterial([&value](Material* mat) {
					mat->SetTexture(value, value, Material::MAT_TEXTURE_TYPE::BACKEDAO);
				});
			}
		}
	}
}

void longmarch::Scene3DCom::ImGuiRender()
{
	if (ImGui::TreeNode("Scene3D"))
	{
		constexpr int yoffset_item = 5;
		// Scene Mesh
		{
			const auto meshName = (m_objDatasRef) ? m_objDatasRef->Name() : "None";
			auto vs = ResourceManager<Scene3DNode>::GetInstance()->GetAllNames();
			vs.insert(vs.begin(), std::string("None"));
			int index = MAX(LongMarch_findFristIndex(vs, meshName), 0);
			const auto vc = LongMarch_StrVec2ConstChar(vs);
			if (ImGui::Combo("Scene Mesh", &index, &vc[0], vc.size()))
			{
				if (const std::string& name = vs[index]; name != "None")
				{
					Scene3DManager::GetInstance()->LoadSceneNodeToEntity(EntityDecorator{ m_this, m_world }, name);
				}
				else
				{
					m_objDatasRef = nullptr;
				}
			}
		}
		if (!m_objDatasRef)
		{
			ImGui::Separator();
			ImGui::TreePop();
			return;
		}
		if (ImGui::TreeNode("Rendering"))
		{
			{
				// Visible
				bool val = m_visible;
				if (ImGui::Checkbox("Visible", &val))
				{
					m_visible = val;
				}
			}
			{
				// Hide in Game
				bool val = m_hideInGame;
				if (ImGui::Checkbox("Hide In Game", &val))
				{
					m_hideInGame = val;
				}
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				// Cast Shadow
				bool val = m_castShadow;
				if (ImGui::Checkbox("Cast Shadow", &val))
				{
					m_castShadow = val;
				}
			}
			{
				// Cast Reflection
				bool val = m_castReflection;
				if (ImGui::Checkbox("Cast Reflection", &val))
				{
					m_castReflection = val;
				}
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				// Use translucency rendering or not
				bool val = IsParticleRenderType();
				if (ImGui::Checkbox("Particle Rendering", &val))
				{
					SetParticleRenderType(val);
				}
				ImGuiUtil::InlineHelpMarker("Use particle rendering or not.");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				// Use translucency rendering or not
				bool val = IsTranslucenctRenderType();
				if (ImGui::Checkbox("Translucenct Rendering", &val))
				{
					SetTranslucenctRenderType(val);
				}
				ImGuiUtil::InlineHelpMarker("Use translucency rendering or not.");
			}
			{
				// Translucency sort priority, positive draw at front, negative draw at back 
				int val = m_translucencySortPriority;
				if (ImGui::InputInt("Translucency Sort Priority", &val))
				{
					m_translucencySortPriority = val;
				}
				ImGuiUtil::InlineHelpMarker("Translucency sort priority, positive draw at front, negative draw at back. This can be used to sort among particles and transparent objects.");
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		// Material
		for (auto& [level, data] : *m_objDatasRef)
		{
			auto levelname = "Level " + Str(level) + " : " + data->meshName;
			if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Level " + Str(level) + " : " + data->meshName, levelname + Str(data.get()))))
			{
				auto& mesh = data->meshData;
				auto& mat = data->material;
				ENGINE_EXCEPT_IF(!mesh || !mat, L"Trying to ImGuiRender a nullptr mesh or material!");
				if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Material", "mat" + Str(data.get()))))
				{
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Textures", "tex" + Str(data.get()))))
					{
						auto rm_texture2D = ResourceManager<Texture2D>::GetInstance();
						auto vs = rm_texture2D->GetAllNames();
						vs.insert(vs.begin(), std::string("None"));
						const auto vc = LongMarch_StrVec2ConstChar(vs);
						{
							const auto texture_name = rm_texture2D->GetName(mat->textures.albedo_texture);
							int index = MAX(LongMarch_findFristIndex(vs, texture_name), 0);
							if (ImGui::Combo(LongMarch_ImGuiHashTagName("Albedo", "alb_tex" + Str(data.get())), &index, &vc[0], vc.size()))
							{
								if (const std::string& name = vs[index]; name != "None")
								{
									mat->SetTexture(name, name, Material::MAT_TEXTURE_TYPE::ALBEDO);
								}
								else
								{
									mat->UnsetTexture(Material::MAT_TEXTURE_TYPE::ALBEDO);
								}
							}
							ImGuiUtil::TextureViewerWithZoom(mat->textures.albedo_texture->TryGet());
						}
						ImGui::Dummy(ImVec2(0, yoffset_item));
						{
							const auto texture_name = rm_texture2D->GetName(mat->textures.normal_texture);
							int index = MAX(LongMarch_findFristIndex(vs, texture_name), 0);
							if (ImGui::Combo(LongMarch_ImGuiHashTagName("Normal", "nrm_tex" + Str(data.get())), &index, &vc[0], vc.size()))
							{
								if (const std::string& name = vs[index]; name != "None")
								{
									mat->SetTexture(name, name, Material::MAT_TEXTURE_TYPE::NORMAL);
								}
								else
								{
									mat->UnsetTexture(Material::MAT_TEXTURE_TYPE::NORMAL);
								}
							}
							ImGuiUtil::TextureViewerWithZoom(mat->textures.normal_texture->TryGet());
						}
						ImGui::Dummy(ImVec2(0, yoffset_item));
						{
							const auto texture_name = rm_texture2D->GetName(mat->textures.metallic_texture);
							int index = MAX(LongMarch_findFristIndex(vs, texture_name), 0);
							if (ImGui::Combo(LongMarch_ImGuiHashTagName("Metallic", "met_tex" + Str(data.get())), &index, &vc[0], vc.size()))
							{
								if (const std::string& name = vs[index]; name != "None")
								{
									mat->SetTexture(name, name, Material::MAT_TEXTURE_TYPE::METALLIC);
								}
								else
								{
									mat->UnsetTexture(Material::MAT_TEXTURE_TYPE::METALLIC);
								}
							}
							ImGuiUtil::TextureViewerWithZoom(mat->textures.metallic_texture->TryGet());
						}
						ImGui::Dummy(ImVec2(0, yoffset_item));
						{
							const auto texture_name = rm_texture2D->GetName(mat->textures.roughness_texture);
							int index = MAX(LongMarch_findFristIndex(vs, texture_name), 0);
							if (ImGui::Combo(LongMarch_ImGuiHashTagName("Roughness", "roug_tex" + Str(data.get())), &index, &vc[0], vc.size()))
							{
								if (const std::string& name = vs[index]; name != "None")
								{
									mat->SetTexture(name, name, Material::MAT_TEXTURE_TYPE::ROUGHNESS);
								}
								else
								{
									mat->UnsetTexture(Material::MAT_TEXTURE_TYPE::ROUGHNESS);
								}
							}
							ImGuiUtil::TextureViewerWithZoom(mat->textures.roughness_texture->TryGet());
						}
						ImGui::Dummy(ImVec2(0, yoffset_item));
						{
							const auto texture_name = rm_texture2D->GetName(mat->textures.ao_texture);
							int index = MAX(LongMarch_findFristIndex(vs, texture_name), 0);
							if (ImGui::Combo(LongMarch_ImGuiHashTagName("Ambient Occlusion", "ao_tex" + Str(data.get())), &index, &vc[0], vc.size()))
							{
								if (const std::string& name = vs[index]; name != "None")
								{
									mat->SetTexture(name, name, Material::MAT_TEXTURE_TYPE::BACKEDAO);
								}
								else
								{
									mat->UnsetTexture(Material::MAT_TEXTURE_TYPE::BACKEDAO);
								}
							}
							ImGuiUtil::TextureViewerWithZoom(mat->textures.ao_texture->TryGet());
						}
						ImGui::Separator();
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Kd", "kd_tree" + Str(data.get()))))
					{
						auto kd = mat->Kd;
						ImGui::ColorPicker3(LongMarch_ImGuiHashTagName("Kd", "kd_picker" + Str(data.get())), &kd[0], ImGuiColorEditFlags_NoAlpha);
						{
							float speed = 0.01f;
							ImGui::DragFloat(LongMarch_ImGuiHashTagName("R", "kd_picker_r" + Str(data.get())), &kd[0], speed, 0.f, 1.f);
							ImGui::DragFloat(LongMarch_ImGuiHashTagName("G", "kd_picker_g" + Str(data.get())), &kd[1], speed, 0.f, 1.f);
							ImGui::DragFloat(LongMarch_ImGuiHashTagName("B", "kd_picker_b" + Str(data.get())), &kd[2], speed, 0.f, 1.f);
						}
						mat->Kd = kd;
						ImGui::Separator();
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Alpha", "alpha_tree" + Str(data.get()))))
					{
						float speed = 0.01f;
						auto alpha = mat->alpha;
						if (ImGui::DragFloat(LongMarch_ImGuiHashTagName("Alpha", "alpha_picker" + Str(data.get())), &alpha, speed, 0.0f, 1.0f))
						{
							mat->alpha = alpha;
						}
						ImGui::Separator();
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Metallic", "met_tree" + Str(data.get()))))
					{
						float speed = 0.01f;
						auto metallic = mat->metallic;
						if (ImGui::DragFloat(LongMarch_ImGuiHashTagName("Metallic", "met_picker" + Str(data.get())), &metallic, speed, 0.0f, 1.0f))
						{
							mat->metallic = metallic;
						}
						ImGui::Separator();
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Roughness", "roug_tree" + Str(data.get()))))
					{
						float speed = 0.01f;
						auto roughness = mat->roughness;
						if (ImGui::DragFloat(LongMarch_ImGuiHashTagName("Roughness", "roug_picker" + Str(data.get())), &roughness, speed, 0.0f, 1.0f))
						{
							mat->roughness = roughness;
						}
						ImGui::Separator();
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(LongMarch_ImGuiHashTagName("Emssive", "emssive_tree" + Str(data.get()))))
					{
						auto emissive = mat->emissive;
						if (ImGui::Checkbox(LongMarch_ImGuiHashTagName("Emssive", "emssive_checker" + Str(data.get())), &emissive))
						{
							mat->emissive = emissive;
						}
						ImGui::Separator();
						ImGui::TreePop();
					}
					ImGui::Separator();
					ImGui::TreePop();
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}
