#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"

#define ECS_TYPE_NAME_MAX_LEN 32

namespace longmarch
{
	/*
	Data class that stores type name of a entity, used to determine entity type and potentially can used to represent base-derived relationship
	*/
	struct CACHE_ALIGN16 TypeNameCom final: BaseComponent<TypeNameCom>
	{
		TypeNameCom() = default;
		explicit TypeNameCom(const std::string& _name);
		void SetTypeName(const std::string& _name);
		const char* GetTypeName() const;

		bool IsSameType(const char* _name) const;
		bool IsBaseTypeOf(const char* _name) const;
		bool IsDerivedTypeOf(const char* _name) const;

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:
		char m_type[ECS_TYPE_NAME_MAX_LEN]{ "" };
	};
}
