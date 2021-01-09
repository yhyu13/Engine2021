#include "engine-precompiled-header.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "Scene3DManager.h"

void AAAAgames::Scene3DManager::LoadSceneNodeFromAssimp(const std::string& sceneNodeName)
{
	auto sceneNode = ResourceManager<AssimpSceneObject>::GetInstance()->TryGet(sceneNodeName)->Get();
	auto path = ResourceManager<AssimpSceneObject>::GetInstance()->GetPath(sceneNode);
	Scene3DNode sceneData{ sceneNodeName };
	RecurseLoad(sceneData, sceneNodeName, path.remove_filename(), sceneNode->GetScene(), sceneNode->GetRoot(), AssimpHelper::Glm2Assimp(Mat4(1.0f)), 0);

	/**************************************************************
	*	Register the loaded mesh data
	**************************************************************/
	auto rm = ResourceManager<Scene3DNode>::GetInstance();
	rm->AddResource(sceneNodeName, path, MemoryManager::Make_shared<Scene3DNode>(std::move(sceneData)));
	// Register load from file func if it has not been set yet
	if (!rm->HasLoadFromFileFunc())
	{
		rm->SetLoadFromFileFunc([this](const fs::path& path)->std::shared_ptr<Scene3DNode>
		{
			auto sceneNode = ResourceManager<AssimpSceneObject>::GetInstance()->LoadFromFile(path.string(), path)->Get();
			auto name = ResourceManager<AssimpSceneObject>::GetInstance()->GetName(sceneNode);
			Scene3DNode sceneData{ name };
			RecurseLoad(sceneData, name, fs::path(path).remove_filename(), sceneNode->GetScene(), sceneNode->GetRoot(), AssimpHelper::Glm2Assimp(Mat4(1.0f)), 0);
			return MemoryManager::Make_shared<Scene3DNode>(std::move(sceneData));
		});
	}
}

void AAAAgames::Scene3DManager::LoadSceneNodeToEntity(EntityDecorator rootEntity, const std::string& sceneNodeName)
{
	DEBUG_PRINT("Loading scene node: " + sceneNodeName);
	if (auto scene = rootEntity.GetComponent<Scene3DCom>(); scene.Valid())
	{
		if (auto resourceManager = ResourceManager<Scene3DNode>::GetInstance();
			resourceManager->Has(sceneNodeName))
		{
			scene->SetSceneData(resourceManager->TryGet(sceneNodeName)->Get());
		}
		else
		{
			scene->SetSceneData(resourceManager->TryGet(sceneNodeName));
		}
	}
}

