#pragma once
#include "engine/core/EngineCore.h"
#include <iostream>

namespace AAAAgames
{
	typedef uint32_t EntityID;
	typedef int32_t EntityType;

	/**
	 * @brief Default constructor creates: (EntityID:0, EntityType:0).
		In your entity enum class, you should create an item called
		EMPTY which equals 0 that serves a empty entity

	 * @detail The entities are just the data containers. Each entity gets an
		id. Entities and their components are held together by component
		managers.For more information on component-managers, please check
		ComponentManager.h.
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	struct CACHE_ALIGN8 Entity
	{
		EntityID m_id{ 0 };
		EntityType m_type{ 0 };

		/*
		Default constructor creates : (EntityID:0, EntityType : 0).
			In your entity enum class, you should create an item called
			EMPTY which equals 0 that serves a empty entity.
		*/
		Entity() = default;

		/*
		Default constructor creates : (EntityID:0, EntityType : 0).
			In your entity enum class, you should create an item called
			EMPTY which equals 0 that serves a empty entity.
		*/
		explicit Entity(EntityID id, EntityType type)
			:
			m_id(id),
			m_type(type)
		{}

		bool Valid() const
		{
			return *this != Entity();
		}

		friend inline bool operator==(const Entity& lhs, const Entity& rhs) {
			return lhs.m_id == rhs.m_id && lhs.m_type == rhs.m_type;
		}

		friend inline bool operator!=(const Entity& lhs, const Entity& rhs) {
			return !(lhs.m_id == rhs.m_id && lhs.m_type == rhs.m_type);
		}

		friend inline bool operator<(const Entity& lhs, const Entity& rhs) {
			return lhs.m_id < rhs.m_id;
		}

		friend inline std::ostream& operator<<(std::ostream& os, const AAAAgames::Entity& dt)
		{
			os << dt.m_id << '/' << dt.m_type;
			return os;
		}
	};
}

/*
	Custum hash function for Entity
*/
namespace std
{
	template <>
	struct hash<AAAAgames::Entity>
	{
		std::size_t operator()(const AAAAgames::Entity& e) const
		{
			return hash<AAAAgames::EntityID>()(e.m_id);
		}
	};
}