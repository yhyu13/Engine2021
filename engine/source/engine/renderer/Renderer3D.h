#pragma once
#include "engine/EngineEssential.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "mesh/Mesh.h"
#include "camera/PerspectiveCamera.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/core/thread/Lock.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "engine/events/EventQueue.h"

namespace longmarch
{
	// TODO Serialize these macros in engine configuration json for more dynamic changes
#ifndef LongMarch_MAX_LIGHT
#define LongMarch_MAX_LIGHT 1024 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef MAX_SPOT_LIGHT_SHADOWS
#define MAX_SPOT_LIGHT_SHADOWS 16 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef MAX_POINT_LIGHT_SHADOWS
#define MAX_POINT_LIGHT_SHADOWS 16 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef LongMarch_MAX_NUM_DIRECTIONAL_SHADOW
#define LongMarch_MAX_NUM_DIRECTIONAL_SHADOW 1 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef LongMarch_MAX_SHADOW_PASS_BATCH
#define LongMarch_MAX_SHADOW_PASS_BATCH 1024 // Max number objects to drawn in a single batch for shadowing
#endif

#ifndef LongMarch_MAX_SCENE_PASS_BATCH
#define LongMarch_MAX_SCENE_PASS_BATCH 64 // After this number of textures are to be drawn, the batch buffer is flushed to render because this is the max number of textures that gpu shader has registerd
#endif

#ifndef LongMarch_INPLACE_SHADOW_GUASSIAN_PASS
#define LongMarch_INPLACE_SHADOW_GUASSIAN_PASS 1 // Doing guassian blur for shadow maps inplace if 1
#endif

	class Scene3DNode;
	struct Scene3DCom;
	struct Transform3DCom;
	struct Body3DCom;

	class ENGINE_API Renderer3D : public BaseAtomicClassNI
	{
	public:
		NONINSTANTIABLE(Renderer3D);
		enum class LIGHT_TYPE : int32_t // Light type order is critical
		{
			DIRECTIONAL = 0,
			POINT = 1,
			SPOT = 2,
		};
		enum class RENDER_PASS : int32_t
		{
			EMPTY = 0,
			SHADOW,
			SCENE,
			PICKING,
		};
		enum class RENDER_MODE : int32_t
		{
			CANONICAL = 0,
			MULTIDRAW,
		};

		enum class RENDER_PIPE : int32_t
		{
			CLUSTER = 0,
			DEFERRED,
			FORWARD,
		};
		/**************************************************************
		*	Render object buffer data stored on CPU side
		*
		**************************************************************/

		static const int MAX_PARTICLE_INSTANCES = 10000;
		static const int PARTICLE_INSTANCED_DATA_LENGTH = 21;
		struct ParticleInstanceData
		{
			LongMarch_Vector<Mat4> models; // model matrix for each particle in the particle-system
			LongMarch_Vector<Vec4f> textureOffsets; // texture offsets for each particle in the particle-system
			LongMarch_Vector<float> blendFactors; // blend factor for each particle in the particle-system

			float textureRows; // common for all particles of a particle-system

			ParticleInstanceData()
				: models(), textureOffsets(), blendFactors(), textureRows(1.0f)
			{}
		};

		struct CACHE_ALIGN8 RenderObj_CPU
		{
			explicit RenderObj_CPU(const EntityDecorator& e)
				:
				entity(e)
			{}
			EntityDecorator entity;
		};
		struct CACHE_ALIGN8 RenderData_CPU
		{
			explicit RenderData_CPU(const Entity& e, MeshData* mesh, Material* mat, const Mat4& Tr, const Mat4& prevTr)
				:
				entity(e),
				mesh(mesh),
				mat(mat),
				transform(Tr),
				prevTransform(prevTr)
			{}
			Entity entity;
			MeshData* mesh{ nullptr };
			Material* mat{ nullptr };
			Mat4 transform;
			Mat4 prevTransform;
		};
		/**************************************************************
		*	Light buffer data stored on CPU side
		*	This buffer is meant to be universal for all types of lights
		**************************************************************/
		struct CACHE_ALIGN32 LightBuffer_CPU
		{
			LightBuffer_CPU() = default;
			LightBuffer_CPU(const LightBuffer_CPU& other) = default;
			LightBuffer_CPU& operator=(const LightBuffer_CPU& rhs) = default;
			LightBuffer_CPU(LightBuffer_CPU&& other) = default;
			LightBuffer_CPU& operator=(LightBuffer_CPU&& rhs) = default;
			~LightBuffer_CPU() = default;

