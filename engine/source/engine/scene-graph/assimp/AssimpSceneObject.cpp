#include "engine-precompiled-header.h"
#include "AssimpSceneObject.h"

AAAAgames::AssimpSceneObject::AssimpSceneObject(const fs::path& path)
{
	// Reference: http://assimp.sourceforge.net/lib_html/threading.html
	// The C-API is thread safe.
	auto _path = path.string();
	DEBUG_PRINT("Reading " + _path);
	m_aiscene = aiImportFile(_path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph);
	auto erro = wStr(aiGetErrorString());
	ENGINE_EXCEPT_IF(!m_aiscene, L"Failed to load assimp file: " + path.wstring() + L" with Error: " + erro);
}

AAAAgames::AssimpSceneObject::~AssimpSceneObject()
{
	aiReleaseImport(m_aiscene);
}

const aiScene* AAAAgames::AssimpSceneObject::GetScene() const noexcept
{
	return m_aiscene;
}

const aiNode* AAAAgames::AssimpSceneObject::GetRoot() const noexcept
{
	return m_aiscene->mRootNode;
}
