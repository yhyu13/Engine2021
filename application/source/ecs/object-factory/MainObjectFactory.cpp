#include "application-precompiled-header.h"
#include "MainObjectFactory.h"
#include "engine/scene-graph/Scene3DManager.h"
#include "engine/ecs/header/header.h"
#include "engine/renderer/header/header.h"
#include "engine/audio/AudioManager.h"
#include "editor/ecs/header/header.h"
#include "ecs/header/header.h"

longmarch::MainObjectFactory::MainObjectFactory()
{
	s_instance = this;
	m_EntityNameToTypeList.emplace_back(GameEntity::TypeNameMap);
	m_EntityTypeToNameList.emplace_back(GameEntity::TypeNameMapInv);

	const LongMarch_Vector<std::string> components{
	};
	std::move(components.begin(), components.end(), std::back_inserter(m_ComponentNameList));

	const LongMarch_Vector<std::string> componentSys{
		"EditorCameraControllerComSys",
		"PlayerControllerComSys",
		"EditorPickingComSys",
		"NPCPathFindingControllerComSys",
		"AIControllerComSys",
		"Particle3DComSys",
	};
	std::move(componentSys.begin(), componentSys.end(), std::back_inserter(m_ComponentSystemNameList));
}

BaseComponentInterface* longmarch::MainObjectFactory::AddComponentByName(const std::string& com_type, EntityDecorator e) const
{
	auto com = ObjectFactory::AddComponentByName(com_type, e);
	if (!com)
	{
		/*
		 1, Modify this map for new compoent
		*/
		static const std::unordered_map<std::string, int> m{
		{"AIControllerCom", 1 },
		{"Particle3DCom", 2},
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
				if (!e.HasComponent<AIControllerCom>())
				{
					e.AddComponent(AIControllerCom(e));
				}
				com = e.GetComponent<AIControllerCom>();
			}
			break;
			case 2:
			{
				if (!e.HasComponent<Particle3DCom>())
				{
					e.AddComponent(Particle3DCom());
				}
				com = e.GetComponent<Particle3DCom>();
			}
			break;
			default:
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Component is not registered");
				break;
			}
		}
	}
	return com;
}

std::shared_ptr<BaseComponentSystem> longmarch::MainObjectFactory::AddComponentSystemByName(const std::string& comsys_type, GameWorld* world) const
{
	auto system = ObjectFactory::AddComponentSystemByName(comsys_type, world);
	if (!system)
	{
		/*
			1, Modify this map for new compoent systems
		*/
		static const std::unordered_map<std::string, int> m{
		{"EditorCameraControllerComSys", 1 },
		{"PlayerControllerComSys", 2 },
		{"EditorPickingComSys", 3 },
		{"NPCPathFindingControllerComSys", 4},
		{"AIControllerComSys", 5},
		{"Particle3DComSys", 7},
		};

		if (const auto& it = m.find(comsys_type); it != m.end())
		{
			/*
				2, Modify this switch statement for new compoent systems
			*/
			switch (it->second)
			{
			case 1:
				system = MemoryManager::Make_shared<EditorCameraControllerComSys>();
				break;
			case 2:
				system = MemoryManager::Make_shared<PlayerControllerComSys>();
				break;
			case 3:
				system = MemoryManager::Make_shared<EditorPickingComSys>();
				break;
			case 4:
				system = MemoryManager::Make_shared<NPCPathFindingControllerComSys>();
				break;
			case 5:
				system = MemoryManager::Make_shared<AIControllerComSys>();
				break;
			case 7:
				system = MemoryManager::Make_shared<Particle3DComSys>();
				break;			
			default:
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"System is not registered");
				break;
			}
			world->RegisterSystem(system, it->first);
		}
	}
	return system;
}

bool longmarch::MainObjectFactory::RemoveComponentByName(const std::string& com_type, EntityDecorator entity) const
{
	if (!ObjectFactory::RemoveComponentByName(com_type, entity))
	{
		throw NotImplementedException();
		return false;
	}
	return true;
}

bool longmarch::MainObjectFactory::RemoveComponentSystemByName(const std::string& comsys_type, GameWorld* world) const
{
	if (!ObjectFactory::RemoveComponentSystemByName(comsys_type, world))
	{
		throw NotImplementedException();
		return false;
	}
	return true;
}

void longmarch::MainObjectFactory::LoadLevel(const fs::path& filepath, GameWorld* world) const
{
	FileSystem::ExistCheck(filepath);
	throw NotImplementedException();
}

void longmarch::MainObjectFactory::SaveLevel(const fs::path& filepath, GameWorld* world) const
{
	throw NotImplementedException();
}