			Vec3f Pos;
			Vec3f Kd;
			Quaternion Direction;
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic,w-exponent */
			Entity thisLight;
			int32_t type; /*type: 0-directional light, 1-point light, 2-spot light*/
			bool castShadow;
		};
		/**************************************************************
		*	Light buffer data stored on GPU side
		*	Must be consistent with data format inshader
		**************************************************************/
		struct DirectionalLightBuffer_GPU
		{
			inline float* GetPtr() { return &(this->Pos_shadowMapIndex[0]); }
			Vec4f Pos_shadowMapIndex;
			Vec4f Kd_shadowMatrixIndex;  /* Let shadowMatrixIndex be -1 to indicate not casting shadow */
			Vec4f Dir;
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic, w- intensity multiplier */
			Vec4f numCSM_lambda_near_far;
			Vec4f shadowMatrixIndcies;
		};
		struct PointLightBuffer_GPU
		{
			inline float* GetPtr() { return &(this->Pos_shadowMapIndex[0]); }
			Vec4f Pos_shadowMapIndex;
			Vec4f Kd_shadowMatrixIndex;  /* Let shadowMatrixIndex be -1 to indicate not casting shadow */
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic, w- intensity multiplier */
			Vec4f Radius_CollisionRadius_SoftEdgeRatio;
		};
		struct SpotLightBuffer_GPU
		{
			inline float* GetPtr() { return &(this->Pos_shadowMapIndex[0]); }
			Vec4f Pos_shadowMapIndex;
			Vec4f Kd_shadowMatrixIndex;  /* Let shadowMatrixIndex be -1 to indicate not casting shadow */
			Vec4f Dir;
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic, w- intensity multiplier */
			Vec4f Radius_CollisionRadius_CosInnerCone_CosOutterCone;
			Vec4f SoftEdgeRatio;
		};
		struct ShadowData_GPU
		{
			inline float* GetPtr() { return &(this->PVMatrix[0][0]); }
			Mat4 PVMatrix;
			Vec4f ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti;
		};
		struct PointLightPVMatrix_GPU
		{
			inline float* GetPtr() { return &(this->PVMatrices[0][0][0]); }
			Mat4 PVMatrices[6];
		};
		/**************************************************************
		*	Model Material buffer data stored on GPU side
		*	Must be consistent with data format inshader
		**************************************************************/
		struct ModelBuffer_GPU
		{
			explicit ModelBuffer_GPU(const Mat4& _PrevModleTr, const Mat4& _ModleTr, const Mat4& _NormalModleTr)
				:
				PrevModleTr(_PrevModleTr), ModleTr(_ModleTr), NormalModleTr(_NormalModleTr)
			{}
			inline float* GetPtr() { return &(this->PrevModleTr[0][0]); }
			Mat4 PrevModleTr;
			Mat4 ModleTr;
			Mat4 NormalModleTr;
		};
		struct MaterialBuffer_GPU
		{
			explicit MaterialBuffer_GPU(Material* Mat)
			{
				Albedo_Emissive.xyz = Mat->Kd;
				Albedo_Emissive.w = Mat->emissive;
				Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex.x = Mat->metallic;
				Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex.y = Mat->roughness;
				Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex.z = Mat->textures.is_albedo_srgb();
				Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex.w = Mat->textures.has_ao();
				AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.x = Mat->textures.has_albedo();
				AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.y = Mat->textures.has_normal();
				AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.z = Mat->textures.has_metallic();
				AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.w = Mat->textures.has_roughness();
			}
			inline float* GetPtr() { return &(this->Albedo_Emissive[0]); }
			Vec4f Albedo_Emissive;
			Vec4f Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex;
			Vec4f AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex;
		};

		struct CACHE_ALIGN32 GPUBuffer
		{
			LongMarch_Vector<std::shared_ptr<ShadowBuffer>> DirectionalLightShadowBuffer;
			LongMarch_Vector<std::shared_ptr<ShadowBuffer>> PointLightShadowBuffer;
			LongMarch_Vector<std::shared_ptr<ShadowBuffer>> SpotLightShadowBuffer;