void AAAAgames::Scene3DManager::RecurseLoad(Scene3DNode& sceneData, const std::string& sceneName, const fs::path& sceneDir, const aiScene* aiscene, const aiNode* node, const aiMatrix4x4& parentTr, unsigned int level)
{
	// Partial Credit : Prof. Gary Herron @ CS541,CS460/560 DigiPen Institute of Technology 2020/2021
	if (level == 0)
	{
		PRINT("Loading node : " + sceneName);
		PRINT(Str("  %d animations", aiscene->mNumAnimations)); // This is what 460/560 is all about
		PRINT(Str("  %d meshes", aiscene->mNumMeshes));         // Verts and faces for the skin.
		PRINT(Str("  %d materials", aiscene->mNumMaterials));   // More graphics info
		PRINT(Str("  %d textures", aiscene->mNumTextures));     // More graphics info
		// Prints a graphical representation of the bone hierarchy.
		AssimpHelper::ShowBoneHierarchy(aiscene, aiscene->mRootNode);
		// Prints all the animation info for each animation in the file
		for (unsigned int i = 0; i < aiscene->mNumAnimations; ++i)
		{
			AssimpHelper::ShowAnimation(aiscene->mAnimations[i]);
		}
		// Prints all the mesh info for each mesh in the file
		for (unsigned int i = 0; i < aiscene->mNumMeshes; ++i)
		{
			AssimpHelper::ShowMesh(aiscene->mMeshes[i]);
		}
		// Load skeleton and animation
		if (aiscene->mNumAnimations != 0)
		{
			auto skeleton = Skeleton::LoadSkeleton(aiscene, sceneName);
			ResourceManager<Skeleton>::GetInstance()->AddResource(sceneName, "", skeleton);
			auto animation = Animation3D::LoadAnimation(aiscene, sceneName);
			animation->SetSkeleton(skeleton);
			ResourceManager<Animation3D>::GetInstance()->AddResource(sceneName, "", animation);
		}
	}
	// Accumulating transformations while traversing down the hierarchy.
	aiMatrix4x4 childTr = parentTr * node->mTransformation; //TODO: whether or not to accumulate transformation
		// Inverse tranpose to get the normal transformation
	aiMatrix3x3 normalTr = aiMatrix3x3(childTr);
	normalTr.Inverse().Transpose();

	// Loop through this node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* aimesh = aiscene->mMeshes[node->mMeshes[i]];

		// Extract this node's surface material.
		aiString texPath;
		aiMaterial* mtl = aiscene->mMaterials[aimesh->mMaterialIndex];

		{
			auto material = MemoryManager::Make_shared<Material>();
			{
				// Albedo
				if (AI_SUCCESS == mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &texPath)
					|| AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
				{
					auto filename = texPath.C_Str();
					material->SetTexture(filename, sceneDir / filename, Material::MAT_TEXTURE_TYPE::ALBEDO);
				}
				else if (aiColor4D baseColorFactor; AI_SUCCESS == mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColorFactor))
				{
					material->Kd = { baseColorFactor.r, baseColorFactor.g, baseColorFactor.b };
				}
				else if (aiColor4D baseColorFactor; AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_DIFFUSE, baseColorFactor))
				{
					material->Kd = { baseColorFactor.r, baseColorFactor.g, baseColorFactor.b };
				}
				// Metallic
				if (AI_SUCCESS == mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &texPath)
					|| AI_SUCCESS == mtl->GetTexture(aiTextureType_METALNESS, 0, &texPath))
				{
					auto filename = texPath.C_Str();
					material->SetTexture(filename, sceneDir / filename, Material::MAT_TEXTURE_TYPE::METALLIC);
				}
				else if (ai_real metallicFactor; AI_SUCCESS == mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor))
				{
					material->metallic = metallicFactor;
				}
				// Roughness
				if (AI_SUCCESS == mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &texPath)
					|| AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath))
				{
					auto filename = texPath.C_Str();
					material->SetTexture(filename, sceneDir / filename, Material::MAT_TEXTURE_TYPE::ROUGHNESS);
				}
				else if (ai_real roughnessFactor; AI_SUCCESS == mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor))
				{
					material->roughness = roughnessFactor;
				}
				// Normal
				if (AI_SUCCESS == mtl->GetTexture(aiTextureType_NORMALS, 0, &texPath))
				{
					auto filename = texPath.C_Str();
					material->SetTexture(filename, sceneDir / filename, Material::MAT_TEXTURE_TYPE::NORMAL);
				}
				// Ambient Occlusion
				if (AI_SUCCESS == mtl->GetTexture(aiTextureType_LIGHTMAP, 0, &texPath)
					|| AI_SUCCESS == mtl->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texPath))
				{
					auto filename = texPath.C_Str();
					material->SetTexture(filename, sceneDir / filename, Material::MAT_TEXTURE_TYPE::AO);
				}
			}

			// Arrays to hold all vertex and triangle data.
			auto meshdata = MemoryManager::Make_shared<MeshData>();
			{
				// Loop through all vertices and record the
				// vertex/normal/texture/tangent data with the node's model
				// transformation applied.
				meshdata->vertices.reserve(aimesh->mNumVertices);
				for (unsigned int t = 0; t < aimesh->mNumVertices; ++t)
				{
					aiVector3D aipnt = childTr * aimesh->mVertices[t];
					aiVector3D ainrm = aimesh->HasNormals() ? normalTr * aimesh->mNormals[t] : aiVector3D(0, 0, 1);
					aiVector3D aitex = aimesh->HasTextureCoords(0) ? aimesh->mTextureCoords[0][t] : aiVector3D(0, 0, 0);
					aiVector3D aitan = aimesh->HasTangentsAndBitangents() ? normalTr * aimesh->mTangents[t] : aiVector3D(1, 0, 0);
#if MESH_VERTEX_DATA_FORMAT == 0
					meshdata->vertices.emplace_back(Vec3f(aipnt.x, aipnt.y, aipnt.z),
						Vec3f(ainrm.x, ainrm.y, ainrm.z),
						Vec2f(aitex.x, aitex.y),
						Vec3f(aitan.x, aitan.y, aitan.z));
#elif MESH_VERTEX_DATA_FORMAT == 1
					const auto& N = Vec3f(ainrm.x, ainrm.y, ainrm.z);
					const auto& T = Vec3f(aitan.x, aitan.y, aitan.z);
					const auto& B = glm::cross(T, N);
					const auto& qTangent = (Geommath::ToQTangent(N, T, B));
					meshdata->vertices.emplace_back(Vec3f(aipnt.x, aipnt.y, aipnt.z),
						Vec4f(qTangent.x, qTangent.y, qTangent.z, qTangent.w),
						Vec2f(aitex.x, aitex.y));
#elif MESH_VERTEX_DATA_FORMAT == 2
					const auto& N = Vec3f(ainrm.x, ainrm.y, ainrm.z);
					const auto& T = Vec3f(aitan.x, aitan.y, aitan.z);
					meshdata->vertices.emplace_back(Vec3f(aipnt.x, aipnt.y, aipnt.z),
						(Vec2f(Geommath::CartesianToSpherical(N).yz)),
						(Vec2f(aitex.x, aitex.y)),
						(Vec2f(Geommath::CartesianToSpherical(T).yz)));
#elif MESH_VERTEX_DATA_FORMAT == 3
					const auto& N = Vec3f(ainrm.x, ainrm.y, ainrm.z);
					const auto& T = Vec3f(aitan.x, aitan.y, aitan.z);
					meshdata->vertices.emplace_back(Vec3f(aipnt.x, aipnt.y, aipnt.z),
						Geommath::PackVec2ToHVec2(Vec2f(Geommath::CartesianToSpherical(N).yz)),
						Geommath::PackVec2ToHVec2(Vec2f(aitex.x, aitex.y)),
						Geommath::PackVec2ToHVec2(Vec2f(Geommath::CartesianToSpherical(T).yz)));
#elif MESH_VERTEX_DATA_FORMAT == 4
					constexpr float s = 2048.f;
					constexpr float scale = 1.0f / s;
					const auto& N = Vec3f(ainrm.x, ainrm.y, ainrm.z);
					const auto& T = Vec3f(aitan.x, aitan.y, aitan.z);
					meshdata->vertices.emplace_back(Geommath::PackVec4ToHVec4(Vec4f(aipnt.x * scale, aipnt.y * scale, aipnt.z * scale, s)),
						Geommath::PackVec2ToHVec2(Vec2f(Geommath::CartesianToSpherical(N).yz)),
						Geommath::PackVec2ToHVec2(Vec2f(aitex.x, aitex.y)),
						Geommath::PackVec2ToHVec2(Vec2f(Geommath::CartesianToSpherical(T).yz)));
#endif
				}

				// Loop through all faces, recording indices
				meshdata->indices.reserve(aimesh->mNumFaces);
				for (unsigned int i = 0; i < aimesh->mNumFaces; ++i)
				{
					aiFace* aiface = &aimesh->mFaces[i];
					ENGINE_EXCEPT_IF(aiface->mNumIndices != 3, L"A Triangle mesh " + wStr(node->mName.data) + L" has " + wStr(aiface->mNumIndices) + L" indices instead of 3!");
					meshdata->indices.emplace_back(aiface->mIndices[0], aiface->mIndices[1], aiface->mIndices[2]);
				}
			}