void longmarch::MainObjectFactory::LoadResources(const fs::path& filepath, GameWorld* world) const
{
	FileSystem::ExistCheck(filepath);
	const auto& json = FileSystem::GetCachedJsonCPP(filepath);

	// Multithread loading of resources (but still stall the calling thread to wait for all resources to be loaded)
	LongMarch_Vector<AssetLoader::LoaderRef> waitList;
	{
		const auto& polys = json["model"];
		ResourceManager<AssimpSceneObject>::GetInstance()->SetLoadFromFileFunc(AssimpSceneObject::LoadFromFile);
		for (Json::Value::ArrayIndex i = 0; i < polys.size(); ++i) {
			const auto& _item = polys[i];
			const auto& path = _item["path"].asString();
			const auto& id = _item["id"].asString();
			auto job = AssetLoader::Load(path, [id, this](const fs::path& path)->AssetLoader::DataSourceRef {
				try
				{
					auto res = ResourceManager<AssimpSceneObject>::GetInstance()->LoadFromFile(id, path)->Get();
					Scene3DManager::GetInstance()->LoadSceneNodeFromAssimp(id);
					return res;
				}
				catch (EngineException& e) { EngineException::Push(std::move(e)); }
				catch (std::exception& e) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception")); }
				catch (...) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception")); }
				}, [id](AssetLoader::DataSourceRef) {
					ResourceManager<AssimpSceneObject>::GetInstance()->Remove(id);
				}, AssetLoader::Options{ true, false }, false);
			waitList.emplace_back(job);
		}
	}
	{
		const auto& texture = json["texture"];
		ResourceManager<Image2D>::GetInstance()->SetLoadFromFileFunc(Image2D::LoadFromFile);
		for (Json::Value::ArrayIndex i = 0; i < texture.size(); ++i) {
			const auto& _item = texture[i];
			const auto& path = _item["path"].asString();
			const auto& id = _item["id"].asString();
			auto job = AssetLoader::Load(path, [id, this](const fs::path& path)->AssetLoader::DataSourceRef {
				try
				{
					return ResourceManager<Image2D>::GetInstance()->LoadFromFile(id, path)->Get();
				}
				catch (EngineException& e) { EngineException::Push(std::move(e)); }
				catch (std::exception& e) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception")); }
				catch (...) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception")); }
				}, [_item, id](AssetLoader::DataSourceRef data) {
					const auto& img = std::static_pointer_cast<Image2D>(data);
					Texture::Setting setting;
					setting.width = img->GetWidth();
					setting.height = img->GetHeight();
					setting.channels = img->GetChannels();
					setting.float_type = img->IsFloatType();
					setting.input_data = img->GetData();
					{
						auto v = _item["mipmap"];
						setting.has_mipmap = (!v.isNull()) ? v.asBool() : true;
					}
					{
						auto v = _item["linear_filter"];
						setting.linear_filter = (!v.isNull()) ? v.asBool() : true;
					}
					setting.rows = _item["rows"].isNull() ? 1 : _item["rows"].asInt();
					ResourceManager<Texture2D>::GetInstance()->AddResource(id, "", Texture2D::Create(setting)); //! Individual component should not have a path related to it
					ResourceManager<Image2D>::GetInstance()->Remove(id);
				}, AssetLoader::Options{ true, false }, false);
			waitList.emplace_back(job);
		}
	}

	{
		/**************************************************************
		*	FMOD Sound should be loaded in the main thread
		**************************************************************/
		const auto& sound = json["sound"];
		auto audioManger = AudioManager::GetInstance();
		for (Json::Value::ArrayIndex i = 0; i < sound.size(); ++i) {
			const auto& _item = sound[i];
			const auto& _path = _item["path"].asString();
			const auto& _id = _item["id"].asString();
			const auto& _loop = _item["loop"].asBool();
			audioManger->LoadSound(_id, _path, _loop);
		}
	}
	AssetLoader::WaitForAll(waitList);
}

void longmarch::MainObjectFactory::LoadSystems(const fs::path& filepath, GameWorld* world) const
{
	FileSystem::ExistCheck(filepath);
	Json::Value systems = FileSystem::GetCachedJsonCPP(filepath)["system"];

	for (auto i(0u); i < systems.size(); ++i)
	{
		auto system = AddComponentSystemByName(systems[i].asString(), world);
		if (!system)
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown system: " + str2wstr(systems[i].asString()));
		}
	}
	DEBUG_PRINT("Finish loading system.");
}

