#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "editor/renderer/render-pass/PickingPass3D.h"
#include "editor/renderer/render-pass/OutlinePass3D.h"

namespace longmarch
{
    /**
     * @brief Editor's object picking sytem
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class EditorPickingComSys final : public BaseComponentSystem
    {
    public:
        NONCOPYABLE(EditorPickingComSys);
        COMSYS_DEFAULT_COPY(EditorPickingComSys);

        EditorPickingComSys() = default;
        virtual void Init() override;
        virtual void Render() override;
        virtual void Render2() override;
        virtual void RenderUI() override;

        void SetSceneDockDrawList(ImDrawList* drawList);

    private:
        void ManipulatePickedEntityGizmos(const Entity& e);

    private:
        PickingPass m_pickingPass;
        OutlinePass m_outlinePass;
        ImDrawList* m_sceneDockDrawList{nullptr};
    };
}
