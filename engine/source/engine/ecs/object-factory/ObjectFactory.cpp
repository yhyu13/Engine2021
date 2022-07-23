#include "engine-precompiled-header.h"
#include "ObjectFactory.h"
#include "engine/ecs/header/header.h"

longmarch::ObjectFactory::ObjectFactory()
{
	s_instance = nullptr;
	m_EntityNameToTypeList.emplace_back(EngineEntity::TypeNameMap);
	m_EntityTypeToNameList.emplace_back(EngineEntity::TypeNameMapInv);

	const LongMarch_Vector<std::string> components{
		"Transform3DCom",
		"PerspectiveCameraCom",
		"Scene3DCom",
		"Body3DCom",
		"LightCom",
		"IDNameCom",
		"Animation3DCom",
	};
	std::move(components.begin(), components.end(), std::back_inserter(m_ComponentNameList));

	const LongMarch_Vector<std::string> componentSys{
	"Scene3DComSys",
	"EntityGCComSys",
	"Transform3DComSys",
	"PerspectiveCameraComSys",
	"Body3DComSys",
	"Animation3DComSys",
	};
	std::move(componentSys.begin(), componentSys.end(), std::back_inserter(m_ComponentSystemNameList));
}

const LongMarch_Vector<std::string> longmarch::ObjectFactory::GetAllEntityTypeName() const
{
	LongMarch_Vector<std::string> ret;
	for (const auto& name2type : m_EntityNameToTypeList)
	{
		LongMarch_MapKeyToVec(name2type, ret);
	}
	return ret;
}

const LongMarch_Vector<std::string> longmarch::ObjectFactory::GetAllComponentName() const
{
	return m_ComponentNameList;
}

const LongMarch_Vector<std::string> longmarch::ObjectFactory::GetAllComponentSystemName() const
{
	return m_ComponentSystemNameList;
}

const EntityType longmarch::ObjectFactory::GetEntityTypeFromName(const std::string& s_type) const
{
	for (auto& list : m_EntityNameToTypeList)
	{
		if (LongMarch_contains(list, s_type))
		{
			return list.at(s_type);
		}
	}
	ENGINE_EXCEPT(L"Entity type " + wStr(s_type) + L" is not found!");
	EntityType temp;
	return temp;
}

const std::string longmarch::ObjectFactory::GetEntityNameFromType(EntityType e_type) const
{
	for (auto& list : m_EntityTypeToNameList)
	{
		if (LongMarch_contains(list, e_type))
		{
			return list.at(e_type);
		}
	}
	ENGINE_EXCEPT(L"Entity type " + wStr(e_type) + L" is not found!");
	std::string temp;
	return temp;
}

BaseComponentInterface* longmarch::ObjectFactory::AddComponentByName(const std::string& com_type, EntityDecorator entity) const
{
	/*
	 1, Modify this map for new component TODO @yuhang : Refactor
	*/
	static const std::unordered_map<std::string, int> m{
	{"Transform3DCom", 1},
	{"PerspectiveCameraCom", 2},
	{"Scene3DCom", 3},
	{"Body3DCom", 4},
	{"LightCom", 5},
	{"IDNameCom", 6},
	{"Animation3DCom", 7},
	};

	BaseComponentInterface* ret = nullptr;
	if (const auto& it = m.find(com_type); it != m.end())
	{
		/*
			2, Modify this switch statement for new component
		*/
		switch (it->second)
		{
		case 1:
		{
			if (!entity.HasComponent<Transform3DCom>())
			{
				entity.Volatile().AddComponent(Transform3DCom(entity));
			}
			auto com = entity.GetComponent<Transform3DCom>();
			ret = com.GetPtr();
		}
		break;
		case 2:
		{
			if (!entity.HasComponent<PerspectiveCameraCom>())
			{
				entity.Volatile().AddComponent(PerspectiveCameraCom());
			}
			auto com = entity.GetComponent<PerspectiveCameraCom>();
			ret = com.GetPtr();
		}
		break;
		case 3:
		{
			if (!entity.HasComponent<Scene3DCom>())
			{
				entity.Volatile().AddComponent(Scene3DCom(entity));
			}
			auto com = entity.GetComponent<Scene3DCom>();
			ret = com.GetPtr();
		}
		break;
		case 4:
		{
			if (!entity.HasComponent<Body3DCom>())
			{
				entity.Volatile().AddComponent(Body3DCom());
			}
			auto com = entity.GetComponent<Body3DCom>();
			ret = com.GetPtr();
		}
		break;
		case 5:
		{
			if (!entity.HasComponent<LightCom>())
			{
				entity.Volatile().AddComponent(LightCom());
			}
			auto com = entity.GetComponent<LightCom>();
			ret = com.GetPtr();
		}
		break;
		case 6:
		{
			if (!entity.HasComponent<IDNameCom>())
			{
				entity.Volatile().AddComponent(IDNameCom(entity, Str(entity)));
			}
			auto com = entity.GetComponent<IDNameCom>();
			ret = com.GetPtr();
		}
		break;
		case 7:
		{
			if (!entity.HasComponent<Animation3DCom>())
			{
				entity.Volatile().AddComponent(Animation3DCom(entity));
			}
			auto com = entity.GetComponent<Animation3DCom>();
			ret = com.GetPtr();
		}
		break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unregistered component!");
			break;
		}
	}
	return ret;
}

