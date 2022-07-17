#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"

namespace longmarch
{
	/*
	Data class that stores ID and name of a entity (which is used as a mean to indentification from UI/editor)
	*/
	struct MS_ALIGN16 IDNameCom final: BaseComponent<IDNameCom> 
	{
		IDNameCom() = default;
		explicit IDNameCom(const Entity& _this, const std::string& _name = "");
		void SetName(const std::string& name);
		std::string GetName();
		std::string GetUniqueName();
		Entity GetEntity();

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;
		
	private:
		Entity m_this;
		std::string m_name{ "" };
	};
}
