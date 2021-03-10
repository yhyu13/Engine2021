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
#define MAX_SPOT_LIGHT_SHADOWS 8 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef MAX_POINT_LIGHT_SHADOWS
#define MAX_POINT_LIGHT_SHADOWS 8 // Must match "./asset/shader/include/LighStruch.h"
#endif

#ifndef LongMarch_MAX_NUM_DIRECTIONAL_SHADOW
#define LongMarch_MAX_NUM_DIRECTIONAL_SHADOW 2 // Must match "./asset/shader/include/LighStruch.h"
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
			CUSTOM,
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

		constexpr static int MAX_PARTICLE_INSTANCES = 1024;
		constexpr static int PARTICLE_INSTANCED_DATA_LENGTH = 21;
		struct ParticleInstanceData_CPU
		{
			LongMarch_Vector<Mat4> models; // model matrix for each particle in the particle-system
			LongMarch_Vector<Vec4f> textureOffsets; // texture offsets for each particle in the particle-system
			LongMarch_Vector<float> blendFactors; // blend factor for each particle in the particle-system
			Entity entity;
			float textureRows; // common for all particles of a particle-system
		};
		using ParticleInstanceDrawData = LongMarch_Vector<std::pair<std::shared_ptr<Texture2D>, Renderer3D::ParticleInstanceData_CPU>>;

		struct RenderObj_CPU
		{
			explicit RenderObj_CPU(const EntityDecorator& e)
				:
				entity(e)
			{}
			EntityDecorator entity;
		};
		struct RenderData_CPU
		{
			explicit RenderData_CPU(const Entity& e, std::shared_ptr<MeshData> mesh, std::shared_ptr<Material> mat, const Mat4& Tr, const Mat4& prevTr)
				:
				entity(e),
				mesh(mesh),
				mat(mat),
				transform(Tr),
				prevTransform(prevTr)
			{}
			Entity entity;
			std::shared_ptr<MeshData> mesh{ nullptr };
			std::shared_ptr<Material> mat{ nullptr };
			Mat4 transform;
			Mat4 prevTransform;
		};
		/**************************************************************
		*	Light buffer data stored on CPU side
		*	This buffer is meant to be universal for all types of lights
		**************************************************************/
		struct LightBuffer_CPU
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
			Vec4f Dir; /* Light direction */
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic, w- intensity multiplier */
			Vec4f numCSM_lambda_near_far;
			Vec4f shadowMatrixIndcies; /* Since there are at most 4 levels of CSM for directional light, we can use a vec4 to store all 4 indices */
		};
		struct PointLightBuffer_GPU
		{
			inline float* GetPtr() { return &(this->Pos_shadowMapIndex[0]); }
			Vec4f Pos_shadowMapIndex;
			Vec4f Kd_shadowMatrixIndex;  /* Let shadowMatrixIndex be -1 to indicate not casting shadow */
			Vec4f Attenuation; /* x-const,y-linear,z-quadratic, w- intensity multiplier */
			Vec4f Radius_CollisionRadius_SoftEdgeRatio;
			Vec4f shadowMatrixIndcies_1_2_3; /* Since there are at most 6 shadow matrices for point light, we can use a vec4 to store the first 3 */
			Vec4f shadowMatrixIndcies_4_5_6; /* Since there are at most 6 shadow matrices for point light, we can use a vec4 to store store the last 3 */
		};
		struct SpotLightBuffer_GPU
		{
			inline float* GetPtr() { return &(this->Pos_shadowMapIndex[0]); }
			Vec4f Pos_shadowMapIndex;
			Vec4f Kd_shadowMatrixIndex;  /* Let shadowMatrixIndex be -1 to indicate not casting shadow */
			Vec4f Dir; /* Light direction */
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
			explicit MaterialBuffer_GPU(const std::shared_ptr<Material>& Mat)
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

		struct GPUBuffer
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

			std::shared_ptr<FrameBuffer> PrevFinalFrameBuffer; // Previous frame's final frame buffer
			std::shared_ptr<FrameBuffer> CurrentFinalFrameBuffer; // Frame buffer just before tone mapping
			std::shared_ptr<FrameBuffer> PrevOpaqueLightingFrameBuffer; // Stores opaque lighting color
			std::shared_ptr<FrameBuffer> CurrentFrameBuffer;
			std::shared_ptr<FrameBuffer> FrameBuffer_1;
			std::shared_ptr<FrameBuffer> FrameBuffer_2;
			std::shared_ptr<FrameBuffer> FrameBuffer_3;
			std::shared_ptr<FrameBuffer> FrameBuffer_4;
			std::shared_ptr<FrameBuffer> CurrentDynamicAOBuffer;
			std::shared_ptr<FrameBuffer> CurrentDynamicSSRBuffer;
			std::shared_ptr<FrameBuffer> CurrentDynamicBloomBuffer;
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

		struct MultiDrawBuffer
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
			LongMarch_UnorderedMap_flat<std::shared_ptr<Texture2D>, uint32_t> MultiDraw_UniqueTextureLUT; // scene
			LongMarch_Vector<std::pair<std::shared_ptr<Material>, LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>>>> MultiDraw_MaterialTexToBind; // scene

			LongMarch_Vector<Mat4> MultiDraw_BoneTransformMatrix; // animation
			LongMarch_Vector<uint32_t> MultiDraw_BoneBaseOffset; // animation
			LongMarch_Vector<DrawIndexedIndirectCommand> MultiDraw_CmdBuffer; // Multidraw shared
			LongMarch_UnorderedMap_Par_flat<std::shared_ptr<MeshData>, LongMarch_Vector<uint32_t>> MultiDraw_MeshDataToDraw; // Multidraw shared
			LongMarch_UnorderedMap_Par_flat<std::shared_ptr<MeshData>, CmdBufferSimplified> MeshData_CmdBuffer_Map; // Multidraw shared

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
		struct CPUBuffer
		{
			friend Renderer3D;
		public:
			LongMarch_Vector<LightBuffer_CPU> LIGHTS_BUFFERED;
			LongMarch_Vector<RenderObj_CPU> RENDERABLE_OBJ_OPAQUE;
			LongMarch_Vector<RenderObj_CPU> RENDERABLE_OBJ_TRANSPARENT;
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
		struct Renderer3DStorage
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
			LongMarch_Vector<std::string> ListRenderShadersToPopulateData;
			LongMarch_Vector<std::string> ListShadersToPopulateData;

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
			const uint32_t fragTexture_3_slot = { 3u };
			const uint32_t fragTexture_empty_slot = { 4u }; //<! Slots before this slot are reserved for uniform texture bindings, you can bind custom textures (including gbuffer textures) after this slot

			Vec2u window_size;
			Vec2u resolution;
			float resolution_ratio; //!< ratio of resolution vs. window_size

			int gBuffer_display_mode;

			int toneMapping_mode;
			float value_gamma;

			struct
			{
				bool enable;
				uint32_t ao_samples;
				uint32_t ao_gaussian_kernal;
				uint32_t ao_sample_resolution_downScale;
				float ao_sample_radius;
				float ao_scale;
				float ao_power;
				bool enable_indirect_light_bounce;
				float indirect_light_bounce_scale;
			} AOSettings;

			struct
			{
				bool enable;
				uint32_t ssr_gaussian_kernal;
				uint32_t ssr_sample_resolution_downScale;
			} SSRSettings;

			struct
			{
				bool enable;
				uint32_t bloom_gaussian_kernal;
				uint32_t bloom_sample_resolution_downScale;
				float bloom_threshold;
				float bloom_blend_strength;
			} BloomSettings;

			bool enable_deferredShading;
			bool enable_reverse_z;

			bool enable_env_mapping;
			bool enable_shadow;

			bool enable_debug_cluster_light;

			bool enable_motionblur;
			int motionblur_shutterSpeed;

			bool enable_fxaa;
			bool enable_taa;
			bool enable_smaa;

			bool enable_drawingBoundingVolume;
			bool enable_wireframe;

			bool window_size_changed_this_frame;

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

				unsigned int gridSizeX;
				unsigned int gridSizeY;
				unsigned int gridSizeZ;
				unsigned int numClusters;// = gridSizeX * gridSizeY * gridSizeZ;
				unsigned int maxLightsPerCluster = 50;
				LongMarch_Vector<Vec4f> clusterColors;
			} ClusterData;
		};

		static void Init();
		static void Shutdown() {};

		/**************************************************************
		*	Render3D highlevel API
		*
		**************************************************************/
		static bool ShouldRendering();
		static void BeginRendering(const PerspectiveCamera* camera);

		static void BeginShadowing(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render_opaque,
			const std::function<void()>& f_render_trasparent,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName
		);
		static void EndShadowing();

		static void BeginOpaqueScene(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName
		);
		static void EndOpaqueScene();

		static void BeginOpaqueLighting(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName
		);
		static void EndOpaqueLighting();

		static void BeginTransparentSceneAndLighting(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName
		);
		static void EndTransparentSceneAndLighting();

		static void BeginPostProcessing();
		static void EndPostProcessing();
		
		static void EndRendering();

		/**************************************************************
		*	Render3D lowlevel API
		*
		**************************************************************/
		//! For custom render pass
		static void Draw(const RenderData_CPU& data);
		//! For lighting, shadow pass
		static void Draw(Entity entity, const std::shared_ptr<MeshData>& Mesh, const std::shared_ptr<Material>& Mat, const Mat4& transform, const Mat4& PrevTransform, const std::string& shaderName);
		static void Draw(Entity entity, const std::shared_ptr<Scene3DNode>& sceneNode, const Mat4& transform, const Mat4& PrevTransform, const std::string& shaderName);

		//! For canonical drawing
		static void DrawMesh(const std::shared_ptr<VertexArray>& MeshVertexArray);

		static void DrawParticles(const ParticleInstanceDrawData& particleData);
		static void DrawParticlesInstance(float* data, const int instanceCount);

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

		static void UpdateMeshToMultiDraw(const LongMarch_Vector<std::shared_ptr<MeshData>>& Meshs);
		static void AppendMeshToMultiDraw(std::shared_ptr<MeshData> Mesh);
		/**************************************************************
		*	Render3D inline API
		*
		**************************************************************/
		static RendererAPI::API GetAPI();
		static void ToggleReverseZ(bool enable);

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
		static Vec3f _HSVtoRGB(float H, float S, float V);

		static void _BeginForwardGeomtryPass(const PerspectiveCamera* camera, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginDeferredGeomtryPass(const PerspectiveCamera* camera, const std::shared_ptr<GBuffer>& gBuffer_out);
		static void _PopulateShadingPassUniformsVariables(const PerspectiveCamera* camera); 
		static void _PopulateShadowPassVariables();

		static void _BindSkyBoxTexture();
		static void _BeginSkyBoxPass(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginDynamicAOPass(const std::shared_ptr<GBuffer>& gbuffer_in);
		static void _BeginDynamicSSRPass(const std::shared_ptr<GBuffer>& gbuffer_in);

		static void _BeginDeferredLightingPass(
			const PerspectiveCamera* camera,
			const std::function<void()>& f_render,
			const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
			const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
			const std::function<void(const std::string&)>& f_setRenderShaderName,
			const std::shared_ptr<FrameBuffer>& framebuffer_out
		);
		static void _BeginForwardLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginClusterLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _RenderBoundingBox(const std::shared_ptr<FrameBuffer>& framebuffer_out);

		static void _BeginSSRPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginSSAOPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);

		static void _BeginTAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginFXAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginSMAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_edge, const std::shared_ptr<FrameBuffer>& framebuffer_blend, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginMotionBlurPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginBloomPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_bright, const std::shared_ptr<FrameBuffer>& framebuffer_blend, const std::shared_ptr<FrameBuffer>& framebuffer_out);
		static void _BeginToneMappingPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out);

		static void _RenderFullScreenQuad();
		static void _RenderFullScreenCube();

		static void _RenderBatch();
		static bool _BeginRenderBatch();
		static void _EndRenderBatch();
		static void _RestBatchTextureList();

	private:
		static void _ON_TOGGLE_DEBUG_CLUSTER(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_TOGGLE_ENV_MAPPING(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_TOGGLE_SHADOW(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_SWITCH_GBUFFER_MODE(EventQueue<EngineGraphicsDebugEventType>::EventPtr e);
		static void _ON_TOGGLE_MOTION_BLUR(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_TAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_FXAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_TOGGLE_SMAA(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SWITCH_TONEMAP_MODE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_GAMMA_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_AO_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_SSR_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
		static void _ON_SET_BLOOM_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e);
	public:
		inline static Renderer3DStorage s_Data;
	};
}