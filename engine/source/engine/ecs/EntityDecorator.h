#pragma once
#include "Entity.h"
#include "BaseComponent.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/utility/TypeHelper.h"

namespace AAAAgames
{
	class GameWorld;

	template<typename ComponentType>
	class ComponentDecorator;

	/**
	 * @brief Entity decorators are the wrapper constructs for entities.
	 *
	 *	These decorators help in creating useful abstractions for
	 *	entity specific operations.
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */

	struct EntityDecorator
	{
		EntityDecorator() = default;
		explicit EntityDecorator(const Entity& entity, GameWorld* world)
			:
			m_entity(entity),
			m_world(world)
		{}

		template<typename ComponentType>
		void AddComponent(const ComponentType& component);

		template<typename ComponentType>
		void RemoveComponent();

		template<typename ComponentType>
		ComponentDecorator<ComponentType> GetComponent();

		template<typename ComponentType>
		bool HasComponent();

		__A4GAMES_TRVIAL_TEMPLATE__
		A4GAMES_Vector<BaseComponentInterface*> GetAllComponent();

		inline bool Valid() const
		{
			return m_entity != Entity() && m_world != nullptr;
		}

		//! Reset entity decocrator (aka. invalidate it)
		inline void Reset()
		{
			m_entity = Entity(); 
			m_world = nullptr;
		}

		inline const Entity GetEntity() const
		{
			return m_entity;
		}

		inline const EntityID GetID() const
		{
			return m_entity.m_id;
		}

		inline const EntityType GetType() const
		{
			return m_entity.m_type;
		}

		inline GameWorld* GetWorld() const
		{
			return m_world;
		}

		inline operator Entity() const
		{
			return m_entity;
		}

		inline operator EntityID() const
		{
			return m_entity.m_id;
		}

		inline operator EntityType() const
		{
			return m_entity.m_type;
		}

	private:
		Entity m_entity;
		GameWorld* m_world{ nullptr };
	};
}

__A4GAMES_TRVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& os, const AAAAgames::EntityDecorator& ed);

__A4GAMES_TRVIAL_TEMPLATE__
bool operator==(const EntityDecorator& lhs, const EntityDecorator& rhs);

__A4GAMES_TRVIAL_TEMPLATE__
bool operator!=(const EntityDecorator& lhs, const EntityDecorator& rhs);

__A4GAMES_TRVIAL_TEMPLATE__
bool operator<(const EntityDecorator& lhs, const EntityDecorator& rhs);

/*
	Custum hash function for EntityDecorator
*/
namespace std
{
	template <>
	struct hash<AAAAgames::EntityDecorator>
	{
		std::size_t operator()(const AAAAgames::EntityDecorator& e) const
		{
			return hash<AAAAgames::EntityID>()(e.GetID());
		}
	};
}