#pragma once
#include "engine/math/Geommath.h"
#include "engine/renderer/Texture.h"
#include "engine/core/asset-manager/ResourceManager.h"

namespace longmarch
{
	class Material
	{
	public:
		enum class MAT_TEXTURE_TYPE : uint32_t
		{
			EMPTY = 0,
			ALBEDO,
			NORMAL,
			METALLIC,
			ROUGHNESS,
			AO,
			NUM
		};
	public:
		inline static void Init() { s_placeholder_Texture2D = Texture2D::Create(Texture::Setting()); }
		static std::shared_ptr<Material> LoadFromFile(const fs::path& path);
		static bool IsTextureValid(const std::shared_ptr<Texture2D>& tex);

		void SetTexture(const std::string& name, const fs::path& filepath, Material::MAT_TEXTURE_TYPE type);
		void UnsetTexture(Material::MAT_TEXTURE_TYPE type);
		void BindAllTexture(std::initializer_list<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>> textures_to_bind);
		void BindAllTexture(const LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>>& textures_to_bind);

	public:
		struct Textures
		{
			ResourceManager<Texture2D>::ResourceHandle albedo_texture{ nullptr };
			ResourceManager<Texture2D>::ResourceHandle normal_texture{ nullptr };
			ResourceManager<Texture2D>::ResourceHandle metallic_texture{ nullptr };
			ResourceManager<Texture2D>::ResourceHandle roughness_texture{ nullptr };
			ResourceManager<Texture2D>::ResourceHandle ao_texture{ nullptr };

			inline bool is_albedo_srgb() { return (IsTextureValid(albedo_texture->TryGet()) ? !albedo_texture->Get()->IsFloatType() : false); }
			inline bool has_albedo() { return IsTextureValid(albedo_texture->TryGet()); }
			inline bool has_normal() { return IsTextureValid(normal_texture->TryGet()); }
			inline bool has_metallic() { return IsTextureValid(metallic_texture->TryGet()); }
			inline bool has_roughness() { return IsTextureValid(roughness_texture->TryGet()); }
			inline bool has_ao() { return IsTextureValid(ao_texture->TryGet()); }
		} textures;
		Vec3f Kd{ Vec3f(.65) };
		float metallic{ 0.5f };
		float roughness{ .5f };
		bool emissive{ false };
	public:
		inline static std::shared_ptr<Texture2D> s_placeholder_Texture2D{ nullptr };
	};
}