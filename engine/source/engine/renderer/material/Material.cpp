#include "engine-precompiled-header.h"
#include "Material.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/asset-manager/AssetLoader.h"
#include "engine/renderer/Image2D.h"

#define ASYNC_LOAD

std::shared_ptr<Material> longmarch::Material::LoadFromFile(const fs::path& path)
{
	// TODO : implement material load from file function
	throw NotImplementedException();
	return nullptr;
}

bool longmarch::Material::IsTextureValid(const std::shared_ptr<Texture2D>& tex)
{
	if (tex)
	{
		return tex != s_placeholder_Texture2D;
	}
	else
	{
		return false;
	}
}

void longmarch::Material::BindAllTexture(std::initializer_list<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>> textures_to_bind)
{
	BindAllTexture(LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>>(textures_to_bind));
}

void longmarch::Material::BindAllTexture(const LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>>& textures_to_bind)
{
	static auto _resolveTexture = [](ResourceManager<Texture2D>::ResourceHandle& handle) -> std::shared_ptr<Texture2D>
	{
		if (auto ptr = handle->TryGet(); ptr)
		{
			return ptr;
		}
		else
		{
			return s_placeholder_Texture2D;
		}
	};
	for (const auto& [slot, type] : textures_to_bind)
	{
		switch (type)
		{
		case Material::MAT_TEXTURE_TYPE::ALBEDO:
			_resolveTexture(textures.albedo_texture)->BindTexture(slot);
			break;
		case Material::MAT_TEXTURE_TYPE::NORMAL:
			_resolveTexture(textures.normal_texture)->BindTexture(slot);
			break;
		case Material::MAT_TEXTURE_TYPE::METALLIC:
			_resolveTexture(textures.metallic_texture)->BindTexture(slot);
			break;
		case Material::MAT_TEXTURE_TYPE::ROUGHNESS:
			_resolveTexture(textures.roughness_texture)->BindTexture(slot);
			break;
		case Material::MAT_TEXTURE_TYPE::BACKEDAO:
			_resolveTexture(textures.ao_texture)->BindTexture(slot);
			break;
		default:
			ENGINE_EXCEPT(L"Invalid texture type!");
		}
	}
}

