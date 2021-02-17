#include "engine-precompiled-header.h"
#include "LightCom.h"
#include "engine/renderer/Renderer3D.h"

#define POINT_LIGHT_ARRAY_SHADOW_MAP

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
				val2["algorithm"] = shadow.shadowAlgorithmMode;
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
						shadow.shadowAlgorithmMode = (glm::clamp)(val2.asUInt(), 0u, 4u);
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
						shadow.gaussianKernal = (glm::clamp)(shadow.gaussianKernal, 3u, 51u);
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
	LOCK_GUARD2();
	if (ImGui::TreeNode("Light"))
	{
		if (ImGui::TreeNode("General"))
		{
			{
				float v = collisionRadius;
				if (ImGui::InputFloat("Collision Radius", &v))
				{
					collisionRadius = v;
				}
			}
			{
				auto atten = attenuation;
				if (ImGui::InputFloat4("Attenuation", &atten[0]))
				{
					attenuation = atten;
				}
			}
			{
				static const char* lightType[]{ "Directional", "Point", "Spot" };
				auto _type = (int)type;
				if (ImGui::Combo("Type", &_type, lightType, IM_ARRAYSIZE(lightType)))
				{
					type = (LightCom::LIGHT_TYPE)_type;
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Directional"))
		{
			{
				int v = directionalLight.numOfCSM;
				if (ImGui::SliderInt("# CSM", &v, 1, 4))
				{
					directionalLight.numOfCSM = v;
				}
			}
			{
				float v = directionalLight.lambdaCSM;
				if (ImGui::SliderFloat("CSM Lambda", &v, 0.0f, 1.0f))
				{
					directionalLight.lambdaCSM = v;
				}
			}
			{
				// CSM debug
				static const char* csmDebug[]{ "None", "Levels", "Camera depth","Light depth" };
				static int selected_csmDebug = 0;
				if (ImGui::Combo("CSM Debug", &selected_csmDebug, csmDebug, IM_ARRAYSIZE(csmDebug)))
				{
					Renderer3D::s_Data.directional_light_display_mode = selected_csmDebug;
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Point"))
		{
			{
				float v = pointLight.radius;
				if (ImGui::InputFloat("Radius", &v))
				{
					pointLight.radius = (glm::max)(v, 0.f);
				}
			}
			{
				float v = pointLight.softEdgeRatio;
				float speed = 0.01f;
				if (ImGui::DragFloat("Soft Edge Ratio", &v, speed, 0.0f, 1.0f))
				{
					pointLight.softEdgeRatio = v;
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Spot"))
		{
			{
				float v = spotLight.distance;
				if (ImGui::InputFloat("Distance", &v))
				{
					spotLight.distance = (glm::max)(v, 0.f);
				}
			}
			{
				float v = spotLight.innerConeRad * RAD2DEG;
				float speed = 0.25f;
				if (ImGui::DragFloat("Inner Cone", &v, speed, 0.0f, 170.f))
				{
					spotLight.innerConeRad = v * DEG2RAD;
				}
			}
			{
				float v = spotLight.outterConeRad * RAD2DEG;
				float speed = 0.25f;
				if (ImGui::DragFloat("Outter Cone", &v, speed, 0.0f, 170.f))
				{
					spotLight.outterConeRad = v * DEG2RAD;
				}
			}
			{
				float v = spotLight.softEdgeRatio;
				float speed = 0.01f;
				if (ImGui::DragFloat("Soft Edge Ratio", &v, speed, 0.0f, 1.0f))
				{
					spotLight.softEdgeRatio = v;
				}
			}
			{
				float v = spotLight.aspectWByH;
				float speed = 0.01f;
				if (ImGui::DragFloat("Aspect WByH", &v, speed, 0.5f, 2.0f))
				{
					spotLight.aspectWByH = v;
				}
			}
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
			{
				static const char* shadowAlgorithmModes[]{ "Direct", "PCF", "Poisson", "2MSM", "4MSM" };
				int v = shadow.shadowAlgorithmMode;
				if (ImGui::Combo("Shadow Algorithm", &v, shadowAlgorithmModes, IM_ARRAYSIZE(shadowAlgorithmModes)))
				{
					shadow.shadowAlgorithmMode = v;
					if (v < 3)
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
			}
			{
				float v = shadow.depthBiasHigherBound;
				if (ImGui::InputFloat("Depth Bias High", &v))
				{
					shadow.depthBiasHigherBound = (glm::max)(v, 1.0f);
				}
			}
			{
				float v = shadow.depthBiasMultiplier;
				if (ImGui::InputFloat("Depth Bias Multi", &v))
				{
					shadow.depthBiasMultiplier = (glm::max)(v, .0f);
				}
			}
			{
				float v = shadow.nrmBiasMultiplier;
				if (ImGui::InputFloat("Nrm Bias Multi", &v))
				{
					shadow.nrmBiasMultiplier = v;
				}
			}
			{
				float v = shadow.nearZ;
				if (ImGui::InputFloat("Near Z", &v))
				{
					shadow.nearZ = (glm::max)(v, 0.1f);
				}
			}
			{
				float v = shadow.farZ;
				if (ImGui::InputFloat("Far Z", &v))
				{
					shadow.farZ = v;
				}
			}
			{
				float v = shadow.dropOffDistance;
				if (ImGui::InputFloat("Shadow Map Drop Off Distance", &v))
				{
					shadow.dropOffDistance = v;
				}
			}
			{
				int v = shadow.dimension;
				ImGui::InputInt("Shadow Map Size", &v, 128);
				if (ImGui::Button("Restore Shadow Map Size"))
				{
					v = shadow.origin_dimension;
				}
				if (shadow.dimension != v)
				{
					shadow.dimension = (glm::max)(v, 128);
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
			}
			{
				bool v = shadow.bEnableGaussianBlur;
				if (ImGui::Checkbox("Enable Gaussian Blur", &v))
				{
					shadow.bEnableGaussianBlur = v;
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
			}
			{
				int v = shadow.gaussianKernal;
				if (ImGui::SliderInt("Gaussian Kernel Size", &v, 3, 51))
				{
					shadow.gaussianKernal = v;
					if (shadow.gaussianKernal % 2u == 0)
					{
						++shadow.gaussianKernal;
					}
					shadow.gaussianKernal = (glm::clamp)(shadow.gaussianKernal, 3u, 51u);
				}
			}
			{
				int v = shadow.backBufferDimension;
				ImGui::InputInt("Gaussian Backbuffer Shadow Map Size", &v, 128);
				if (ImGui::Button("Restore Backbuffer Shadow Map Size"))
				{
					v = shadow.origin_backBufferDimension;
				}
				if (shadow.backBufferDimension != v)
				{
					shadow.backBufferDimension = (glm::clamp)(v, 128, static_cast<int>(shadow.dimension));
					ReleaseShadowBuffer();
					AllocateShadowBuffer();
				}
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}

void longmarch::LightCom::AllocateShadowBuffer()
{
	if (!shadow.bInit && shadow.bCastShadow)
	{
		shadow.bInit = true;
		switch (type)
		{
		case longmarch::LightCom::LIGHT_TYPE::DIRECTIONAL:
			shadow.shadowBuffer = ShadowBuffer::CreateArray(
				shadow.dimension, shadow.dimension,
				directionalLight.numOfCSM,
				ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
			break;
		case longmarch::LightCom::LIGHT_TYPE::POINT:
#ifdef POINT_LIGHT_ARRAY_SHADOW_MAP
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
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.backBufferDimension, shadow.backBufferDimension,
					directionalLight.numOfCSM,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
				break;
			case longmarch::LightCom::LIGHT_TYPE::POINT:
#ifdef POINT_LIGHT_ARRAY_SHADOW_MAP
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
					6,
					ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
#else
				shadow.shadowBuffer2 = ShadowBuffer::Create(
					shadow.dimension, shadow.dimension,
					ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE);
#endif
				shadow.shadowBuffer2 = ShadowBuffer::CreateArray(
					shadow.dimension, shadow.dimension,
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
	}
}

void longmarch::LightCom::ReleaseShadowBuffer()
{
	shadow.shadowBuffer.reset();
	shadow.shadowBuffer2.reset();
	shadow.bInit = false;
}

bool longmarch::LightCom::HandleShadowBufferDropOff(float distance)
{
	constexpr uint32_t drop_off_ratio = 4;
	float drop_off_ratio_distance = shadow.dropOffDistance / static_cast<float>(drop_off_ratio);
	if (distance > drop_off_ratio_distance)
	{
		uint32_t ratio = glm::max(static_cast<uint32_t>(glm::roundEven(distance / drop_off_ratio_distance)), 2u);
		if (ratio > drop_off_ratio)
		{
			return true;
		}
		uint32_t target1 = (glm::max)(shadow.origin_dimension / ratio, 128u);
		uint32_t target2 = (glm::max)(shadow.origin_backBufferDimension / ratio, 128u);
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
		shadow.dimension = (glm::max)(shadow.origin_dimension, 128u);
		shadow.backBufferDimension = (glm::max)(shadow.origin_backBufferDimension, 128u);
		ReleaseShadowBuffer();
		AllocateShadowBuffer();
	}
	return false;
}