void longmarch::MainObjectFactory::LoadGameWorldScene(const fs::path& filepath, GameWorld* world) const
{
	FileSystem::ExistCheck(filepath);
	Json::Value doc;
	{
		ENG_TIME("Load:Parse");
		doc = FileSystem::GetCachedJsonCPP(filepath);
	}
	{
		ENG_TIME("Load:Deserialize");
		auto& roots = doc["root"];
		ASSERT(roots.isArray(), "LoadGameWorldScene: root must be an array!");
		for (auto i(0u); i < roots.size(); ++i)
		{
			DeserializeEntity(EntityDecorator(), roots[i], world, 0);
		}
	}
}

void longmarch::MainObjectFactory::DeserializeEntity(EntityDecorator parentEntity, const Json::Value& object, GameWorld* world, int level) const
{
	const auto& type = object["0_type"];
	if (type.isNull())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Entity type should not be null!");
	}

	auto s_type = type.asString();
	auto e_type = GetEntityTypeFromName(s_type);

	auto entity = world->GenerateEntity3D(e_type, true, false);
	DEBUG_PRINT(std::string("   ", level) + Str(level) + " " + type.asString());

	if (level > 0)
	{
		auto parentCom = entity.GetComponent<ParentCom>();
		parentCom->SetParent(parentEntity);
	}

	const auto& com = object["1_com"];
	if (!com.isNull())
	{
		for (auto i(0u); i < com.size(); ++i)
		{
			const auto& _com = com[i];
			DEBUG_PRINT(std::string("   ", level) + " --" + _com["id"].asString());
			DeserializedCom(entity, _com["id"], _com["value"]);
		}
	}
	const auto& child = object["2_child"];
	if (!child.isNull())
	{
		for (auto i(0u); i < child.size(); ++i)
		{
			const auto& _child = child[i];
			DeserializeEntity(entity, _child, world, level + 1);
		}
	}
}

void longmarch::MainObjectFactory::DeserializedCom(EntityDecorator entity, const Json::Value& id, const Json::Value& value) const
{
	if (id.isNull())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Component id is not specified!");
	}

	auto com = AddComponentByName(id.asString(), entity);
	if (!com)
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown component: " + str2wstr(id.asString()));
	}
	com->JsonDeserialize(value);
}

void longmarch::MainObjectFactory::SaveGameWorldScene(const fs::path& filepath, GameWorld* world) const
{
	FileSystem::ExistCheck(filepath);
	Json::Value doc;
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "    ";
	builder["precision"] = 4;
	builder["precisionType"] = "decimal";
	builder["dropNullPlaceholders"] = false;
	std::unique_ptr<Json::StreamWriter> writer(
		builder.newStreamWriter());
	{
		ENG_TIME("Save:Serialize");
		// Write entities
		SerializeEntity(EntityDecorator(), doc["root"], world, 0);

		// Write system as well
		auto& system_val = doc["system"];
		const auto& names = world->GetAllComponentSystemName();
		for (auto&& name : names)
		{
			system_val.append(Json::Value(name));
		}
	}
	{
		ENG_TIME("Save:Write");
		auto& output = FileSystem::OpenOfstream(filepath, FileSystem::FileType::OPEN_BINARY);
		writer->write(doc, &output);
		FileSystem::CloseOfstream(filepath);
		FileSystem::RemoveCachedJsonCPP(filepath);
		DEBUG_PRINT("Save game world to " + filepath.string());
	}
}

void longmarch::MainObjectFactory::SerializeEntity(EntityDecorator parentEntity, Json::Value& object, GameWorld* world, int level) const
{
	if (level == 0)
	{
		auto root = world->GetTheOnlyEntityWithType((EntityType)EngineEntityType::SCENE_ROOT);
		{
			auto rootEntity = EntityDecorator(root, world);
			Json::Value childVal(Json::arrayValue);
			SerializeEntity(rootEntity, childVal, world, level + 1);
			auto val = SerializedCom(rootEntity);
			val["2_child"] = std::move(childVal);
			object.append(std::move(val));
		}
	}
	else
	{
		for (auto& child : world->GetComponent<ChildrenCom>(parentEntity)->GetChildren())
		{
			auto childEntity = EntityDecorator(child, world);
			Json::Value childVal(Json::arrayValue);
			SerializeEntity(childEntity, childVal, world, level + 1);
			auto val = SerializedCom(childEntity);
			val["2_child"] = std::move(childVal);
			object.append(std::move(val));
		}
	}
}

Json::Value longmarch::MainObjectFactory::SerializedCom(EntityDecorator entity) const
{
	Json::Value val;
	{
		auto e_type = entity.GetType();
		auto s_type = GetEntityNameFromType(e_type);
		val["0_type"] = std::move(s_type);

		Json::Value coms(Json::arrayValue);
		for (auto& com : entity.GetAllComponent())
		{
			com->JsonSerialize(coms);
		}
		val["1_com"] = std::move(coms);
	}
	return val;
}