#pragma once
#include "engine/core/EngineCore.h"
#include "BitMaskSignature.h"

#include <iostream>

namespace longmarch
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
        ComponentManager.h. To avoid false sharing, Entity class is 64 bytes aligned
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    struct MS_ALIGN8 Entity
    {
        EntityID m_id{0}; // Entity id, >0 is valid
        EntityType m_type{0};

        /*
        Default constructor creates : (EntityID:0, EntityType : 0).
            In your entity enum class, you should create an item called
            EMPTY which equals 0 that serves a empty entity.
        */
        Entity() = default;

        explicit Entity(EntityID id, EntityType type)
            :
            m_id(id),
            m_type(type)
        {
        }

        Entity(const Entity& other)
            : m_id(other.m_id),
              m_type(other.m_type)
        {
        }

        Entity(Entity&& other) noexcept
            : m_id(other.m_id),
              m_type(other.m_type)
        {
        }

        Entity& operator=(const Entity& other)
        {
            if (this == &other)
                return *this;
            m_id = other.m_id;
            m_type = other.m_type;
            return *this;
        }

        Entity& operator=(Entity&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_id = other.m_id;
            m_type = other.m_type;
            return *this;
        }

        bool Valid() const
        {
            return this->m_id > 0;
        }

        friend inline bool operator==(const Entity& lhs, const Entity& rhs)
        {
            return lhs.m_id == rhs.m_id && lhs.m_type == rhs.m_type;
        }

        friend inline bool operator<(const Entity& lhs, const Entity& rhs)
        {
            return lhs.m_id < rhs.m_id;
        }

        friend inline std::ostream& operator<<(std::ostream& os, const longmarch::Entity& e)
        {
            os << '(' << e.m_id << '/' << e.m_type << ')';
            return os;
        }

        friend inline std::wostream& operator<<(std::wostream& os, const longmarch::Entity& e)
        {
            os << '(' << e.m_id << '/' << e.m_type << ')';
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
    struct hash<longmarch::Entity>
    {
        std::size_t operator()(const longmarch::Entity& e) const noexcept
        {
            return hash<longmarch::EntityID>()(e.m_id);
        }
    };
}
