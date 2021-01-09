#include "engine-precompiled-header.h"
#include "AssimpSceneObject.h"

longmarch::AssimpSceneObject::AssimpSceneObject(const fs::path& path)
{
	// Reference: http://assimp.sourceforge.net/lib_html/threading.html
	// The C-API is thread safe.
	auto _path = path.string();
	DEBUG_PRINT("Reading " + _path);
	m_aiscene = aiImportFile(_path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph);
	auto erro = wStr(aiGetErrorString());
	ENGINE_EXCEPT_IF(!m_aiscene, L"Failed to load assimp file: " + path.wstring() + L" with Error: " + erro);
}

longmarch::AssimpSceneObject::~AssimpSceneObject()
{
	aiReleaseImport(m_aiscene);
}

const aiScene* longmarch::AssimpSceneObject::GetScene() const noexcept
{
	return m_aiscene;
}

const aiNode* longmarch::AssimpSceneObject::GetRoot() const noexcept
{
	return m_aiscene->mRootNode;
}
