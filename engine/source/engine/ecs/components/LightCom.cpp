#include "engine-precompiled-header.h"
#include "LightCom.h"
#include "engine/renderer/Renderer3D.h"

#define USE_DIRECTIONAL_LIGHT_COMPARE_TEXTURE
#define USE_POINT_LIGHT_ARRAY_TEXTURE

void longmarch::LightCom::JsonSerialize(Json::Value& value)
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD2();
	{
		Json::Value output;
		output["id"] = "LightCom";
		auto& val = output["value"];
		static const auto& _default = LightCom();

		if (attenuation != _default.attenuation)
		{
			val["attenuation"] = LongMarch_ArrayToJsonValue(attenuation, 4);
		}
		if (collisionRadius != _default.collisionRadius)
		{
			val["collision_radius"] = collisionRadius;
		}
		{
			auto& val2 = val["shadow"];
			if (shadow.dropOffDistance != _default.shadow.dropOffDistance)
			{
				val2["dropOff"] = shadow.dropOffDistance;
			}
			if (shadow.nearZ != _default.shadow.nearZ)
			{
				val2["nearZ"] = shadow.nearZ;
			}
			if (shadow.farZ != _default.shadow.farZ)
			{
				val2["farZ"] = shadow.farZ;
			}
			if (shadow.bCastShadow != _default.shadow.bCastShadow)
			{
				val2["enable"] = shadow.bCastShadow;
			}
			if (shadow.dimension != _default.shadow.dimension)
			{
				val2["dimension"] = shadow.dimension;
			}
			if (shadow.shadowAlgorithmMode != _default.shadow.shadowAlgorithmMode)
			{
				val2["algorithm"] = s_shadowAlgorithmModes[shadow.shadowAlgorithmMode];
			}
			if (shadow.depthBiasHigherBound != _default.shadow.depthBiasHigherBound)
			{
				val2["depthBiasHigherBound"] = shadow.depthBiasHigherBound;
			}
			if (shadow.depthBiasMultiplier != _default.shadow.depthBiasMultiplier)
			{
				val2["depthBiasMultiplier"] = shadow.depthBiasMultiplier;
			}
			if (shadow.nrmBiasMultiplier != _default.shadow.nrmBiasMultiplier)
			{
				val2["nrmBiasMultiplier"] = shadow.nrmBiasMultiplier;
			}
			if (shadow.bEnableGaussianBlur != _default.shadow.bEnableGaussianBlur)
			{
				val2["enable_guassian_blur"] = shadow.bEnableGaussianBlur;
			}
			if (shadow.bEnableTransparentShadow != _default.shadow.bEnableTransparentShadow)
			{
				val2["enable_transparent_shadow"] = shadow.bEnableTransparentShadow;
			}
			if (shadow.gaussianKernal != _default.shadow.gaussianKernal)
			{
				val2["guassian_kernel_size"] = shadow.gaussianKernal;
			}
			if (shadow.backBufferDimension != _default.shadow.backBufferDimension)
			{
				val2["dimension_guassian_backbuffer"] = shadow.backBufferDimension;
			}
		}
		switch (type)
		{
		case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
		{
			auto& val2 = val["directional"];
			val2["type"] = true;
			if (directionalLight.numOfCSM != _default.directionalLight.numOfCSM)
			{
				val2["CSM"] = directionalLight.numOfCSM;
			}
			if (directionalLight.lambdaCSM != _default.directionalLight.lambdaCSM)
			{
				val2["lambda"] = directionalLight.lambdaCSM;
			}
		}
		break;
		case longmarch::LightCom::LIGHT_TYPE::POINT:
		{
			auto& val2 = val["point"];
			val2["type"] = true;
			if (pointLight.radius != _default.pointLight.radius)
			{
				val2["radius"] = pointLight.radius;
			}
			if (pointLight.softEdgeRatio != _default.pointLight.softEdgeRatio)
			{
				val2["soft_edge_ratio"] = pointLight.softEdgeRatio;
			}
		}
		break;
		case longmarch::LightCom::LIGHT_TYPE::SPOT:
		{
			auto& val2 = val["spot"];
			val2["type"] = true;
			if (spotLight.innerConeRad != _default.spotLight.innerConeRad)
			{
				val2["inner_cone"] = spotLight.innerConeRad * RAD2DEG;
			}
			if (spotLight.outterConeRad != _default.spotLight.outterConeRad)
			{
				val2["outter_cone"] = spotLight.outterConeRad * RAD2DEG;
			}
			if (spotLight.aspectWByH != _default.spotLight.aspectWByH)
			{
				val2["aspectWByH"] = spotLight.aspectWByH;
			}
			if (spotLight.distance != _default.spotLight.distance)
			{
				val2["distance"] = spotLight.distance;
			}
			if (spotLight.softEdgeRatio != _default.spotLight.softEdgeRatio)
			{
				val2["soft_edge_ratio"] = spotLight.softEdgeRatio;
			}
		}
		break;
		}
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::LightCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	LOCK_GUARD2();
	{
		bool type_init = false;
		{
			auto& val = value["attenuation"];
			if (!val.isNull())
			{
				attenuation = Vec4f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat(), val[3].asFloat());
			}
		}
		{
			auto& val = value["collision_radius"];
			if (!val.isNull())
			{
				collisionRadius = (glm::max)(val.asFloat(), 0.0f);
			}
		}
		{
			auto& val = value["shadow"];
			if (!val.isNull())
			{
				{
					auto& val2 = val["dropOff"];
					if (!val2.isNull())
					{
						shadow.dropOffDistance = (glm::max)(val2.asFloat(), 0.1f);
					}
				}
				{
					auto& val2 = val["nearZ"];
					if (!val2.isNull())
					{
						shadow.nearZ = (glm::max)(val2.asFloat(), 0.1f);
					}
				}
				{
					auto& val2 = val["farZ"];
					if (!val2.isNull())
					{
						shadow.farZ = (glm::max)(val2.asFloat(), 0.1f);
					}
				}
				{
					auto& val2 = val["enable"];
					if (!val2.isNull())
					{
						shadow.bCastShadow = val2.asBool();
					}
				}
				{
					auto& val2 = val["dimension"];
					if (!val2.isNull())
					{
						shadow.origin_dimension = shadow.dimension = val2.asUInt();
					}
				}
				{
					auto& val2 = val["algorithm"];
					if (!val2.isNull())
					{
						auto algo = val2.asCString();
						for (int i = 0; i < IM_ARRAYSIZE(s_shadowAlgorithmModes); ++i)
						{
							if (strcmp(s_shadowAlgorithmModes[i], algo) == 0)
							{
								shadow.shadowAlgorithmMode = i;
							}
						}
					}
				}
				{
					auto& val2 = val["depthBiasHigherBound"];
					if (!val2.isNull())
					{
						shadow.depthBiasHigherBound = val2.asFloat();
					}
				}
				{
					auto& val2 = val["depthBiasMultiplier"];
					if (!val2.isNull())
					{
						shadow.depthBiasMultiplier = val2.asFloat();
					}
				}
				{
					auto& val2 = val["nrmBiasMultiplier"];
					if (!val2.isNull())
					{
						shadow.nrmBiasMultiplier = val2.asFloat();
					}
				}
				{
					auto& val2 = val["enable_guassian_blur"];
					if (!val2.isNull())
					{
						shadow.bEnableGaussianBlur = val2.asBool();
					}
				}
				{
					auto& val2 = val["enable_transparent_shadow"];
					if (!val2.isNull())
					{
						shadow.bEnableTransparentShadow = val2.asBool();
					}
				}
				{
					auto& val2 = val["dimension_guassian_backbuffer"];
					if (!val2.isNull())
					{
						shadow.origin_backBufferDimension = shadow.backBufferDimension = (glm::min)(val2.asUInt(), shadow.dimension);
					}
				}
				{
					auto& val2 = val["guassian_kernel_size"];
					if (!val2.isNull())
					{
						shadow.gaussianKernal = val2.asUInt();
						if (shadow.gaussianKernal % 2u == 0)
						{
							++shadow.gaussianKernal;
						}
						shadow.gaussianKernal = (glm::clamp)(shadow.gaussianKernal, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
					}
				}
			}
		}
		{
			auto& val = value["spot"];
			if (!val.isNull())
			{
				if (type_init)
				{
					EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"LightCom cannot be multiple types!"));
				}
				else
				{
					type_init = true;
					type = LIGHT_TYPE::SPOT;
				}
				{
					auto& val2 = val["inner_cone"];
					if (!val2.isNull())
					{
						spotLight.innerConeRad = glm::clamp(val2.asFloat(), 0.0f, 170.f) * DEG2RAD;
					}
				}
				{
					auto& val2 = val["outter_cone"];
					if (!val2.isNull())
					{
						spotLight.outterConeRad = glm::clamp(val2.asFloat(), 0.0f, 170.f) * DEG2RAD;
					}
				}
				{
					auto& val2 = val["aspectWByH"];
					if (!val2.isNull())
					{
						spotLight.aspectWByH = (glm::max)(val2.asFloat(), 0.0f);
					}
				}
				{
					auto& val2 = val["distance"];
					if (!val2.isNull())
					{
						spotLight.distance = (glm::max)(val2.asFloat(), 0.0f);
					}
				}
				{
					auto& val2 = val["soft_edge_ratio"];
					if (!val2.isNull())
					{
						spotLight.softEdgeRatio = glm::clamp(val2.asFloat(), 0.0f, 1.0f);
					}
				}
			}
		}
		{
			auto& val = value["directional"];
			if (!val.isNull())
			{
				if (type_init)
				{
					EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"LightCom cannot be multiple types!"));
				}
				else
				{
					type_init = true;
					type = LIGHT_TYPE::DIRECTIONAL;
				}
				{
					auto& val2 = val["CSM"];
					if (!val2.isNull())
					{
						directionalLight.numOfCSM = glm::clamp(val2.asUInt(), 1u, 4u);
					}
				}
				{
					auto& val2 = val["lambda"];
					if (!val2.isNull())
					{
						directionalLight.lambdaCSM = glm::clamp(val2.asFloat(), 0.0f, 1.0f);
					}
				}
			}
		}
		{
			auto& val = value["point"];
			if (!val.isNull())
			{
				if (type_init)
				{
					EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"LightCom cannot be multiple types!"));
				}
				else
				{
					type_init = true;
					type = LIGHT_TYPE::POINT;
				}
				{
					auto& val2 = val["radius"];
					if (!val2.isNull())
					{
						pointLight.radius = (glm::max)(val2.asFloat(), 0.0f);
					}
				}
				{
					auto& val2 = val["soft_edge_ratio"];
					if (!val2.isNull())
					{
						pointLight.softEdgeRatio = glm::clamp(val2.asFloat(), 0.0f, 1.0f);
					}
				}
			}
		}
		if (!type_init)
		{
			EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"LightCom type is not set! Choose one of the following: directional, point, spot!"));
		}
	}
}