#if MESH_VERTEX_DATA_FORMAT == 4
			{
				for (unsigned int i = 0; i < aimesh->mNumBones; ++i)
				{
					const aiBone* bone = aimesh->mBones[i];
					const std::string boneName(bone->mName.C_Str());
					//! Fill in bond info and bone index
					const auto& skeletonRef = ResourceManager<Skeleton>::GetInstance()->TryGet(sceneName)->Get();
					const auto& boneIndexLUT = skeletonRef->boneIndexLUT;
					if (auto it = boneIndexLUT.find(boneName); it != boneIndexLUT.end())
					{
						unsigned int boneIndex = it->second;
						for (unsigned int i = 0; i < bone->mNumWeights; ++i)
						{
							auto index = bone->mWeights[i].mVertexId; //!< index of a vertex in this mesh
							auto weight = bone->mWeights[i].mWeight; //!< influence on of this bone on that vertex
							meshdata->vertices[index].AddBoneIndexWeightPair(Vec2f(boneIndex, weight));
						}
					}
					else
					{
						ENGINE_EXCEPT(L"Bone " + str2wstr(boneName) + L" does not exist in the skeleton!");
					}
				}
			}
#endif

			// Add scene object data
			{
				auto mesh = MemoryManager::Make_shared<Mesh>(aimesh->mName.C_Str());
				mesh->meshData = meshdata;
				mesh->material = material;
				sceneData.meshTree.emplace_back(level, mesh);
				// TODO mesh name is not gauranteed to be unique, need to reconsider managing mesh and so on.
				//auto name = sceneName + "_" + Str(level) + "_" + aimesh->mName.C_Str();
				//ResourceManager<Mesh>::GetInstance()->AddResource(name, "", mesh); //! Individual component should not have a path related to it
				//ResourceManager<MeshData>::GetInstance()->AddResource(name, "", meshdata); //! Individual component should not have a path related to it
				//ResourceManager<Material>::GetInstance()->AddResource(name, "", material); //! Individual component should not have a path related to it
			}
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		RecurseLoad(sceneData, sceneName, sceneDir, aiscene, node->mChildren[i], childTr, level + 1);
	}
}