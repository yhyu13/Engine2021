/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/15/2020
- End Header ----------------------------*/

#pragma once

#include "engine/ecs/object-factory/ObjectFactory.h"
#include "engine/ecs/EntityDecorator.h"
#include <json/json.h>

namespace longmarch
{
	class MainObjectFactory final : public ObjectFactory
	{
	public:
		MainObjectFactory();

		virtual BaseComponentInterface* AddComponentByName(const std::string& com_type, EntityDecorator e) const override;
		virtual std::shared_ptr<BaseComponentSystem> AddComponentSystemByName(const std::string& comsys_type, GameWorld* world) const override;

		virtual bool RemoveComponentByName(const std::string& com_type, EntityDecorator entity) const override;
		virtual bool RemoveComponentSystemByName(const std::string& comsys_type, GameWorld* world) const override;

		virtual void LoadLevel(const fs::path& filepath, GameWorld* world) const override;
		virtual void SaveLevel(const fs::path& filepath, GameWorld* world) const override;

		virtual void LoadResources(const fs::path& filepath, GameWorld* world) const override;
		virtual void LoadSystems(const fs::path& filepath, GameWorld* world) const override;
		virtual void LoadGameWorldScene(const fs::path& filepath, GameWorld* world) const override;
		virtual void SaveGameWorldScene(const fs::path& filepath, GameWorld* world) const override;
	private:
		void DeserializeEntity(EntityDecorator parentEntity, const Json::Value& object, GameWorld* world, int level) const;
		void DeserializedCom(EntityDecorator entity, const Json::Value& id, const Json::Value& value) const;

		void SerializeEntity(EntityDecorator parentEntity, Json::Value& object, GameWorld* world, int level) const;
		Json::Value SerializedCom(EntityDecorator entity) const;
	};
}
