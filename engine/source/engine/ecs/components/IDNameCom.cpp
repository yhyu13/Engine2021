#include "engine-precompiled-header.h"
#include "IDNameCom.h"

longmarch::IDNameCom::IDNameCom(const Entity& _this, const std::string& _name)
	:
	m_this(_this),
	m_name(_name)
{
	if (m_name == "")
	{
		m_name = Str(m_this);
	}
}

void longmarch::IDNameCom::SetName(const std::string& name)
{
	LOCK_GUARD();
	m_name = name;
}

std::string longmarch::IDNameCom::GetName()
{
	LOCK_GUARD();
	return m_name;
}

std::string longmarch::IDNameCom::GetUniqueName()
{
	LOCK_GUARD();
	return Str(m_this) + "_" + m_name;
}

Entity longmarch::IDNameCom::GetEntity()
{
	return m_this;
}

void longmarch::IDNameCom::JsonSerialize(Json::Value& value)
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD();
	{
		Json::Value output;
		output["id"] = "IDNameCom";
		auto& val = output["value"];
		static const auto& _default = IDNameCom();

		if (m_name != _default.m_name)
		{
			val["name"] = m_name;
		}
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::IDNameCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	{
		auto& val = value["name"];
		if (!val.isNull())
		{
			SetName(val.asString());
		}
	}
}

void longmarch::IDNameCom::ImGuiRender()
{
	if (ImGui::TreeNode("ID Name"))
	{
		{
			std::string name = GetName();
			std::vector<char> arr;
			arr.resize(name.length() + 64);
			strcpy(&arr[0], name.c_str());
			ImGui::InputText("Name", &arr[0], arr.size(), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll);
			std::string _name(arr.begin(), arr.end());
			if (name != _name)
			{
				SetName(_name);
			}
		}
		{
			ImGui::Text(("Unique ID : " + GetUniqueName()).c_str());
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}
