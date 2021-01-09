#pragma once

#include "engine/EngineEssential.h"
#include "engine/ecs/EntityType.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/BaseComponentSystem.h"

namespace AAAAgames
{
	class GameWorld;

	class ObjectFactory
	{
	public:
		//! Your application should inherit ObjectFactory and initialize s_instance to this pointer
		ObjectFactory();
		virtual ~ObjectFactory() = default;

		const A4GAMES_Vector<std::string> GetAllEntityTypeName() const;
		const A4GAMES_Vector<std::string> GetAllComponentName() const;
		const A4GAMES_Vector<std::string> GetAllComponentSystemName() const;

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
		A4GAMES_Vector<EntityNameToType> m_EntityNameToTypeList;
		A4GAMES_Vector<EntityTypeToName> m_EntityTypeToNameList;

		A4GAMES_Vector<std::string> m_ComponentNameList;
		A4GAMES_Vector<std::string> m_ComponentSystemNameList;

	public:
		inline static ObjectFactory* s_instance = { nullptr };
	};
}