			// Gaussian kernel are samples from Gaussian distribution with mean = 0 and std = width / 2
			LongMarch_UnorderedMap<uint32_t, std::tuple<int, std::shared_ptr<ShaderStorageBuffer>, std::shared_ptr<ShaderStorageBuffer>>> GuassinKernelHalf;
			// Gaussian kernel are samples from Gaussian distribution with mean = 0 and std = width / 2 (This version should only apply to 2D bilinear filtered texture/textureArray (cubemap would not work))
			LongMarch_UnorderedMap<uint32_t, std::tuple<int, std::shared_ptr<ShaderStorageBuffer>, std::shared_ptr<ShaderStorageBuffer>>> GuassinKernelHalfBilinear;

			LongMarch_UnorderedMap<std::string, LongMarch_UnorderedMap<std::string, std::shared_ptr<SkyBoxBuffer>>> EnvCubeMaps;
			LongMarch_UnorderedMap<std::string, std::shared_ptr<Texture2D>> EnvMaps;
			inline static LongMarch_Vector<Mat4> s_default_bone_transform{ Mat4(1.0f) };

			std::shared_ptr<FrameBuffer> PrevWindowFrameBuffer;
			std::shared_ptr<FrameBuffer> CurrentWindowFrameBuffer;
			std::shared_ptr<FrameBuffer> FinalFrameBuffer;
			std::shared_ptr<FrameBuffer> PrevFrameBuffer;
			std::shared_ptr<FrameBuffer> CurrentFrameBuffer;
			std::shared_ptr<FrameBuffer> FrameBuffer_1;
			std::shared_ptr<FrameBuffer> FrameBuffer_2;
			std::shared_ptr<FrameBuffer> FrameBuffer_3;
			std::shared_ptr<FrameBuffer> FrameBuffer_4;
			std::shared_ptr<FrameBuffer> CurrentDynamicAOBuffer;
			std::shared_ptr<GBuffer> CurrentGBuffer;
			std::shared_ptr<GBuffer> CurrentThinGBuffer;
			std::shared_ptr<UniformBuffer> CurrentModelTrBuffer;
			std::shared_ptr<UniformBuffer> CurrentModelBuffer;
			std::shared_ptr<UniformBuffer> CurrentMaterialBuffer;
			std::shared_ptr<Texture2D> BrdfIntegrateLUT; // lighting
			std::shared_ptr<Texture2D> SmaaAreaLUT; // smaa
			std::shared_ptr<Texture2D> SmaaSearchLUT; // smaa

			std::shared_ptr<ShaderStorageBuffer> DirectionalLightBuffer; // lighting / shadow
			std::shared_ptr<ShaderStorageBuffer> PointLightBuffer; // lighting / shadow
			std::shared_ptr<ShaderStorageBuffer> SpotLightBuffer; // lighting / shadow
			std::shared_ptr<ShaderStorageBuffer> ShadowPVMatrixBuffer; // shadow
			std::shared_ptr<ShaderStorageBuffer> BoneTransformMatrixBuffer; // animation

			// Cluster
			std::shared_ptr<ShaderStorageBuffer> AABBvolumeGridBuffer;
			std::shared_ptr<ShaderStorageBuffer> ScreenToViewBuffer;
			std::shared_ptr<ShaderStorageBuffer> ClusterColorBuffer;
			std::shared_ptr<ShaderStorageBuffer> LightIndexListBuffer;
			std::shared_ptr<ShaderStorageBuffer> LightGridBuffer;
			std::shared_ptr<ShaderStorageBuffer> LightIndexGlobalCountBuffer;

			std::shared_ptr<VertexArray> FullScreenQuadVAO;
			std::shared_ptr<VertexArray> BBoxVAO;
			std::shared_ptr<VertexBuffer> BBoxInstBO;
			std::shared_ptr<VertexArray> FullScreenCubeVAO;

			std::shared_ptr<VertexArray> particleVAO;
			std::shared_ptr<VertexBuffer> particleInstBO;
		};

