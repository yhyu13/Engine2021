#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/scene-graph/Scene3DNode.h"


namespace longmarch
{
	struct Transform3DCom;
	struct Animation3DCom;

	/* Data class of mesh, material */
	struct CACHE_ALIGN32 Scene3DCom final : BaseComponent<Scene3DCom>
	{
		using SceneDataRef = std::shared_ptr<Scene3DNode>;

		Scene3DCom() = default;
		explicit Scene3DCom(const EntityDecorator& _this);

		void SetSceneData(const SceneDataRef& obj);
		void SetSceneData(const ResourceManager<Scene3DNode>::ResourceHandle& handle);
		//! Return scene data reference, could wait on potential resource handle or not
		SceneDataRef& GetSceneData(bool waitOnHandle);

		void SetShaderName(const std::string& name);

		bool IsVisible() const;
		void SetVisiable(bool b);
		bool IsHideInGame() const;
		void SetHideInGame(bool b);
		bool IsCastShadow() const;
		void SetCastShadow(bool b);
		bool IsCastReflection() const;
		void SetCastReflection(bool b);
		bool IsTranslucenctRendering() const;
		void SetTranslucenctRendering(bool v);
		int GetTranslucencySortPriority() const;
		void SetTranslucencySortPriority(int v);

		/*
			Drawable flag will be reset to true upon calling the Render() function,
			meaning SetDrawable() shall be called before each Render() call.
		*/
		void SetShouldDraw(bool b, bool _override = true);


		void Draw(const std::function<void(const Renderer3D::RenderData_CPU&)>& drawFunc);
		void Draw();

		virtual void JsonSerialize(Json::Value& value) override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;

	private:
		ResourceManager<Scene3DNode>::ResourceHandle m_objDatasHandle{ nullptr };
		SceneDataRef m_objDatasRef{ nullptr }; // Reference to the scene meshs and materials that this component would draw
		
		std::string m_shaderName = { "" }; // Name of a custom shader used to rendering

		Entity m_this; // Bookkeeper of which entity this component belong to
		
		float m_translucencyAlpha{ 1.0f };
		int m_translucencySortPriority{ 0 };
		bool m_translucent{ false };

		bool m_shoudlDraw{ true };
		bool m_visible{ true };
		bool m_hideInGame{ false };
		bool m_castReflection{ true };
		bool m_castShadow{ true };
	};
}