void longmarch::LightCom::ImGuiRender()
{
	constexpr int yoffset_item = 5;
	constexpr int width_item = 100;
	constexpr int width_item_1 = 200;
	constexpr int width_item_2 = 400;

	if (ImGui::TreeNode("Light"))
	{
		ImGuiUtil::InlineHelpMarker("Light color and intensity is handled in material section of the scene component");
		if (ImGui::TreeNode("General"))
		{
			{
				ImGui::PushItemWidth(width_item);
				float v = collisionRadius;
				float speed = 0.01f;
				if (ImGui::DragFloat("Collision Radius", &v, speed, (glm::max)(v - 1, 0.f), v + 1))
				{
					collisionRadius = v;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item_2);
				auto atten = attenuation;
				if (ImGui::InputFloat4("Attenuation", &atten[0]))
				{
					attenuation = atten;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item_1);
				static const char* lightType[]{ "Directional", "Point", "Spot" };
				auto _type = (int)type;
				if (ImGui::Combo("Type", &_type, lightType, IM_ARRAYSIZE(lightType)))
				{
					type = (LightCom::LIGHT_TYPE)_type;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGui::PopItemWidth();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Directional"))
		{
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Point"))
		{
			{
				ImGui::PushItemWidth(width_item);
				float v = pointLight.radius;
				float speed = 0.1f;
				if (ImGui::DragFloat("Radius", &v, speed, (glm::max)(v - 1, 0.f), v + 1))
				{
					pointLight.radius = (glm::max)(v, 0.f);
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = pointLight.softEdgeRatio;
				float speed = 0.01f;
				if (ImGui::DragFloat("Soft Edge Ratio", &v, speed, 0.0f, 1.0f))
				{
					pointLight.softEdgeRatio = v;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Spot"))
		{
			{
				ImGui::PushItemWidth(width_item);
				float v = spotLight.distance;
				float speed = 0.1f;
				if (ImGui::DragFloat("Distance", &v, speed, (glm::max)(v - 1, 0.f), v + 1))
				{
					spotLight.distance = (glm::max)(v, 0.f);
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = spotLight.innerConeRad * RAD2DEG;
				float speed = 0.25f;
				if (ImGui::DragFloat("Inner Cone", &v, speed, 0.0f, 170.f))
				{
					spotLight.innerConeRad = v * DEG2RAD;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = spotLight.outterConeRad * RAD2DEG;
				float speed = 0.25f;
				if (ImGui::DragFloat("Outter Cone", &v, speed, 0.0f, 170.f))
				{
					spotLight.outterConeRad = v * DEG2RAD;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = spotLight.softEdgeRatio;
				float speed = 0.01f;
				if (ImGui::DragFloat("Soft Edge Ratio", &v, speed, 0.0f, 1.0f))
				{
					spotLight.softEdgeRatio = v;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = spotLight.aspectWByH;
				float speed = 0.01f;
				if (ImGui::DragFloat("Aspect ratio (with/height)", &v, speed, 0.5f, 2.0f))
				{
					spotLight.aspectWByH = v;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Shadow"))
		{
			{
				bool v = shadow.bCastShadow;
				if (ImGui::Checkbox("Cast Shadow", &v))
				{
					shadow.bCastShadow = v;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item_1);
				int v = shadow.shadowAlgorithmMode;
				if (ImGui::Combo("Shadow Algorithm", &v, s_shadowAlgorithmModes, IM_ARRAYSIZE(s_shadowAlgorithmModes)))
				{
					shadow.shadowAlgorithmMode = v;
					if (v <= 3)
					{
						shadow.bEnableGaussianBlur = false;
					}
					else
					{
						shadow.bEnableGaussianBlur = true;
					}
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("MSM2 and MSM4 are only available for point light and spot light");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			switch (type)
			{
			case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
			{
				ImGui::PushItemWidth(width_item_1);
				int v = directionalLight.numOfCSM;
				if (ImGui::SliderInt("# CSM", &v, 1, 4))
				{
					directionalLight.numOfCSM = v; 
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("Cascade Shadow Map (CSM) is a technique that allows several directional shadow maps of the same resolution to cover scene objects at different distances to the camera. \n This allows directional shadow to cover every details of the scene at every distances.");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item_1);
				float v = directionalLight.lambdaCSM;
				if (ImGui::SliderFloat("CSM Lambda", &v, 0.0f, 1.0f))
				{
					directionalLight.lambdaCSM = v;
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("If you see artifacts at far distances, move different levels of CSM back and forth.");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item_1);
				// CSM debug
				static const char* csmDebug[]{ "None", "Levels", "Shadow Intensity" };
				static int selected_csmDebug = 0;
				if (ImGui::Combo("CSM Debug", &selected_csmDebug, csmDebug, IM_ARRAYSIZE(csmDebug)))
				{
					Renderer3D::s_Data.directional_light_display_mode = selected_csmDebug;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			break;
			case longmarch::LightCom::LIGHT_TYPE::POINT:
				break;
			case longmarch::LightCom::LIGHT_TYPE::SPOT:
				break;
			default:
				break;
			}
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.depthBiasHigherBound;
				float speed = 0.1f;
				if (ImGui::DragFloat("Depth Bias High", &v, speed, (glm::max)(v - 1, 1.f), v + 1))
				{
					shadow.depthBiasHigherBound = (glm::max)(v, 1.0f);
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("Set the shadow map depth bias upper bound");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.depthBiasMultiplier;
				float speed = 0.1f;
				if (ImGui::DragFloat("Depth Bias Multi", &v, speed, (glm::max)(v - 1, 0.f), v + 1))
				{
					shadow.depthBiasMultiplier = (glm::max)(v, .0f);
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("Set the shadow map depth slope bias multiplier");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.nrmBiasMultiplier;
				float speed = 0.1f;
				if (ImGui::DragFloat("Normal Bias Multi", &v, speed, v-1, v+1))
				{
					shadow.nrmBiasMultiplier = v;
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("Set the shadow map depth normal bias multiplier");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.nearZ;
				float speed = 0.1f;
				if (ImGui::DragFloat("Shadow Map Near plane", &v, speed, (glm::max)(v * 0.9f, 0.1f), v * 1.1f))
				{
					shadow.nearZ = (glm::max)(v, 0.1f);
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.farZ;
				float speed = 0.1f;
				if (ImGui::DragFloat("Shadow Map Far plane", &v, speed, (glm::max)(v * 0.9f, 0.1f), v * 1.1f))
				{
					shadow.farZ = v;
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				float v = shadow.dropOffDistance;
				float speed = 0.1f;
				if (ImGui::DragFloat("Shadow Map Drop Off Distance", &v, speed, (glm::max)(v * 0.9f, 0.0f), v * 1.1f))
				{
					shadow.dropOffDistance = v;
				}
				ImGui::PopItemWidth();
				ImGuiUtil::InlineHelpMarker("Shadow map would choose to render in lower resolution on being greater distances to the camera. \n Does not apply to shadow of directional light, use CSM instead.");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				int v = shadow.origin_dimension;
				ImGui::InputInt("Shadow Map Size", &v, 128);
				if (shadow.origin_dimension != v)
				{
					shadow.origin_dimension = (glm::max)(v, 128);
					shadow.dimension = shadow.origin_dimension;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				bool v = shadow.bEnableTransparentShadow;
				if (ImGui::Checkbox("Enable Transparent Shadow", &v))
				{
					shadow.bEnableTransparentShadow = v;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				bool v = shadow.bEnableGaussianBlur;
				if (ImGui::Checkbox("Enable Gaussian Blur", &v))
				{
					shadow.bEnableGaussianBlur = v;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGuiUtil::InlineHelpMarker("For MSM2 and MSM4, this value should be true, otherwise, set to false");
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				int v = shadow.gaussianKernal;
				if (ImGui::SliderInt("Gaussian Kernel Size", &v, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX))
				{
					shadow.gaussianKernal = v;
					shadow.gaussianKernal = (glm::clamp)(shadow.gaussianKernal, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
			{
				ImGui::PushItemWidth(width_item);
				int v = shadow.origin_backBufferDimension;
				ImGui::InputInt("Gaussian Backbuffer Shadow Map Size", &v, 128);
				if (shadow.origin_backBufferDimension != v)
				{
					shadow.origin_backBufferDimension = (glm::clamp)(v, 128, static_cast<int>(shadow.dimension));
					shadow.backBufferDimension = shadow.origin_backBufferDimension;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
				ImGui::PopItemWidth();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}

void longmarch::LightCom::AllocateShadowBuffer()
{
	if (!shadow.bInit)
	{
		shadow.bInit = true;
		switch (type)
		{
		case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
#ifdef USE_DIRECTIONAL_LIGHT_COMPARE_TEXTURE
			shadow.shadowBuffer = ShadowBuffer::CreateArray(
				shadow.dimension, shadow.dimension,
				directionalLight.numOfCSM,
				ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE);
#else
			shadow.shadowBuffer = ShadowBuffer::CreateArray(
				shadow.dimension, shadow.dimension,
				directionalLight.numOfCSM,
				ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#endif
			break;
		case longmarch::LightCom::LIGHT_TYPE::POINT:
#ifdef USE_POINT_LIGHT_ARRAY_TEXTURE
			shadow.shadowBuffer = ShadowBuffer::CreateArray(
				shadow.dimension, shadow.dimension,
				6,
				ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#else
			shadow.shadowBuffer = ShadowBuffer::Create(
				shadow.dimension, shadow.dimension,
				ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE);
#endif
			break;
		case longmarch::LightCom::LIGHT_TYPE::SPOT:
			shadow.shadowBuffer = ShadowBuffer::Create(
				shadow.dimension, shadow.dimension,
				ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4);
			break;
		}
		if (shadow.bEnableGaussianBlur)
		{
			switch (type)
			{
			case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
#ifdef USE_DIRECTIONAL_LIGHT_COMPARE_TEXTURE
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.backBufferDimension, shadow.backBufferDimension,
					directionalLight.numOfCSM,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE);
#else
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.backBufferDimension, shadow.backBufferDimension,
					directionalLight.numOfCSM,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#endif
				break;
			case longmarch::LightCom::LIGHT_TYPE::POINT:
#ifdef USE_POINT_LIGHT_ARRAY_TEXTURE
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.backBufferDimension, shadow.backBufferDimension,
					6,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#else
				shadow.shadowBuffer2 = ShadowBuffer::Create(
					shadow.dimension, shadow.dimension,
					ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE);
#endif
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.backBufferDimension, shadow.backBufferDimension,
					6,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
				break;
			case longmarch::LightCom::LIGHT_TYPE::SPOT:
				shadow.shadowBuffer2 = ShadowBuffer::Create(
					shadow.backBufferDimension, shadow.backBufferDimension,
					ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4);
				break;
			}
		}
		if (shadow.bEnableTransparentShadow)
		{
			switch (type)
			{
			case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
#ifdef USE_DIRECTIONAL_LIGHT_COMPARE_TEXTURE
				shadow.shadowBuffer3 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
					directionalLight.numOfCSM,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE);
#else
				shadow.shadowBuffer3 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
					directionalLight.numOfCSM,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#endif
				break;
			case longmarch::LightCom::LIGHT_TYPE::POINT:
#ifdef USE_POINT_LIGHT_ARRAY_TEXTURE
				shadow.shadowBuffer3 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
					6,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#else
				shadow.shadowBuffer2 = ShadowBuffer::Create(
					shadow.dimension, shadow.dimension,
					ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE);
#endif
				shadow.shadowBuffer3 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
					6,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
				break;
			case longmarch::LightCom::LIGHT_TYPE::SPOT:
				shadow.shadowBuffer3 = ShadowBuffer::Create(
					shadow.dimension, shadow.dimension,
					ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4);
				break;
			}
		}
	}
}

void longmarch::LightCom::ReleaseShadowBuffer()
{
	shadow.bInit = false;
	shadow.shadowBuffer.reset();
	shadow.shadowBuffer2.reset();
	shadow.shadowBuffer3.reset();
}

bool longmarch::LightCom::HandleShadowBufferDropOff(float distance)
{
	constexpr uint32_t minimum_shadow_map_size = 128u;
	constexpr uint32_t drop_off_max_ratio = 4u;
	float drop_off_threshold_distance = shadow.dropOffDistance / static_cast<float>(drop_off_max_ratio);
	if (distance > drop_off_threshold_distance)
	{
		// Apply drop off when distance to camera is greater than threshold
		uint32_t ratio = glm::max(static_cast<uint32_t>(glm::roundEven(distance / drop_off_threshold_distance)), 2u);
		if (ratio > drop_off_max_ratio)
		{
			// Early quit on exceeding maximum drop off ratio
			return true;
		}
		uint32_t target1 = (glm::max)(shadow.origin_dimension / ratio, minimum_shadow_map_size);
		uint32_t target2 = (glm::max)(shadow.origin_backBufferDimension / ratio, minimum_shadow_map_size);
		if (shadow.dimension != target1 || shadow.backBufferDimension != target2)
		{
			shadow.dimension = target1;
			shadow.backBufferDimension = target2;
			ReleaseShadowBuffer();
			AllocateShadowBuffer();
		}
	}
	else if (shadow.dimension != shadow.origin_dimension || shadow.backBufferDimension != shadow.origin_backBufferDimension)
	{
		shadow.dimension = (glm::max)(shadow.origin_dimension, minimum_shadow_map_size);
		shadow.backBufferDimension = (glm::max)(shadow.origin_backBufferDimension, minimum_shadow_map_size);
		ReleaseShadowBuffer();
		AllocateShadowBuffer();
	}
	return false;
}