		struct CACHE_ALIGN32 MultiDrawBuffer
		{
			struct CmdBufferSimplified
			{
				uint32_t indexCount;
				uint32_t firstIndex;
				uint32_t baseVertex;
			};

			// CPU buffers
			LongMarch_Vector<Mat4> MultiDraw_ShadowModelTr; // Shadow

			LongMarch_Vector<ModelBuffer_GPU> MultiDraw_ModelBuffer; // scene
			LongMarch_Vector<MaterialBuffer_GPU> MultiDraw_MaterialBuffer; // scene
			LongMarch_Vector<int> MultiDraw_TextureId; // scene
			LongMarch_UnorderedMap_flat<Texture2D*, uint32_t> MultiDraw_UniqueTextureLUT; // scene
			LongMarch_Vector<std::pair<Material*, LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>>>> MultiDraw_MaterialTexToBind; // scene

			LongMarch_Vector<Mat4> MultiDraw_BoneTransformMatrix; // animation
			LongMarch_Vector<uint32_t> MultiDraw_BoneBaseOffset; // animation
			LongMarch_Vector<DrawIndexedIndirectCommand> MultiDraw_CmdBuffer; // Multidraw shared
			LongMarch_UnorderedMap_Par_flat<MeshData*, LongMarch_Vector<uint32_t>> MultiDraw_MeshDataToDraw; // Multidraw shared
			LongMarch_UnorderedMap_Par_flat<MeshData*, CmdBufferSimplified> MeshData_CmdBuffer_Map; // Multidraw shared

			// GPU buffers
			std::shared_ptr<ShaderStorageBuffer> MultiDraw_ssbo_ShadowModelTrsBuffer; // Shadow

			std::shared_ptr<ShaderStorageBuffer> MultiDraw_ssbo_ModelTrsBuffer; // scene
			std::shared_ptr<ShaderStorageBuffer> MultiDraw_ssbo_MaterialsBuffer; // scene
			std::shared_ptr<ShaderStorageBuffer> MultiDraw_ssbo_BoneBaseOffset; // animation

			std::shared_ptr<VertexArray> MultiDraw_VAO; // Multidraw shared
			std::shared_ptr<VertexBuffer> MultiDraw_VBO; // Multidraw shared
			std::shared_ptr<VertexBuffer> MultiDraw_InstBO; // Multidraw shared
			std::shared_ptr<IndexBuffer> MultiDraw_IBO; // Multidraw shared
			std::shared_ptr<IndexedIndirectCommandBuffer> MultiDraw_CBO; // Multidraw shared
			uint32_t MultiDraw_MeshDataToDrawIndexCounter; // Multidraw shared

			struct MultiDrawRenderPassCallback
			{
				std::function<void()>* submitCallback;  // Multidraw shared
				std::function<void()>* clearCallback;  // Multidraw shared
			} multiDrawRenderPassCallback;
		};
		struct CACHE_ALIGN32 CPUBuffer
		{
			friend Renderer3D;
		public:
			LongMarch_Vector<LightBuffer_CPU> LIGHTS_BUFFERED;
			LongMarch_Vector<RenderObj_CPU> RENDERABLE_OBJ_BUFFERED;
		private:
			// Lighting data
			LongMarch_Vector<DirectionalLightBuffer_GPU> DIRECTIONAL_LIGHT_PROCESSED;
			LongMarch_Vector<PointLightBuffer_GPU> POINT_LIGHT_PROCESSED;
			LongMarch_Vector<SpotLightBuffer_GPU> SPOT_LIGHT_PROCESSED;
			LongMarch_Vector<ShadowData_GPU> SHADOW_DATA_PROCESSED;
			// BBox data
			LongMarch_Vector<Mat4> InstancedDraw_BVModelTr;

			// TODO add particle related properties
		};
		/**************************************************************
		*	Render3D Status class
		*
		**************************************************************/
		struct CACHE_ALIGN32 Renderer3DStorage
		{
			// CPU buffers
			CPUBuffer cpuBuffer;
			// Multi draw data
			MultiDrawBuffer multiDrawBuffer;
			// GPU buffer
			GPUBuffer gpuBuffer;
			// Shaders
			LongMarch_UnorderedMap_node<std::string, std::shared_ptr<Shader>> ShaderMap;
			std::shared_ptr<Shader> CurrentShader;

