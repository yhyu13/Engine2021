#include "engine-precompiled-header.h"
#include "TypeNameCom.h"

longmarch::TypeNameCom::TypeNameCom(const std::string& _name)
{
	SetTypeName(_name);
}

void longmarch::TypeNameCom::SetTypeName(const std::string& _name)
{
	LOCK_GUARD();
	if (std::strlen(m_type) > 0)
	{
		ENGINE_EXCEPT(L"Type name should be initialized to emtpy!");
	}
	if (std::strlen(_name.c_str()) <= ECS_TYPE_NAME_MAX_LEN)
	{
		strcpy(m_type, _name.c_str());
	}
	else
	{
		ENGINE_EXCEPT(wStr(_name) + L" type name is greater than max length : " + wStr(ECS_TYPE_NAME_MAX_LEN));
	}
}

const char* longmarch::TypeNameCom::GetTypeName() const
{
	LOCK_GUARD();
    return m_type;
}

bool longmarch::TypeNameCom::IsSameType(const char* _name) const
{
	LOCK_GUARD();
	return strcmp(m_type, _name) == 0;
}

bool longmarch::TypeNameCom::IsBaseTypeOf(const char* _name) const
{
	LOCK_GUARD();
	if (std::strlen(m_type) >= std::strlen(_name))
	{
		return false;
	}
	else
	{
		for (int i = 0; i < std::strlen(m_type); ++i)
		{
			if (_name[i] != m_type[i])
			{
				return false;
			}
		}
		return true;
	}
}

bool longmarch::TypeNameCom::IsDerivedTypeOf(const char* _name) const
{
	LOCK_GUARD();
	if (std::strlen(m_type) <= std::strlen(_name))
	{
		return false;
	}
	else
	{
		for (int i = 0; i < std::strlen(_name); ++i)
		{
			if (_name[i] != m_type[i])
			{
				return false;
			}
		}
		return true;
	}
}

void longmarch::TypeNameCom::JsonSerialize(Json::Value& value) const
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD();
	{
		Json::Value output;
		output["id"] = "TypeNameCom";
		auto& val = output["value"];
		static const auto& _default = TypeNameCom();

		if (strcmp(m_type, _default.m_type) != 0)
		{
			val["name"] = std::string(m_type);
		}
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::TypeNameCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	{
		auto& val = value["name"];
		if (!val.isNull())
		{
			SetTypeName(val.asString());
		}
	}
}

void longmarch::TypeNameCom::ImGuiRender()
{
	if (ImGui::TreeNode("Type Name"))
	{
		{
			ImGui::Text(("Type Name : " + std::string(GetTypeName())).c_str());
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}
