#pragma once

#include "engine/core/utility/JsonHelper.h"
#include "engine/ui/ImGuiDriver.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/exception/EngineException.h"

#define ENABLE_COMPONENT_LOCK_DEFAULT 0 // By default, turn component wise lock to false

namespace longmarch
{
    class GameWorld;
    typedef uint32_t ComponentTypeIndex_T;

    /**
     * @brief Each component-type gets a type-index. The type-index is used in calculating
        component-signatures (bit-masks).
     *
     * For more information, please check BitMaskSignature.h.
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    struct ComponentTypeIndex
    {
        inline static std::atomic<ComponentTypeIndex_T> s_index = {0};
    };

    /**
     * @brief Interaface class for all components
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    struct BaseComponentInterface
    {
        virtual ~BaseComponentInterface() = default;
        //! JsonCPP deserialization
        virtual void JsonDeserialize(const Json::Value& value)
        {
        }

        //! JsonCPP serialization
        virtual void JsonSerialize(Json::Value& value) const
        {
        }

        //! Editor component inspection
        virtual void ImGuiRender()
        {
        }

        //! Customize copying data from one component to another
        virtual void Copy(BaseComponentInterface* other)
        {
        }
    };

    /**
     * @brief Every component in the ECS architecture is simply a data-storage unit.
     *
     * To create new components:

        // CRTP
        struct Transform : public BaseComponent<Transform> {
            explicit Transform(float x, float y) : x(x), y(y) {};
            float x;
            float y;
        };
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    template <typename ComponentType>
    struct BaseComponent : protected BaseAtomicClass, public BaseComponentInterface
    {
        BaseComponent()
        {
            m_atomic_lock_enabled = ENABLE_COMPONENT_LOCK_DEFAULT;
        }

        explicit BaseComponent(GameWorld* world) : m_world(world)
        {
            m_atomic_lock_enabled = ENABLE_COMPONENT_LOCK_DEFAULT;
        }

        virtual ~BaseComponent() = default;

        //! Set the component to be in a new world
        void SetWorld(GameWorld* world) const
        {
            m_world = world;
        }

        //! Reset component datas to default values
        void Reset()
        {
            static auto _default = ComponentType();
            this->Copy(&_default);
        }

        //! All components of a given type belong to the same type-index. For example, all Position components have the same type-index.
        static const ComponentTypeIndex_T TypeIndex()
        {
            static const ComponentTypeIndex_T index = ComponentTypeIndex::s_index++;
            return index;
        }

    public:
        mutable GameWorld* m_world{nullptr};
    };

    /**
     * @brief Use this function to get the family-code for a component-type.
              Example usage: GetComponentFamily<Transform>()
     *
     * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
     */
    template <typename ComponentType>
    static const ComponentTypeIndex_T GetComponentTypeIndex()
    {
        return BaseComponent<ComponentType>::TypeIndex();
    }
}