			Vec4f cube_directions[6];
			Vec3f cube_ups[6];

			uint32_t NUM_LIGHT;
			uint32_t NUM_DIRECTIONAL_LIGHT;
			uint32_t NUM_POINT_LIGHT;
			uint32_t NUM_SPOT_LIGHT;
			uint32_t NUM_SHADOW;
			uint32_t MAX_LIGHT;

			uint32_t MAX_DIRECTIONAL_SHADOW;
			uint32_t MAX_POINT_SHADOW;
			uint32_t MAX_SPOT_SHADOW;

			uint32_t MAX_SHADOW_BATCH;
			uint32_t MAX_SCENE_BATCH;

			int directional_light_display_mode;

			RENDER_PASS RENDER_PASS;
			RENDER_MODE RENDER_MODE;

			std::string CurrentEnvMapName;

			const uint32_t fragTexture_0_slot = { 0u };
			const uint32_t fragTexture_1_slot = { 1u };
			const uint32_t fragTexture_2_slot = { 2u };
			//! Before this slot are reserved for uniform texture bindings, you can bind custom textures (including gbuffer textures) after this slot
			const uint32_t fragTexture_empty_slot = { 3u };

			Vec2u resolution;
			float resolution_ratio;
			uint32_t resolution_shadowMap;

			int gBuffer_display_mode;

			int toneMapping_mode;
			float value_gamma;

			struct
			{
				bool enable;
				uint32_t aoSamples;
				uint32_t aoGaussianKernal;
				uint32_t aoSampleResolutionDownScale;
				float aoSampleRadius;
				float aoScale;
				float aoPower;
				bool enable_indirect_light_bounce;
			}AOSettings;

			bool enable_deferredShading;
			bool enable_reverse_z;
			bool enable_shadow;

			bool enable_debug_cluster_light;

			bool enable_motionblur;
			float motionblur_shutterPeriod;

			bool window_size_changed_this_frame;
			bool enable_fxaa;
			bool enable_taa;
			bool enable_smaa;

			bool enable_drawingBoundingVolume;
			bool enable_wireframe;

			RENDER_PIPE RENDER_PIPE;

			struct ClusterStorage {
				struct VolumeTileAABB {
					Vec4f minPoint;
					Vec4f maxPoint;
				} frustrum;

				struct ScreenToView {
					Mat4 inverseProjectionMat;
					unsigned int tileSizes[4];
					unsigned int screenWidth;
					unsigned int screenHeight;
				}screenToView;

				// 16x9x24 subdivision
				const unsigned int gridSizeX = 16;
				const unsigned int gridSizeY = 9;
				const unsigned int gridSizeZ = 24;
				const unsigned int numClusters = gridSizeX * gridSizeY * gridSizeZ;
				unsigned int maxLightsPerCluster = 50;
				LongMarch_Vector<Vec4f> clusterColors;
			}ClusterData;
		};

		static void Init();
		static void Shutdown() {};
		inline static void OnWindowResize(uint32_t width, uint32_t height) { RenderCommand::SetViewport(0, 0, width, height); }

		/**************************************************************
		*	Render3D highlevel API
		*
		**************************************************************/
		static void BeginRendering();
		static void BeginShadowing(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const Vec4f& clear_color = Vec4f(0., 0., 0., 1)
		);
		static void EndShadowing();
		static void BeginScene(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const Vec4f& clear_color = Vec4f(0., 0., 0., 1)
		);
		static void EndScene();
		static void BeginLighting(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const Vec4f& clear_color = Vec4f(0., 0., 0., 1)
		);
		static void EndLighting();
		static void BeginPostProcessing();
		static void EndPostProcessing();
		static void EndRendering();

		static void BeginRenderingParticles(
			PerspectiveCamera* camera,
			const std::function<void(PerspectiveCamera*)>& f_render,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const Vec4f& clear_color = Vec4f(0., 0., 0., 1));
		static void EndRenderingParticles();

		/**************************************************************
		*	Render3D lowlevel API
		*
		**************************************************************/
		//! For custom render pass
		static void Draw(const RenderData_CPU& data);
		//! For lighting, shadow pass
		static void Draw(Entity entity, MeshData* Mesh, Material* Mat, const Mat4& transform, const Mat4& PrevTransform, const std::string& shaderName);
		static void Draw(Entity entity, Scene3DNode* sceneNode, const Mat4& transform, const Mat4& PrevTransform, const std::string& shaderName);
		//! For canonical drawing
		static void DrawMesh(const std::shared_ptr<VertexArray>& MeshVertexArray);