void longmarch::Material::SetTexture(const std::string& name, const fs::path& filepath, Material::MAT_TEXTURE_TYPE type)
{
	auto resourceManager = ResourceManager<Texture2D>::GetInstance();
	if (!resourceManager->HasLoadFromFileFunc())
	{
		resourceManager->SetLoadFromFileFunc(Texture2D::LoadFromFile);
	}
#if defined(ASYNC_LOAD)
	static auto AsyncLoadMaterial = [](const std::string& id, const fs::path& path)
	{
		auto resourceManager = ResourceManager<Image2D>::GetInstance();
		if (!resourceManager->HasLoadFromFileFunc())
		{
			resourceManager->SetLoadFromFileFunc(Image2D::LoadFromFile);
		}
		AssetLoader::Load(path, [id](const fs::path& path)->AssetLoader::DataSourceRef {
			try
			{
				return ResourceManager<Image2D>::GetInstance()->LoadFromFile(id, path)->Get();
			}
			catch (EngineException& e) { EngineException::Push(std::move(e)); }
			catch (std::exception& e) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, wStr(e.what()), L"STL Exception")); }
			catch (...) { EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lib or dll exception", L"Non-STL Exception")); }
			return nullptr;
		}, [id](AssetLoader::DataSourceRef data) {
			const auto& img = std::static_pointer_cast<Image2D>(data);
			Texture::Setting setting;
			setting.width = img->GetWidth();
			setting.height = img->GetHeight();
			setting.channels = img->GetChannels();
			setting.float_type = img->IsFloatType();
			setting.input_data = img->GetData();
			setting.has_mipmap = true;
			setting.linear_filter = true;
			// Add texture in the main thread
			ResourceManager<Texture2D>::GetInstance()->AddResource(id, "", Texture2D::Create(setting)); //! Individual component should not have a path related to it
			// Remove image 2d
			ResourceManager<Image2D>::GetInstance()->Remove(id);
		}, { true, false }, true);
	};
	switch (type)
	{
	case Material::MAT_TEXTURE_TYPE::ALBEDO:
		textures.albedo_texture = resourceManager->TryGet(name);
		// If a material is not loaded, the resource handle should contain a future generated by a promise
		// We then call the async loading to add that resource which fulfill the promise.
		if (!textures.albedo_texture->TryGet() && textures.albedo_texture->IsFutureValid())
		{
			AsyncLoadMaterial(name, filepath);
		}
		break;
	case Material::MAT_TEXTURE_TYPE::NORMAL:
		textures.normal_texture = resourceManager->TryGet(name);
		if (!textures.normal_texture->TryGet() && textures.normal_texture->IsFutureValid())
		{
			AsyncLoadMaterial(name, filepath);
		}
		break;
	case Material::MAT_TEXTURE_TYPE::METALLIC:
		textures.metallic_texture = resourceManager->TryGet(name);
		if (!textures.metallic_texture->TryGet() && textures.metallic_texture->IsFutureValid())
		{
			AsyncLoadMaterial(name, filepath);
		}
		break;
	case Material::MAT_TEXTURE_TYPE::ROUGHNESS:
		textures.roughness_texture = resourceManager->TryGet(name);
		if (!textures.roughness_texture->TryGet() && textures.roughness_texture->IsFutureValid())
		{
			AsyncLoadMaterial(name, filepath);
		}
		break;
	case Material::MAT_TEXTURE_TYPE::BACKEDAO:
		textures.ao_texture = resourceManager->TryGet(name);
		if (!textures.ao_texture->TryGet() && textures.ao_texture->IsFutureValid())
		{
			AsyncLoadMaterial(name, filepath);
		}
		break;
	default:
		ENGINE_EXCEPT(L"Invalid texture type!");
	}
#else
	switch (type)
	{
	case Material::MAT_TEXTURE_TYPE::ALBEDO:
		textures.albedo_texture = resourceManager->LoadFromFile(name, filepath);
		break;
	case Material::MAT_TEXTURE_TYPE::NORMAL:
		textures.normal_texture = resourceManager->LoadFromFile(name, filepath);
		break;
	case Material::MAT_TEXTURE_TYPE::METALLIC:
		textures.metallic_texture = resourceManager->LoadFromFile(name, filepath);
		break;
	case Material::MAT_TEXTURE_TYPE::ROUGHNESS:
		textures.roughness_texture = resourceManager->LoadFromFile(name, filepath);
		break;
	case Material::MAT_TEXTURE_TYPE::AO:
		textures.ao_texture = resourceManager->LoadFromFile(name, filepath);
		break;
	default:
		ENGINE_EXCEPT(L"Invalid texture type!");
	}
#endif
}

void longmarch::Material::UnsetTexture(Material::MAT_TEXTURE_TYPE type)
{
	switch (type)
	{
	case Material::MAT_TEXTURE_TYPE::ALBEDO:
		textures.albedo_texture = s_placeholder_Texture2D;
		break;
	case Material::MAT_TEXTURE_TYPE::NORMAL:
		textures.normal_texture = s_placeholder_Texture2D;
		break;
	case Material::MAT_TEXTURE_TYPE::METALLIC:
		textures.metallic_texture = s_placeholder_Texture2D;
		break;
	case Material::MAT_TEXTURE_TYPE::ROUGHNESS:
		textures.roughness_texture = s_placeholder_Texture2D;
		break;
	case Material::MAT_TEXTURE_TYPE::BACKEDAO:
		textures.ao_texture = s_placeholder_Texture2D;
		break;
	default:
		ENGINE_EXCEPT(L"Invalid texture type!");
	}
}