std::shared_ptr<BaseComponentSystem> longmarch::ObjectFactory::AddComponentSystemByName(const std::string& comsys_type, GameWorld* world) const
{
	/*
		1, Modify this map for new compoent systems  TODO @yuhang : Refactor
	*/
	static const std::unordered_map<std::string, int> m{
	{"Scene3DComSys", 1},
	{"EntityGCComSys", 2},
	{"Transform3DComSys", 3},
	{"PerspectiveCameraComSys", 4},
	{"Body3DComSys", 5},
	{"Animation3DComSys", 6},
	};

	std::shared_ptr<BaseComponentSystem> system = nullptr;
	if (const auto& it = m.find(comsys_type); it != m.end())
	{
		/*
			2, Modify this switch statement for new compoent systems
		*/
		switch (it->second)
		{
		case 1:
			system = MemoryManager::Make_shared<Scene3DComSys>();
			break;
		case 2:
			system = MemoryManager::Make_shared<EntityGCComSys>();
			break;
		case 3:
			system = MemoryManager::Make_shared<Transform3DComSys>();
			break;
		case 4:
			system = MemoryManager::Make_shared<PerspectiveCameraComSys>();
			break;
		case 5:
			system = MemoryManager::Make_shared<Body3DComSys>();
			break;
		case 6:
			system = MemoryManager::Make_shared<Animation3DComSys>();
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"System is not registered");
			break;
		}
		world->RegisterSystem(system, it->first);
	}
	return system;
}

bool longmarch::ObjectFactory::RemoveComponentByName(const std::string& com_type, EntityDecorator entity) const
{
	/*
	 1, Modify this map for new compoent  TODO @yuhang : Refactor
	*/
	static const std::unordered_map<std::string, int> m{
	{"Transform3DCom", 1},
	{"PerspectiveCameraCom", 2},
	{"Scene3DCom", 3},
	{"Body3DCom", 4},
	{"LightCom", 5},
	{"IDNameCom", 6},
	{"Animation3DCom", 7},
	};

	if (const auto& it = m.find(com_type); it != m.end())
	{
		/*
			2, Modify this switch statement for new compoent systems
		*/
		switch (it->second)
		{
		case 1:
		{
			if (entity.HasComponent<Transform3DCom>())
			{
				entity.Volatile().RemoveComponent<Transform3DCom>();
			}
			return true;
		}
		break;
		case 2:
		{
			if (entity.HasComponent<PerspectiveCameraCom>())
			{
				entity.Volatile().RemoveComponent<PerspectiveCameraCom>();
			}
			return true;
		}
		break;
		case 3:
		{
			if (entity.HasComponent<Scene3DCom>())
			{
				entity.Volatile().RemoveComponent<Scene3DCom>();
			}
			return true;
		}
		break;
		case 4:
		{
			if (entity.HasComponent<Body3DCom>())
			{
				entity.Volatile().RemoveComponent<Body3DCom>();
			}
			return true;
		}
		break;
		case 5:
		{
			if (entity.HasComponent<LightCom>())
			{
				entity.Volatile().RemoveComponent<LightCom>();
			}
			return true;
		}
		break;
		case 6:
		{
			if (entity.HasComponent<IDNameCom>())
			{
				entity.Volatile().RemoveComponent<IDNameCom>();
			}
			return true;
		}
		break;
		case 7:
		{
			if (entity.HasComponent<Animation3DCom>())
			{
				entity.Volatile().RemoveComponent<Animation3DCom>();
			}
			return true;
		}
		break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unregistered component!");
			break;
		}
	}
	return false;
}

bool longmarch::ObjectFactory::RemoveComponentSystemByName(const std::string& comsys_type, GameWorld* world) const
{
	/*
		1, Modify this map for new compoent systems  TODO @yuhang : Refactor
	*/
	static const std::unordered_map<std::string, int> m{
	{"Scene3DComSys", 1},
	{"EntityGCComSys", 2},
	{"Transform3DComSys", 3},
	{"PerspectiveCameraComSys", 4},
	{"Body3DComSys", 5},
	{"Animation3DComSys", 6},
	};

	throw NotImplementedException();

	return false;
}

void longmarch::ObjectFactory::LoadLevel(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::ObjectFactory::SaveLevel(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::ObjectFactory::LoadResources(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::ObjectFactory::LoadSystems(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::ObjectFactory::LoadGameWorldScene(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::ObjectFactory::SaveGameWorldScene(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}
