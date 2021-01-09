#pragma once

#include "engine/core/EngineCore.h"
#include "engine/math/Geommath.h"
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace longmarch
{
	class AssimpHelper
	{
	public:
		//! http://assimp.sourceforge.net/lib_html/data.html
		inline static Mat4 Assimp2Glm(const aiMatrix4x4& from)
		{
			return Mat4(
				(double)from.a1, (double)from.b1, (double)from.c1, (double)from.d1,
				(double)from.a2, (double)from.b2, (double)from.c2, (double)from.d2,
				(double)from.a3, (double)from.b3, (double)from.c3, (double)from.d3,
				(double)from.a4, (double)from.b4, (double)from.c4, (double)from.d4
			);
		}
		//! http://assimp.sourceforge.net/lib_html/data.html
		inline static aiMatrix4x4 Glm2Assimp(const Mat4& from)
		{
			return aiMatrix4x4(from[0][0], from[1][0], from[2][0], from[3][0],
				from[0][1], from[1][1], from[2][1], from[3][1],
				from[0][2], from[1][2], from[2][2], from[3][2],
				from[0][3], from[1][3], from[2][3], from[3][3]
			);
		}

		// Prints a mesh's info; A mesh contains vertices, faces, normals and
		// more as needed for graphics.  Vertices are tied to bones with
		// weights.
		// Partial Credit : Prof. Gary Herron @ CS541,CS460/560 DigiPen Institute of Technology 2020/2021
		inline static void ShowMesh(aiMesh* mesh)
		{
			constexpr unsigned int MAX = 3;
			// Mesh name and some counts
			PRINT(Str("Mesh %s: %d vertices, %d faces,  %d bones", mesh->mName.C_Str(),
				mesh->mNumVertices, mesh->mNumFaces, mesh->mNumBones));

			// Mesh's bones and weights of all connected vertices.
			for (unsigned int i = 0; i < mesh->mNumBones && i < MAX; ++i) {
				aiBone* bone = mesh->mBones[i];
				PRINT(Str("  %s:  %d weights;  OffsetMatrix:[%f, ...]", bone->mName.C_Str(), bone->mNumWeights, bone->mOffsetMatrix[0][0]));
				for (unsigned int i = 0; i < bone->mNumWeights && i < MAX; ++i) {
					PRINT(Str("    %d %f", bone->mWeights[i].mVertexId, bone->mWeights[i].mWeight));
				}
				if (bone->mNumWeights > MAX) PRINT("    ...");
			}
			if (mesh->mNumBones > MAX) PRINT("  ...");
		}

		// Prints an animation.  An animation contains a few timing parameters
		// and then channels for a number of animated bones.  Each channel
		// contains a V, Q, and S keyframe sequences.
		// Partial Credit : Prof. Gary Herron @ CS541,CS460/560 DigiPen Institute of Technology 2020/2021
		inline static void ShowAnimation(aiAnimation* anim)
		{
			constexpr unsigned int MAX = 3;
			PRINT(Str("Animation: %s  duration (in ticks): %f  tickspersecond: %f  numchannels: %d",
				anim->mName.C_Str(),
				anim->mDuration,
				anim->mTicksPerSecond,
				anim->mNumChannels));

			// The animations channels
			for (unsigned int i = 0; i < anim->mNumChannels && i < MAX; ++i) {
				aiNodeAnim* chan = anim->mChannels[i];

				// Prints the bone name followed by the numbers of each key type
				PRINT("");
				PRINT(Str("    %-15s VQS keys:  %d %d %d",
					chan->mNodeName.C_Str(),
					chan->mNumPositionKeys,
					chan->mNumRotationKeys,
					chan->mNumScalingKeys));

				// The position (V) keys
				PRINT("");
				for (unsigned int i = 0; i < chan->mNumPositionKeys && i < MAX; ++i) {
					aiVectorKey key = chan->mPositionKeys[i];
					PRINT(Str("      V[%d]: %f : (%f %f %f)", i, key.mTime, key.mValue[0], key.mValue[1], key.mValue[2]));
				}
				if (chan->mNumPositionKeys > MAX) PRINT("      ...");

				// The rotation (Q) keys
				PRINT("");
				for (unsigned int i = 0; i < chan->mNumRotationKeys && i < MAX; ++i) {
					aiQuatKey key = chan->mRotationKeys[i];
					PRINT(Str("      Q[%d]: %f : (%f %f %f %f)", i, key.mTime, key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
				}
				if (chan->mNumRotationKeys > MAX) PRINT("      ...");

				// the scaling (S) keys
				PRINT("");
				for (unsigned int i = 0; i < chan->mNumScalingKeys && i < MAX; ++i) {
					aiVectorKey key = chan->mScalingKeys[i];
					PRINT(Str("      S[%d]: %f : (%f %f %f)", i, key.mTime, key.mValue[0], key.mValue[1], key.mValue[2]));
				}
				if (chan->mNumScalingKeys > MAX) PRINT("      ...");
			}

			if (anim->mNumChannels > MAX) PRINT("    ...");
		}

		// Prints the bone hierarchy and relevant info with a graphical
		// representation of the hierarchy.
		// Partial Credit : Prof. Gary Herron @ CS541,CS460/560 DigiPen Institute of Technology 2020/2021
		inline static void ShowBoneHierarchy(const aiScene* scene, const aiNode* node, const int level = 0)
		{
			// Print indentation to show this node's level in the hierarchy
			// Print node name and transformation to parent's node
			PRINT(std::string(level, '-') + ">" + Str("%s   Transformation:[%f, ...]", node->mName.C_Str(), node->mTransformation[0][0]));
			// Recurse onto this node's children
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
			{
				ShowBoneHierarchy(scene, node->mChildren[i], level + 1);
			}
		}
	};
}