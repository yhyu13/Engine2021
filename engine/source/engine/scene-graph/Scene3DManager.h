#pragma once
#include "engine/scene-graph/assimp/AssimpSceneObject.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/ecs/components/3d/Scene3DCom.h"

namespace longmarch
{
    /**
     * @brief Manges scene file loading and intializing Scene3DCom
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class Scene3DManager
    {
    private:
        Scene3DManager() = default;
        NONCOPYABLE(Scene3DManager);
    public:
        static Scene3DManager* GetInstance()
        {
            static Scene3DManager instance;
            return &instance;
        }

        void LoadSceneNodeFromAssimp(const std::string& sceneNodeName);
        void LoadSceneNodeToEntity(EntityDecorator rootEntity, const std::string& sceneNodeName);
    private:
        void RecurseLoad(Scene3DNode& sceneData, const std::string& sceneName, const fs::path& sceneDir,
                         const aiScene* aiscene, const aiNode* node, const aiMatrix4x4& parentTr, unsigned int level);
    };
}
