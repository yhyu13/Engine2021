#pragma once
#include "engine/EngineEssential.h"
#include "Entity.h"

namespace longmarch
{
    /**
     * @brief Entity managers takes care of assigning entity-ids to entities during their creations.
     *
     *  It makes sure that no two entities within
     *	the system share the same entity-id.
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    class EntityManager
    {
    public:
        NONCOPYABLE(EntityManager);
        EntityManager() = default;

        inline const Entity Create(EntityType type)
        {
            auto ret = Entity{++m_entityID, type};
            m_typeToEntity[type].push_back(ret);
            return ret;
        }

        // Should use the same poping strategy as component manager to counter memory diffusion
        inline void Destroy(const Entity& entity)
        {
            LongMarch_EraseFirst(m_typeToEntity[entity.m_type], entity);
        }

        inline void RemoveAll()
        {
            m_typeToEntity.clear();
            m_entityID = 0;
        }

        inline const LongMarch_Vector<Entity> GetAllEntitiesWithType(EntityType type) const
        {
            if (auto it = m_typeToEntity.find(type); it != m_typeToEntity.end())
            {
                return it->second;
            }
            else
            {
                return LongMarch_Vector<Entity>();
            }
        }

        inline Entity GetEntityFromID(EntityID ID) const
        {
            for (const auto& [type, entities] : m_typeToEntity)
            {
                for (const auto& e : entities)
                {
                    if (ID == e.m_id)
                    {
                        return e;
                    }
                }
            }
            return Entity();
        }

        inline std::shared_ptr<EntityManager> Copy() const
        {
            auto ret = MemoryManager::Make_shared<EntityManager>();
            ret->m_typeToEntity = m_typeToEntity;
            ret->m_entityID = m_entityID;
            return ret;
        }

    private:
        LongMarch_UnorderedMap_Par_node<EntityType, LongMarch_Vector<Entity>> m_typeToEntity;
        EntityID m_entityID{0};
    };
}
