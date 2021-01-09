#pragma once

#include "engine/EngineEssential.h"
#include "engine/ecs/EntityType.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/BaseComponentSystem.h"

namespace longmarch
{
	class GameWorld;

	class ObjectFactory
	{
	public:
		//! Your application should inherit ObjectFactory and initialize s_instance to this pointer
		ObjectFactory();
		virtual ~ObjectFactory() = default;

		const LongMarch_Vector<std::string> GetAllEntityTypeName() const;
		const LongMarch_Vector<std::string> GetAllComponentName() const;
		const LongMarch_Vector<std::string> GetAllComponentSystemName() const;

		const EntityType GetEntityTypeFromName(const std::string& s_type) const;
		const std::string GetEntityNameFromType(EntityType e_type) const;

		virtual BaseComponentInterface* AddComponentByName(const std::string& com_type, EntityDecorator entity) const;
		virtual std::shared_ptr<BaseComponentSystem> AddComponentSystemByName(const std::string& comsys_type, GameWorld* world) const;

		virtual bool RemoveComponentByName(const std::string& com_type, EntityDecorator entity) const;
		virtual bool RemoveComponentSystemByName(const std::string& comsys_type, GameWorld* world) const;

		virtual void LoadLevel(const fs::path& filepath, GameWorld* world) const;
		virtual void SaveLevel(const fs::path& filepath, GameWorld* world) const;
		virtual void LoadResources(const fs::path& filepath, GameWorld* world) const;
		virtual void LoadSystems(const fs::path& filepath, GameWorld* world) const;

		virtual void LoadGameWorldScene(const fs::path& filepath, GameWorld* world) const;
		virtual void SaveGameWorldScene(const fs::path& filepath, GameWorld* world) const;

	protected:
		LongMarch_Vector<EntityNameToType> m_EntityNameToTypeList;
		LongMarch_Vector<EntityTypeToName> m_EntityTypeToNameList;

		LongMarch_Vector<std::string> m_ComponentNameList;
		LongMarch_Vector<std::string> m_ComponentSystemNameList;

	public:
		inline static ObjectFactory* s_instance = { nullptr };
	};
}