		/**************************************************************
		*	Render3D debug rendering API
		*
		**************************************************************/
		static void RenderBoundingBox(const Mat4& transform);
		/**************************************************************
		*	Render3D Multidraw API
		*
		**************************************************************/
		static void BuildAllMesh();
		static void BuildAllMaterial();
		static void BuildAllTexture();

		static void UpdateMeshToMultiDraw(const LongMarch_Vector<MeshData*>& Meshs);
		static void AppendMeshToMultiDraw(MeshData* Mesh);
		/**************************************************************
		*	Render3D inline API
		*
		**************************************************************/
		inline static RENDER_MODE GetRenderMode() { return s_Data.RENDER_MODE; }
		inline static void ToggleWireFrameMode() { s_Data.enable_wireframe = !s_Data.enable_wireframe; }
		inline static void ToggleDrawBoundingVolumeMode() { s_Data.enable_drawingBoundingVolume = !s_Data.enable_drawingBoundingVolume; }
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		inline static void ToggleReverseZ(bool enable) { s_Data.enable_reverse_z = enable; RenderCommand::Reverse_Z(enable); }
		inline static std::shared_ptr<FrameBuffer> GetFinalFrameBuffer() { return s_Data.gpuBuffer.FinalFrameBuffer; }

		template<typename Func>
		static void RenderFullScreenQuad(Func& renderCommend)
		{
			renderCommend();
			_RenderFullScreenQuad();
		}
		template<typename Func>
		static void RenderFullScreenCube(Func& renderCommend)
		{
			renderCommend();
			_RenderFullScreenCube();
		}

		static void RenderParticles(const LongMarch_Vector<std::pair<int, ParticleInstanceData>> particleData, const PerspectiveCamera* camera);

		inline static void CommitBatchRendering()
		{
			switch (s_Data.RENDER_MODE)
			{
			case RENDER_MODE::MULTIDRAW:
			{
				_RenderBatch();
			}
			}
		}
	private:
		static void _BeginClusterBuildGrid(const PerspectiveCamera* camera);
		static void _BeginLightCullingPass(const PerspectiveCamera* camera);
		static void _BeginDebugCluster(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static glm::vec3 HSVtoRGB(float H, float S, float V);

		static void _BeginForwardGeomtryPass(const PerspectiveCamera* camera, const Vec4f& clear_color, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginDeferredGeomtryPass(const PerspectiveCamera* camera, const Vec4f& clear_color, const std::shared_ptr<GBuffer>& gBuffer_out);
		static void _PopulateShadingPassUniformsVariables(const PerspectiveCamera* camera);

		static void _BeginSkyBoxPass(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginDynamicAOPass(const std::shared_ptr<GBuffer>& gbuffer_in);

		static void _BeginDeferredLightingPass(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const Vec4f& clear_color = Vec4f(0., 0., 0., 1),
			const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr
		);
		static void _BeginForwardLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginClusterLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _RenderBoundingBox(const std::shared_ptr<FrameBuffer>& framebuffer_out);

		static void _BeginToneMappingPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginTAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginFXAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginSMAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_edge, const std::shared_ptr<FrameBuffer>& framebuffer_blend, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);
		static void _BeginMotionBlurPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out = nullptr);

		static void _RenderFullScreenQuad();
		static void _RenderFullScreenCube();

		static void _RenderBatch();
		static bool _BeginRenderBatch();
		static void _EndRenderBatch();
		static void _RestBatchTextureList();

		static void _RenderParticles(float* data, const int& instanceCount, const std::shared_ptr<FrameBuffer>& framebuffer_out);

	private:
		static void _ON_TOGGLE_SLICES(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_TOGGLE_SHADOW(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_SWITCH_GBUFFER_MODE(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_TOGGLE_MOTION_BLUR(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_TAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_FXAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_SMAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SWITCH_TONEMAP_MODE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_GAMMA_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_AO_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
	public:
		inline static Renderer3DStorage s_Data;
	};
}