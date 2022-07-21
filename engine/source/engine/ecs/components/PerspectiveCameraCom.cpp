#include "engine-precompiled-header.h"
#include "PerspectiveCameraCom.h"

void longmarch::PerspectiveCameraCom::SetCamera(const PerspectiveCamera& cam)
{
	LOCK_GUARD();
	m_camera = cam;
}

PerspectiveCamera* longmarch::PerspectiveCameraCom::GetCamera()
{
	LOCK_GUARD();
	return &m_camera;
}

void longmarch::PerspectiveCameraCom::JsonSerialize(Json::Value& value) const
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD();
	{
		Json::Value output;
		output["id"] = "PerspectiveCameraCom";
		auto& val = output["value"];

		val["LookAt"] = (m_camera.type == PerspectiveCameraType::LOOK_AT);
		val["fovy"] = m_camera.cameraSettings.fovy_rad * RAD2DEG;
		val["aspectWByH"] = m_camera.cameraSettings.aspectRatioWbyH;
		val["near"] = m_camera.cameraSettings.nearZ;
		val["far"] = m_camera.cameraSettings.farZ;
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::PerspectiveCameraCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	LOCK_GUARD();
	{
		{
			auto& val = value["LookAt"];
			if (!val.isNull())
			{
				m_camera.type = (val.asBool()) ? PerspectiveCameraType::LOOK_AT : PerspectiveCameraType::FIRST_PERSON;
			}
		}
		{
			auto& val = value["fovy"];
			if (!val.isNull())
			{
				m_camera.cameraSettings.fovy_rad = val.asFloat() * DEG2RAD;
			}
		}
		{
			auto& val = value["aspectWByH"];
			if (!val.isNull())
			{
				m_camera.cameraSettings.aspectRatioWbyH = val.asFloat();
			}
		}
		{
			auto& val = value["near"];
			if (!val.isNull())
			{
				m_camera.cameraSettings.nearZ = val.asFloat();
			}
		}
		{
			auto& val = value["far"];
			if (!val.isNull())
			{
				m_camera.cameraSettings.farZ = val.asFloat();
			}
		}
	}
}

void longmarch::PerspectiveCameraCom::ImGuiRender()
{
	if (ImGui::TreeNode("PerspectiveCamera"))
	{
		constexpr int yoffset_item = 5;
		constexpr int width_item = 100;

		{
			static const char* perspectiveCamModes[]{ "First Person", "Look At"};
			static int selected_perspectiveCamModes = 0;
			if (ImGui::Combo("Camera Mode", &selected_perspectiveCamModes, perspectiveCamModes, IM_ARRAYSIZE(perspectiveCamModes)))
			{
				switch (selected_perspectiveCamModes)
				{
				case 0:
					m_camera.type = PerspectiveCameraType::FIRST_PERSON;
					break;
				case 1:
					m_camera.type = PerspectiveCameraType::LOOK_AT;
					break;
				default:
					break;
				}
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			float speed = .25f;
			float range = 1.f;
			auto fovy = m_camera.cameraSettings.fovy_rad * RAD2DEG;
			{
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("FOV Y", &fovy, speed, 0, 180))
				{
					m_camera.cameraSettings.fovy_rad = fovy * DEG2RAD;
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			float speed = .05f;
			float range = 1.f;
			auto asp = m_camera.cameraSettings.aspectRatioWbyH;
			{
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Aspect Ratio", &asp, speed, 0, asp + range))
				{
					m_camera.cameraSettings.aspectRatioWbyH = asp;
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			float speed = .05f;
			float range = 1.f;
			auto nearZ = m_camera.cameraSettings.nearZ;
			{
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Near Z", &nearZ, speed, 0.01, m_camera.cameraSettings.farZ))
				{
					m_camera.cameraSettings.nearZ = nearZ;
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			float speed = .25f;
			float range = 1.f;
			auto farZ = m_camera.cameraSettings.farZ;
			{
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Far Z", &farZ, speed, m_camera.cameraSettings.nearZ, farZ + range))
				{
					m_camera.cameraSettings.farZ = farZ;
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}
