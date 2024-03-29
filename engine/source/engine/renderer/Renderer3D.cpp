#include "engine-precompiled-header.h"
#include "Renderer3D.h"
#include "engine/Engine.h"
#include "engine/math/SphericalHarmonics.h"
#include "engine/math/DistributionMath.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"
#include "engine/ecs/components/LightCom.h"
#include "engine/ecs/components/3d/Scene3DCom.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/scene-graph/Scene3DNode.h"
#include "engine/physics/AABB.h"
#include "engine/ecs/GameWorld.h"
#include "engine/renderer/platform/OpenGL/OpenGLUtil.h"

void longmarch::Renderer3D::Init()
{
    if (s_init)
    {
        return;
    }
    s_init = true;
    RenderCommand::Init();

    const auto& engineConfiguration = FileSystem::GetCachedJsonCPP("$root:engine-config.json");
    const auto& graphicsConfiguration = engineConfiguration["graphics"];
    const auto& shaderConfiguration = engineConfiguration["shader"];
    /**************************************************************
    *	Init Renderer3DStorage
    *
    **************************************************************/
    {
        s_Data.RENDER_MODE = (graphicsConfiguration["Multi-draw"].asBool())
                                 ? RENDER_MODE::MULTIDRAW
                                 : RENDER_MODE::CANONICAL;
        s_Data.MAX_LIGHT = LongMarch_MAX_LIGHT;
        s_Data.NUM_LIGHT = 0;

        s_Data.MAX_SPOT_SHADOW = MAX_SPOT_LIGHT_SHADOWS;
        s_Data.MAX_POINT_SHADOW = MAX_POINT_LIGHT_SHADOWS;
        s_Data.MAX_DIRECTIONAL_SHADOW = LongMarch_MAX_NUM_DIRECTIONAL_SHADOW;

        s_Data.MAX_SHADOW_BATCH = LongMarch_MAX_SHADOW_PASS_BATCH;
        s_Data.MAX_SCENE_BATCH = LongMarch_MAX_SCENE_PASS_BATCH;

        s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.reserve(TWO_12);
        s_Data.cpuBuffer.LIGHTS_BUFFERED.reserve(s_Data.MAX_LIGHT);
        s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED.reserve(1);
        s_Data.cpuBuffer.POINT_LIGHT_PROCESSED.reserve(128);
        s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED.reserve(128);
        s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.reserve(128);

        s_Data.gpuBuffer.DirectionalLightShadowBuffer.reserve(LongMarch_MAX_NUM_DIRECTIONAL_SHADOW);
        s_Data.gpuBuffer.PointLightShadowBuffer.reserve(MAX_POINT_LIGHT_SHADOWS);
        s_Data.gpuBuffer.SpotLightShadowBuffer.reserve(MAX_SPOT_LIGHT_SHADOWS);

        {
            s_Data.cube_directions[0] = Vec4f(1, 0, 0, 0);
            s_Data.cube_directions[1] = Vec4f(-1, 0, 0, 0);
            s_Data.cube_directions[2] = Vec4f(0, 1, 0, 0);
            s_Data.cube_directions[3] = Vec4f(0, -1, 0, 0);
            s_Data.cube_directions[4] = Vec4f(0, 0, 1, 0);
            s_Data.cube_directions[5] = Vec4f(0, 0, -1, 0);
        }
        {
            s_Data.cube_ups[0] = Vec3f(0, -1, 0);
            s_Data.cube_ups[1] = Vec3f(0, -1, 0);
            s_Data.cube_ups[2] = Vec3f(0, 0, 1);
            s_Data.cube_ups[3] = Vec3f(0, 0, -1);
            s_Data.cube_ups[4] = Vec3f(0, -1, 0);
            s_Data.cube_ups[5] = Vec3f(0, -1, 0);
        }

        s_Data.multiDrawBuffer.MultiDraw_UniqueTextureLUT.reserve(s_Data.MAX_SCENE_BATCH);
        s_Data.multiDrawBuffer.MultiDraw_TextureId.reserve(s_Data.MAX_SCENE_BATCH);
        _RestBatchTextureList();
        s_Data.multiDrawBuffer.MultiDraw_ShadowModelTr.reserve(s_Data.MAX_SHADOW_BATCH);
        s_Data.multiDrawBuffer.MultiDraw_ModelBuffer.reserve(TWO_11);
        s_Data.multiDrawBuffer.MultiDraw_MaterialBuffer.reserve(TWO_11);
        s_Data.multiDrawBuffer.MultiDraw_MaterialTexToBind.reserve(s_Data.MAX_SCENE_BATCH);
        s_Data.multiDrawBuffer.MultiDraw_BoneTransformMatrix.reserve(TWO_11);
        s_Data.multiDrawBuffer.MultiDraw_BoneBaseOffset.reserve(TWO_11);

        s_Data.multiDrawBuffer.MultiDraw_CmdBuffer.reserve(TWO_11);
        s_Data.multiDrawBuffer.MultiDraw_MeshDataToDraw.reserve(TWO_11);

        s_Data.enable_fxaa = graphicsConfiguration["FXAA"].asBool();
        s_Data.enable_taa = graphicsConfiguration["TAA"].asBool();

        s_Data.enable_deferredShading = graphicsConfiguration["Deferred-shading"].asBool();
        if (s_Data.enable_deferredShading)
        {
            s_Data.RENDER_PIPE = RENDER_PIPE::DEFERRED;
        }
        else
        {
            s_Data.RENDER_PIPE = RENDER_PIPE::FORWARD;
        }

        s_Data.resolution_ratio = graphicsConfiguration["Resolution-ratio"].asFloat();
        s_Data.enable_shadow = graphicsConfiguration["Shadow"].asBool();
        s_Data.gBuffer_display_mode = 0;
        s_Data.toneMapping_mode = 0; // Choose the first tonemap method
        s_Data.value_gamma = 2.2f;
        s_Data.ambient = 0.1f;

        {
            const auto& motion_blur_setttings = graphicsConfiguration["Motion-blur"];
            s_Data.MotionBlur.enable_motionblur = motion_blur_setttings["Enable"].asBool();
            s_Data.MotionBlur.motionblur_shutterSpeed = motion_blur_setttings["Motion-blur-shutter-speed"].asInt();
        }

        {
            const auto& smaa_settings = graphicsConfiguration["SMAA"];
            s_Data.SMAASettings.enable = smaa_settings["Enable"].asBool();
            s_Data.SMAASettings.mode = smaa_settings["Mode"].asInt();
        }
        {
            const auto& gi_settings = graphicsConfiguration["SSGI"];
            s_Data.SSGISettings.enable = gi_settings["Enable"].asBool();
            s_Data.SSGISettings.gi_gaussian_kernal = gi_settings["Gaussian-kernel"].asUInt();
            s_Data.SSGISettings.gi_gaussian_kernal = (glm::clamp)(s_Data.SSGISettings.gi_gaussian_kernal,
                                                                  LongMarch_GUASSIAN_KERNEL_MIN,
                                                                  LongMarch_GUASSIAN_KERNEL_MAX);
            s_Data.SSGISettings.gi_samples = gi_settings["Num-samples"].asUInt();
            s_Data.SSGISettings.gi_sample_resolution_downScale = gi_settings["Res-down-scale"].asUInt();
            s_Data.SSGISettings.gi_sample_radius = gi_settings["Radius"].asFloat();
            s_Data.SSGISettings.gi_strength = gi_settings["Strength"].asFloat();
        }
        {
            const auto& ao_settings = graphicsConfiguration["SSAO"];
            s_Data.SSAOSettings.enable = ao_settings["Enable"].asBool();
            s_Data.SSAOSettings.ao_gaussian_kernal = ao_settings["Gaussian-kernel"].asUInt();
            s_Data.SSAOSettings.ao_gaussian_kernal = (glm::clamp)(s_Data.SSAOSettings.ao_gaussian_kernal,
                                                                  LongMarch_GUASSIAN_KERNEL_MIN,
                                                                  LongMarch_GUASSIAN_KERNEL_MAX);
            s_Data.SSAOSettings.ao_samples = ao_settings["Num-samples"].asUInt();
            s_Data.SSAOSettings.ao_sample_resolution_downScale = ao_settings["Res-down-scale"].asUInt();
            s_Data.SSAOSettings.ao_sample_radius = ao_settings["Radius"].asFloat();
            s_Data.SSAOSettings.ao_scale = ao_settings["Scale"].asFloat();
            s_Data.SSAOSettings.ao_power = ao_settings["Power"].asFloat();
        }
        {
            const auto& ssr_settings = graphicsConfiguration["SSR"];
            s_Data.SSRSettings.enable = ssr_settings["Enable"].asBool();
            s_Data.SSRSettings.ssr_gaussian_kernal = ssr_settings["Gaussian-kernel"].asUInt();
            s_Data.SSRSettings.ssr_gaussian_kernal = (glm::clamp)(s_Data.SSRSettings.ssr_gaussian_kernal,
                                                                  LongMarch_GUASSIAN_KERNEL_MIN,
                                                                  LongMarch_GUASSIAN_KERNEL_MAX);
            s_Data.SSRSettings.ssr_sample_resolution_downScale = ssr_settings["Res-down-scale"].asUInt();
        }
        {
            const auto& bloom_settings = graphicsConfiguration["Bloom"];
            s_Data.BloomSettings.enable = bloom_settings["Enable"].asBool();
            s_Data.BloomSettings.bloom_threshold = bloom_settings["Threshold"].asFloat();
            s_Data.BloomSettings.bloom_blend_strength = bloom_settings["Strength"].asFloat();
            s_Data.BloomSettings.bloom_gaussian_kernal = bloom_settings["Gaussian-kernel"].asUInt();
            s_Data.BloomSettings.bloom_gaussian_kernal = (glm::clamp)(s_Data.BloomSettings.bloom_gaussian_kernal,
                                                                      LongMarch_GUASSIAN_KERNEL_MIN,
                                                                      LongMarch_GUASSIAN_KERNEL_MAX);
            s_Data.BloomSettings.bloom_sample_resolution_downScale = bloom_settings["Res-down-scale"].asUInt();
        }
        {
            const auto& dof_settings = graphicsConfiguration["DOF"];
            s_Data.DOFSettings.enable = dof_settings["Enable"].asBool();
            s_Data.DOFSettings.dof_threshold = dof_settings["Threshold"].asFloat();
            s_Data.DOFSettings.dof_blend_strength = dof_settings["Strength"].asFloat();
            s_Data.DOFSettings.dof_gaussian_kernal = dof_settings["Gaussian-kernel"].asUInt();
            s_Data.DOFSettings.dof_gaussian_kernal = (glm::clamp)(s_Data.DOFSettings.dof_gaussian_kernal,
                                                                  LongMarch_GUASSIAN_KERNEL_MIN,
                                                                  LongMarch_GUASSIAN_KERNEL_MAX);
            s_Data.DOFSettings.dof_sample_resolution_downScale = dof_settings["Res-down-scale"].asUInt();
            s_Data.DOFSettings.dof_refocus_rate = dof_settings["Refocus-rate"].asFloat();
        }

        s_Data.enable_reverse_z = true;
        RenderCommand::Reverse_Z(s_Data.enable_reverse_z);

        s_Data.enable_wireframe = false;

        if (s_Data.MotionBlur.enable_motionblur && !s_Data.enable_deferredShading)
        {
            ENGINE_INFO("Motion Blur requires Deferred Renderering as 'True' in the engine configuration file!");
        }
        if (s_Data.enable_taa && !s_Data.enable_deferredShading)
        {
            ENGINE_INFO("TAA requires Deferred Renderering as 'True' in the engine configuration file!");
        }
        if (s_Data.SSAOSettings.enable && !s_Data.enable_deferredShading)
        {
            ENGINE_INFO("Alchemy SSAO requires Deferred Renderering as 'True' in the engine configuration file!");
        }

        /**************************************************************
        *	Init Shader
        *
        **************************************************************/
        s_Data.ShaderMap["TransparentForwardShader"] = Shader::Create("$shader:forward_shader.vert",
                                                                      "$shader:forward_shader.frag");
        s_Data.ShaderMap["DeferredShader"] = Shader::Create("$shader:deferred_shader.vert",
                                                            "$shader:deferred_shader.frag");
        s_Data.ShaderMap["DynamicSSGIShader"] = Shader::Create("$shader:dynamic_ssgi_shader.vert",
                                                               "$shader:dynamic_ssgi_shader.frag");
        s_Data.ShaderMap["DynamicSSGIBlendShader"] = Shader::Create("$shader:dynamic_ssgi_blend_shader.vert",
                                                                    "$shader:dynamic_ssgi_blend_shader.frag");
        s_Data.ShaderMap["DynamicSSAOShader"] = Shader::Create("$shader:dynamic_ssao_shader.vert",
                                                               "$shader:dynamic_ssao_shader.frag");
        //s_Data.ShaderMap["DynamicSSAOColorShader"] = Shader::Create("$shader:dynamic_ao_color_shader.vert", "$shader:dynamic_ao_color_shader.frag");
        s_Data.ShaderMap["DynamicSSRShader"] = Shader::Create("$shader:dynamic_ssr_shader.vert",
                                                              "$shader:dynamic_ssr_shader.frag");
        s_Data.ShaderMap["DynamicSSRBlendShader"] = Shader::Create("$shader:dynamic_ssr_blend_shader.vert",
                                                                   "$shader:dynamic_ssr_blend_shader.frag");
        s_Data.ShaderMap["Bloom_Brightness_Filter"] = Shader::Create("$shader:dynamic_bloom_brightness_filter.vert",
                                                                     "$shader:dynamic_bloom_brightness_filter.frag");
        s_Data.ShaderMap["Bloom_Blend"] = Shader::Create("$shader:dynamic_bloom_blend_shader.vert",
                                                         "$shader:dynamic_bloom_blend_shader.frag");
        s_Data.ShaderMap["DOF_Dpeth"] = Shader::Create("$shader:dynamic_dof_depth_shader.vert",
                                                       "$shader:dynamic_dof_depth_shader.frag");
        s_Data.ShaderMap["DOF_Blend"] = Shader::Create("$shader:dynamic_dof_blend_shader.vert",
                                                       "$shader:dynamic_dof_blend_shader.frag");

        s_Data.ShaderMap["SMAAShader_edge"] = Shader::Create("$shader:smaa/smaa-edges.vert",
                                                             "$shader:smaa/smaa-edges-color.frag");
        s_Data.ShaderMap["SMAAShader_weight"] = Shader::Create("$shader:smaa/smaa-weights.vert",
                                                               "$shader:smaa/smaa-weights.frag");
        s_Data.ShaderMap["SMAAShader_blend"] = Shader::Create("$shader:smaa/smaa-blend.vert",
                                                              "$shader:smaa/smaa-blend.frag");
        s_Data.ShaderMap["SMAAShader_blend_T2X"] = Shader::Create("$shader:smaa/smaa-color-blend-T2X.vert",
                                                                  "$shader:smaa/smaa-color-blend-T2X.frag");

        s_Data.ShaderMap["FXAAShader"] = Shader::Create("$shader:simple_fxaa.vert", "$shader:simple_fxaa.frag");
        s_Data.ShaderMap["TAAShader"] = Shader::Create("$shader:simple_taa.vert", "$shader:simple_taa.frag");
        s_Data.ShaderMap["MotionBlur"] = Shader::Create("$shader:motion_blur.vert", "$shader:motion_blur.frag");
        s_Data.ShaderMap["DepthCopyShader"] = Shader::Create("$shader:depth_copy_shader.vert",
                                                             "$shader:depth_copy_shader.frag");
        s_Data.ShaderMap["BBoxShader"] = Shader::Create("$shader:bbox_shader.vert", "$shader:bbox_shader.frag");
        s_Data.ShaderMap["OutlineShader"] =
            Shader::Create("$shader:outline_shader.vert", "$shader:outline_shader.frag");
        s_Data.ShaderMap["ToneMapping"] = Shader::Create("$shader:tone_mapping.vert", "$shader:tone_mapping.frag");

        s_Data.ShaderMap["GaussianBlur"] = Shader::Create("$shader:guassian_blur.vert", "$shader:guassian_blur.frag");
        s_Data.ShaderMap["GaussianBlur_CSM"] = Shader::Create("$shader:guassian_blur_CSM.vert",
                                                              "$shader:guassian_blur_CSM.frag");
        s_Data.ShaderMap["GaussianBlur_Cube_PointLight"] = Shader::Create(
            "$shader:guassian_blur_cube_point_light.vert", "$shader:guassian_blur_cube_point_light.frag",
            "$shader:cubemap_geomtry_shader.geom");
        s_Data.ShaderMap["GaussianBlur_Comp_H"] = Shader::Create("$shader:guassian_blur_compute_H.comp");
        s_Data.ShaderMap["GaussianBlur_Comp_V"] = Shader::Create("$shader:guassian_blur_compute_V.comp");
        s_Data.ShaderMap["BilateralBlur"] =
            Shader::Create("$shader:bilateral_blur.vert", "$shader:bilateral_blur.frag");

        s_Data.ShaderMap["ParticleShader"] = Shader::Create("$shader:particle_shader.vert",
                                                            "$shader:particle_shader.frag");

        s_Data.ShaderMap["Equirectangular_To_Cubemap"] = Shader::Create(
            "$shader:equirectangular_to_cubemap_shader.vert", "$shader:equirectangular_to_cubemap_shader.frag",
            "$shader:cubemap_geomtry_shader.geom");
        s_Data.ShaderMap["Cubemap_To_Equirectangular"] = Shader::Create(
            "$shader:cubemap_to_equirectangular_shader.vert", "$shader:cubemap_to_equirectangular_shader.frag");
        s_Data.ShaderMap["SkyboxShader"] = Shader::Create("$shader:skybox_shader.vert", "$shader:skybox_shader.frag");
        s_Data.ShaderMap["Cubemap_Irradiance"] = Shader::Create("$shader:cubemap_irradiance_shader.vert",
                                                                "$shader:cubemap_irradiance_shader.frag",
                                                                "$shader:cubemap_geomtry_shader.geom");
        s_Data.ShaderMap["Cubemap_Radiance"] = Shader::Create("$shader:cubemap_radiance_shader.vert",
                                                              "$shader:cubemap_radiance_shader.frag",
                                                              "$shader:cubemap_geomtry_shader.geom");

        {
            s_Data.ShaderMap["MSMShadowBuffer_Transparent"] = Shader::Create(
                "$shader:shadow/momentShadowMap_shader.vert", "$shader:shadow/momentShadowMap_shader.frag");
            s_Data.ShaderMap["MSMShadowBuffer_Particle"] = Shader::Create(
                "$shader:shadow/momentShadowMap_shader_particle.vert",
                "$shader:shadow/momentShadowMap_shader_particle.frag");
            s_Data.ShaderMap["ShadowBuffer_Transparent"] = Shader::Create(
                "$shader:shadow/ShadowMap_shader_transparent.vert", "$shader:shadow/ShadowMap_shader_transparent.frag");
            s_Data.ShaderMap["ShadowBuffer_Particle"] = Shader::Create("$shader:shadow/ShadowMap_shader_particle.vert",
                                                                       "$shader:shadow/ShadowMap_shader_particle.frag");

            s_Data.ShaderMap["ShadowBuffer_Canonical"] = Shader::Create("$shader:shadow/ShadowMap_shader.vert",
                                                                        "$shader:shadow/ShadowMap_shader.frag");
            // used in outlining pass as a cheap way to draw objects
        }

        switch (s_Data.RENDER_MODE)
        {
        case RENDER_MODE::CANONICAL:
            s_Data.ShaderMap["ForwardShader"] = Shader::Create("$shader:forward_shader.vert",
                                                               "$shader:forward_shader.frag");
            s_Data.ShaderMap["GBufferShader"] = Shader::Create("$shader:gBuffer_shader.vert",
                                                               "$shader:gBuffer_shader.frag");
            s_Data.ShaderMap["ShadowBuffer"] = Shader::Create("$shader:shadow/ShadowMap_shader.vert",
                                                              "$shader:shadow/ShadowMap_shader.frag");
            s_Data.ShaderMap["MSMShadowBuffer"] = Shader::Create("$shader:shadow/momentShadowMap_shader.vert",
                                                                 "$shader:shadow/momentShadowMap_shader.frag");
        //s_Data.ShaderMap["MSMShadowBuffer_Cube"] = Shader::Create("$shader:shadow/momentShadowMap_shader_cube.vert", "$shader:shadow/momentShadowMap_shader_cube.frag", "$shader:cubemap_geomtry_shader.geom"); // Point light shadows use 6 array texture instead
            break;
        case RENDER_MODE::MULTIDRAW:
            s_Data.ShaderMap["ForwardShader"] = Shader::Create("$shader:forward_shader_MultiDraw.vert",
                                                               "$shader:forward_shader_MultiDraw.frag");
            s_Data.ShaderMap["GBufferShader"] = Shader::Create("$shader:gBuffer_shader_MultiDraw.vert",
                                                               "$shader:gBuffer_shader_MultiDraw.frag");
            s_Data.ShaderMap["ShadowBuffer"] = Shader::Create("$shader:shadow/ShadowMap_shader_MultiDraw.vert",
                                                              "$shader:shadow/ShadowMap_shader.frag");
            s_Data.ShaderMap["MSMShadowBuffer"] = Shader::Create("$shader:shadow/momentShadowMap_shader_MultiDraw.vert",
                                                                 "$shader:shadow/momentShadowMap_shader.frag");
        //s_Data.ShaderMap["MSMShadowBuffer_Cube"] = Shader::Create("$shader:shadow/momentShadowMap_shader_cube_MultiDraw.vert", "$shader:shadow/momentShadowMap_shader_cube.frag", "$shader:cubemap_geomtry_shader.geom"); // Point light shadows use 6 array texture instead
            break;
        }

        // Assign default shader
        switch (s_Data.RENDER_PIPE)
        {
        case RENDER_PIPE::DEFERRED:
            s_Data.ShaderMap["OpaqueRenderShader"] = s_Data.ShaderMap["GBufferShader"];
            break;
        case RENDER_PIPE::FORWARD:
            s_Data.ShaderMap["OpaqueRenderShader"] = s_Data.ShaderMap["ForwardShader"];
            break;
        }
        s_Data.CurrentShader = s_Data.ShaderMap["OpaqueRenderShader"];

        for (auto it = s_Data.ShaderMap.begin(); it != s_Data.ShaderMap.end(); ++it)
        {
            it->second->Bind();
            it->second->SetFloat3("Ia", Vec3f(s_Data.ambient));

            it->second->SetInt("u_AlbedoTexture",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   Material::MAT_TEXTURE_TYPE::ALBEDO));
            it->second->SetInt("u_NormalTexture",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   Material::MAT_TEXTURE_TYPE::NORMAL));
            it->second->SetInt("u_MetallicTexture",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   Material::MAT_TEXTURE_TYPE::METALLIC));
            it->second->SetInt("u_RoughnessTexture",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   Material::MAT_TEXTURE_TYPE::ROUGHNESS));
            it->second->SetInt("u_AOTexture",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   Material::MAT_TEXTURE_TYPE::BACKEDAO));

            it->second->SetInt(
                "g_Depth",
                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH));
            it->second->SetInt(
                "g_Normal",
                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL));
            it->second->SetInt("g_Velocity",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY));
            it->second->SetInt("g_Albedo_Emssive",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::ALBEDO_EMSSIVE));
            it->second->SetInt("g_BackedAO_Metallic_Roughness",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::BAKEDAO_METALLIC_ROUGHNESS));
            it->second->SetInt("u_DynamicSSAO",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::NUM));
            it->second->SetInt("u_DynamicSSGI",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::NUM) + 1);
            it->second->SetInt("u_DynamicSSR",
                               s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                   GBuffer::GBUFFER_TEXTURE_TYPE::NUM) + 1);

            it->second->SetInt("u_FragTexture", s_Data.fragTexture_0_slot);
            it->second->SetInt("u_FragTexture1", s_Data.fragTexture_1_slot);
            it->second->SetInt("u_FragTexture2", s_Data.fragTexture_2_slot);
            it->second->SetInt("u_FragTexture3", s_Data.fragTexture_3_slot);
            it->second->SetInt("u_FragTexture4", s_Data.fragTexture_4_slot);

            it->second->SetInt("u_IrradianceMap", s_Data.fragTexture_0_slot);
            it->second->SetInt("u_RadianceMap", s_Data.fragTexture_1_slot);
            it->second->SetInt("u_BrdfLUT", s_Data.fragTexture_2_slot);

            it->second->SetInt("colorTex", s_Data.fragTexture_0_slot);
            it->second->SetInt("edgesTex", s_Data.fragTexture_1_slot);
            it->second->SetInt("blendTex", s_Data.fragTexture_2_slot);
            it->second->SetInt("areaTex", s_Data.fragTexture_empty_slot + 1);
            it->second->SetInt("searchTex", s_Data.fragTexture_empty_slot + 2);
        }

        LongMarch_Vector<std::string> forwardShader = {"ForwardShader", "TransparentForwardShader"};
        LongMarch_Vector<std::string> deferredShader = {"GBufferShader", "DeferredShader"};

        s_Data.ListRenderShadersToPopulateShadowData.insert(s_Data.ListRenderShadersToPopulateShadowData.end(),
                                                            forwardShader.begin(), forwardShader.end());
        s_Data.ListRenderShadersToPopulateShadowData.insert(s_Data.ListRenderShadersToPopulateShadowData.end(),
                                                            deferredShader.begin(), deferredShader.end());

        LongMarch_Vector<std::string> MiscShader = {
            "ToneMapping", "ParticleShader", "BBoxShader", "SkyboxShader", "TAAShader", "DynamicSSAOShader",
            "DynamicSSGIShader", "BilateralBlur", "DynamicSSRShader", "DOF_Blend"
        };
        s_Data.ListShadersToPopulateData = s_Data.ListRenderShadersToPopulateShadowData;
        s_Data.ListShadersToPopulateData.insert(s_Data.ListShadersToPopulateData.end(), MiscShader.begin(),
                                                MiscShader.end());

        /**************************************************************
        *	Init Buffer Objects
        *
        **************************************************************/
        {
            constexpr float mean = 0.f;
            constexpr float num_std = 3.0f;
            for (uint32_t i = LongMarch_GUASSIAN_KERNEL_MIN; i <= LongMarch_GUASSIAN_KERNEL_MAX; ++i)
            {
                {
                    auto [offsets, weights] = DistributionMath::Gaussian1DHalf(i, mean, (i / 2.0f) / num_std);
                    auto weight = ShaderStorageBuffer::Create(&weights[0], weights.X() * sizeof(float));
                    auto offset = ShaderStorageBuffer::Create(&offsets[0], offsets.X() * sizeof(float));
                    s_Data.gpuBuffer.GuassinKernelHalf.try_emplace(i, weights.X(), offset, weight);
                }
                {
                    auto [offsets, weights] = DistributionMath::Gaussian1DHalfBilinear(i, mean, (i / 2.0f) / num_std);
                    auto weight = ShaderStorageBuffer::Create(&weights[0], weights.X() * sizeof(float));
                    auto offset = ShaderStorageBuffer::Create(&offsets[0], offsets.X() * sizeof(float));
                    s_Data.gpuBuffer.GuassinKernelHalfBilinear.try_emplace(i, weights.X(), offset, weight);
                }
            }

            // Uniform buffer
            s_Data.gpuBuffer.DirectionalLightBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.PointLightBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.SpotLightBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.ShadowPVMatrixBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.BoneTransformMatrixBuffer = ShaderStorageBuffer::Create(nullptr, 0);

            s_Data.gpuBuffer.CurrentModelBuffer = UniformBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.CurrentMaterialBuffer = UniformBuffer::Create(nullptr, 0);

            s_Data.multiDrawBuffer.MultiDraw_ssbo_ShadowModelTrsBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.multiDrawBuffer.MultiDraw_ssbo_ModelTrsBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.multiDrawBuffer.MultiDraw_ssbo_MaterialsBuffer = ShaderStorageBuffer::Create(nullptr, 0);
            s_Data.multiDrawBuffer.MultiDraw_ssbo_BoneBaseOffset = ShaderStorageBuffer::Create(nullptr, 0);

            // Frame buffer
            s_Data.gpuBuffer.CurrentFrameBuffer = nullptr;
            s_Data.gpuBuffer.PrevFinalFrameBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
            s_Data.gpuBuffer.CurrentFinalFrameBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
            s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.FrameBuffer_1 = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
            s_Data.gpuBuffer.FrameBuffer_2 = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
            s_Data.gpuBuffer.FrameBuffer_3 = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
            s_Data.gpuBuffer.FrameBuffer_4 = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);

            s_Data.gpuBuffer.PrevDynamicSSGIBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.CurrentDynamicSSGIBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.CurrentDynamicSSAOBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::UINT_R8);
            s_Data.gpuBuffer.CurrentDynamicSSRBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.CurrentBackFaceDepthBuffer = ShadowBuffer::Create(
                1, 1, ShadowBuffer::SHADOW_MAP_TYPE::BASIC);
            s_Data.gpuBuffer.CurrentDynamicBloomBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.CurrentDynamicDOFBuffer = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_1 = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT32_R);
            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_2 = FrameBuffer::Create(
                1, 1, FrameBuffer::BUFFER_FORMAT::FLOAT32_R);

            // GBuffer
            s_Data.gpuBuffer.CurrentGBuffer = GBuffer::Create(1, 1, GBuffer::GBUFFER_TYPE::DEFAULT);
            s_Data.gpuBuffer.CurrentThinGBuffer = GBuffer::Create(1, 1, GBuffer::GBUFFER_TYPE::THIN);

            // Voxel buffer
            {
                constexpr uint32_t dimension = 64;
                s_Data.gpuBuffer.voxelAlbedo = VoxelBuffer::Create(dimension, dimension, dimension,
                                                                   VoxelBuffer::BUFFER_TYPE::ALBEDO);
                s_Data.gpuBuffer.voxelNormal = VoxelBuffer::Create(dimension, dimension, dimension,
                                                                   VoxelBuffer::BUFFER_TYPE::NORMAL);
                s_Data.gpuBuffer.voxelEmissive = VoxelBuffer::Create(dimension, dimension, dimension,
                                                                     VoxelBuffer::BUFFER_TYPE::EMISSIVE);
                s_Data.gpuBuffer.voxelRadiance = VoxelBuffer::Create(dimension, dimension, dimension,
                                                                     VoxelBuffer::BUFFER_TYPE::RADIANCE);
                for (int i = 0; i < 6; ++i)
                {
                    s_Data.gpuBuffer.voxelRadiance_mimaps[i] = VoxelBuffer::Create(
                        dimension / 2, dimension / 2, dimension / 2, VoxelBuffer::BUFFER_TYPE::RADIANCE_MIPMAP);
                    s_Data.gpuBuffer.voxelRadiance_mimaps[i]->GenerateMipMapLevel();
                }
            }

            // IBL related buffers
            {
                const auto& EnvMapConfiguration = graphicsConfiguration["Env-mapping"];
                LongMarch_UnorderedMap<std::string, std::shared_ptr<SkyBoxBuffer>> _envmaps;
                _envmaps.emplace("original", nullptr);
                _envmaps.emplace("irradiance", nullptr);
                _envmaps.emplace("radiance", nullptr);
                const auto& skyboxes_name = EnvMapConfiguration["All-Sky-box"];
                for (auto i(0u); i < skyboxes_name.size(); ++i)
                {
                    const auto& name = skyboxes_name[i].asString();
                    s_Data.gpuBuffer.EnvCubeMaps.emplace(name, _envmaps);
                    s_Data.gpuBuffer.EnvEquiMaps.emplace(name, nullptr);
                }
                s_Data.EnvMapping.CurrentEnvMapName = EnvMapConfiguration["Current-Sky-box"].asString();
                if (s_Data.EnvMapping.CurrentEnvMapName != "")
                {
                    ASSERT(s_Data.gpuBuffer.EnvEquiMaps.contains(s_Data.EnvMapping.CurrentEnvMapName),
                           s_Data.EnvMapping.CurrentEnvMapName + " does not exists!");
                }
                s_Data.EnvMapping.enable_env_mapping = EnvMapConfiguration["Enable"].asBool();
                s_Data.EnvMapping.brdf_lut_name = EnvMapConfiguration["BRDF-lut"].asString();
            }
        }

        {
            // Quad buffer - specialized for triangle drawing
            s_Data.gpuBuffer.FullScreenQuadVAO = VertexArray::Create();
            float quadVertices[] = {
                -1.f, -1.f, 0.0f, 0.0f, 0.0f,
                1.f, -1.f, 0.0f, 1.0f, 0.0f,
                1.f, 1.f, 0.0f, 1.0f, 1.0f,
                -1.f, 1.f, 0.0f, 0.0f, 1.0f
            };
            auto QuadVertexBuffer = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
            QuadVertexBuffer->SetLayout({
                {VertexDataType::Float3, "a_Position"},
                {VertexDataType::Float2, "a_TexCoord"}
            });
            s_Data.gpuBuffer.FullScreenQuadVAO->AddVertexBuffer(QuadVertexBuffer);
            // The quad indices will be used to draw as triangles
            uint8_t squareIndices[6] = {
                0, 1, 2, 2, 3, 0
            };
            auto QuadIndexBuffer = IndexBuffer::Create(squareIndices, sizeof(squareIndices), sizeof(uint8_t));
            s_Data.gpuBuffer.FullScreenQuadVAO->SetIndexBuffer(QuadIndexBuffer);
        }

        {
            // Tri buffer - specialized for full screen triangle drawing
            s_Data.gpuBuffer.FullScreenTriVAO = VertexArray::Create();
            float TriVertices[] = {
                -1.f, -1.f, 0.0f, 0.0f, 0.0f,
                3.f, -1.f, 0.0f, 2.0f, 0.0f,
                -1.f, 3.f, 0.0f, 0.0f, 2.0f
            };
            auto TriVertexBuffer = VertexBuffer::Create(TriVertices, sizeof(TriVertices));
            TriVertexBuffer->SetLayout({
                {VertexDataType::Float3, "a_Position"},
                {VertexDataType::Float2, "a_TexCoord"}
            });
            s_Data.gpuBuffer.FullScreenTriVAO->AddVertexBuffer(TriVertexBuffer);
            // The tri indices will be used to draw as triangles
            uint8_t triIndices[3] = {
                0, 1, 2
            };
            auto TriIndexBuffer = IndexBuffer::Create(triIndices, sizeof(triIndices), sizeof(uint8_t));
            s_Data.gpuBuffer.FullScreenTriVAO->SetIndexBuffer(TriIndexBuffer);
        }

        {
            // BBox buffer - specialized for line drawing
            s_Data.gpuBuffer.BBoxVAO = VertexArray::Create();
            float cubeVertices[] = {
                -0.5, -0.5, -0.5,
                0.5, -0.5, -0.5,
                0.5, 0.5, -0.5,
                -0.5, 0.5, -0.5,
                -0.5, -0.5, 0.5,
                0.5, -0.5, 0.5,
                0.5, 0.5, 0.5,
                -0.5, 0.5, 0.5,
            };
            auto BBoxVertexBuffer = VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
            BBoxVertexBuffer->SetLayout({
                {VertexDataType::Float3, "a_Position"}
            });
            s_Data.gpuBuffer.BBoxVAO->AddVertexBuffer(BBoxVertexBuffer);
            s_Data.gpuBuffer.BBoxInstBO = VertexBuffer::Create(nullptr, 0);
            s_Data.gpuBuffer.BBoxInstBO->SetLayout({
                {VertexDataType::Float4, "a_ModelTre_Col1", 1},
                {VertexDataType::Float4, "a_ModelTre_Col2", 1},
                {VertexDataType::Float4, "a_ModelTre_Col3", 1},
                {VertexDataType::Float4, "a_ModelTre_Col4", 1},
            });
            s_Data.gpuBuffer.BBoxVAO->AddVertexBuffer(s_Data.gpuBuffer.BBoxInstBO);
            // The BBox indices will be used to draw as lines
            uint8_t cubeIndices[] = {
                0, 1, 2, 3,
                4, 5, 6, 7,
                0, 4, 1, 5,
                2, 6, 3, 7,
                0, 3, 4, 7,
                1, 2, 5, 6,
            };
            auto BBoxIndexBuffer = IndexBuffer::Create(cubeIndices, sizeof(cubeIndices), sizeof(uint8_t));
            s_Data.gpuBuffer.BBoxVAO->SetIndexBuffer(BBoxIndexBuffer);
        }

        // quad sprite particle VAO
        {
            s_Data.gpuBuffer.particleVAO = VertexArray::Create();
            float quadVertices[] = {
                -0.5f, 0.5f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.0f, 0.0f,
                0.5f, 0.5f, 1.0f, 1.0f,
                0.5f, -0.5f, 1.0f, 0.0f,
            };
            auto quadVBO = VertexBuffer::Create(quadVertices, sizeof(quadVertices));

            quadVBO->SetLayout({
                {VertexDataType::Float2, "position"},
                {VertexDataType::Float2, "uv"}
            });
            s_Data.gpuBuffer.particleVAO->AddVertexBuffer(quadVBO);
            uint8_t squareIndices[6] = {
                0, 1, 3, 3, 2, 0
            };
            auto QuadIndexBuffer = IndexBuffer::Create(squareIndices, sizeof(squareIndices), sizeof(uint8_t));
            s_Data.gpuBuffer.particleVAO->SetIndexBuffer(QuadIndexBuffer);
            s_Data.gpuBuffer.particleInstBO = VertexBuffer::Create(
                nullptr, MAX_PARTICLE_INSTANCES * PARTICLE_INSTANCED_DATA_LENGTH * sizeof(GLfloat));
            s_Data.gpuBuffer.particleInstBO->SetLayout({
                {VertexDataType::Float4, "model_col0", 1},
                {VertexDataType::Float4, "model_col1", 1},
                {VertexDataType::Float4, "model_col2", 1},
                {VertexDataType::Float4, "model_col3", 1},
                {VertexDataType::Float4, "uv_offsets", 1},
                {VertexDataType::Float, "blend_factor", 1}
            });
            s_Data.gpuBuffer.particleVAO->AddVertexBuffer(s_Data.gpuBuffer.particleInstBO);
        }

        // Cube buffer - specialized for 360 degrees cube drawing
        {
            s_Data.gpuBuffer.FullScreenCubeVAO = VertexArray::Create();
            float cubeVertices[] = {
                // The following indexes are in OpenGL Coordinate!
                // back face
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
            };
            auto CubeVertexBuffer = VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
            CubeVertexBuffer->SetLayout({
                {VertexDataType::Float3, "a_Position"},
                {VertexDataType::Float3, "a_Normal"},
                {VertexDataType::Float2, "a_TexCoord"}
            });
            s_Data.gpuBuffer.FullScreenCubeVAO->AddVertexBuffer(CubeVertexBuffer);
            // Cbe indices will be used to draw triangles
            uint8_t cubeIndices[] = {
                0, 1, 2, 3, 4, 5,
                6, 7, 8, 9, 10, 11,
                12, 13, 14, 15, 16, 17,
                18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29,
                30, 31, 32, 33, 34, 35,
            };
            auto CubeIndexBuffer = IndexBuffer::Create(cubeIndices, sizeof(cubeIndices), sizeof(uint8_t));
            s_Data.gpuBuffer.FullScreenCubeVAO->SetIndexBuffer(CubeIndexBuffer);
        }

        {
            // Multi draw buffer
            s_Data.multiDrawBuffer.MultiDraw_VAO = VertexArray::Create();
            s_Data.multiDrawBuffer.MultiDraw_VBO = VertexBuffer::Create(nullptr, 0);
#if MESH_VERTEX_DATA_FORMAT == 0
			s_Data.multiDrawBuffer.MultiDraw_VBO->SetLayout({
					{ VertexDataType::Float3, "a_VertexPnt" },
					{ VertexDataType::Float3, "a_VertexNrm" },
					{ VertexDataType::Float2, "a_VertexTex" },
					{ VertexDataType::Float3, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 1
			s_Data.multiDrawBuffer.MultiDraw_VBO->SetLayout({
					{ VertexDataType::Float3, "a_VertexPnt" },
					{ VertexDataType::Float4, "a_VertexNrm" },
					{ VertexDataType::Float2, "a_VertexTex" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 2
			s_Data.multiDrawBuffer.MultiDraw_VBO->SetLayout({
					{ VertexDataType::Float3, "a_VertexPnt" },
					{ VertexDataType::Float2, "a_VertexNrm" },
					{ VertexDataType::Float2, "a_VertexTex" },
					{ VertexDataType::Float2, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 3
			s_Data.multiDrawBuffer.MultiDraw_VBO->SetLayout({
					{ VertexDataType::Float3, "a_VertexPnt" },
					{ VertexDataType::HFloat2, "a_VertexNrm" },
					{ VertexDataType::HFloat2, "a_VertexTex" },
					{ VertexDataType::HFloat2, "a_VertexTan" }
				});
#elif MESH_VERTEX_DATA_FORMAT == 4
            s_Data.multiDrawBuffer.MultiDraw_VBO->SetLayout({
                {VertexDataType::HFloat4, "a_VertexPnt"},
                {VertexDataType::HFloat2, "a_VertexNrm"},
                {VertexDataType::HFloat2, "a_VertexTex"},
                {VertexDataType::HFloat2, "a_VertexTan"},
                {VertexDataType::HFloat2, "a_BoneIndexWeight1"},
                {VertexDataType::HFloat2, "a_BoneIndexWeight2"},
                {VertexDataType::HFloat2, "a_BoneIndexWeight3"}
            });
#endif
            s_Data.multiDrawBuffer.MultiDraw_VAO->AddVertexBuffer(s_Data.multiDrawBuffer.MultiDraw_VBO);
            s_Data.multiDrawBuffer.MultiDraw_InstBO = VertexBuffer::Create(nullptr, TWO_16 * sizeof(uint32_t));
            s_Data.multiDrawBuffer.MultiDraw_InstBO->SetLayout({
                {VertexDataType::uInt, "a_InstanceId", 1}
            });
            s_Data.multiDrawBuffer.MultiDraw_VAO->AddVertexBuffer(s_Data.multiDrawBuffer.MultiDraw_InstBO);
            s_Data.multiDrawBuffer.MultiDraw_IBO = IndexBuffer::Create(nullptr, 0, sizeof(MeshData::TriData) / 3);
            s_Data.multiDrawBuffer.MultiDraw_VAO->SetIndexBuffer(s_Data.multiDrawBuffer.MultiDraw_IBO);
            s_Data.multiDrawBuffer.MultiDraw_CBO = IndexedIndirectCommandBuffer::Create(nullptr, 0);
        }
    }
    /**************************************************************
    *	Subscribe for events
    **************************************************************/
    {
        {
            auto queue = EventQueue<EngineGraphicsDebugEventType>::GetInstance();
            queue->Subscribe(EngineGraphicsDebugEventType::SET_ENV_MAPPING, &Renderer3D::_ON_SET_ENV_MAPPING);
            queue->Subscribe(EngineGraphicsDebugEventType::TOGGLE_SHADOW, &Renderer3D::_ON_TOGGLE_SHADOW);
            queue->Subscribe(EngineGraphicsDebugEventType::SWITCH_G_BUFFER_DISPLAY,
                             &Renderer3D::_ON_SWITCH_GBUFFER_MODE);
        }
        {
            auto queue = EventQueue<EngineGraphicsEventType>::GetInstance();
            queue->Subscribe(EngineGraphicsEventType::TOGGLE_MOTION_BLUR, &Renderer3D::_ON_TOGGLE_MOTION_BLUR);
            queue->Subscribe(EngineGraphicsEventType::TOGGLE_TAA, &Renderer3D::_ON_TOGGLE_TAA);
            queue->Subscribe(EngineGraphicsEventType::TOGGLE_FXAA, &Renderer3D::_ON_TOGGLE_FXAA);
            queue->Subscribe(EngineGraphicsEventType::TOGGLE_SMAA, &Renderer3D::_ON_TOGGLE_SMAA);
            queue->Subscribe(EngineGraphicsEventType::SWITCH_TONE_MAPPING, &Renderer3D::_ON_SWITCH_TONEMAP_MODE);
            queue->Subscribe(EngineGraphicsEventType::SET_GAMMA_VALUE, &Renderer3D::_ON_SET_GAMMA_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_SSGI_VALUE, &Renderer3D::_ON_SET_SSGI_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_SSAO_VALUE, &Renderer3D::_ON_SET_SSAO_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_SSR_VALUE, &Renderer3D::_ON_SET_SSR_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_BLOOM_VALUE, &Renderer3D::_ON_SET_BLOOM_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_DOF_VALUE, &Renderer3D::_ON_SET_DOF_VALUE);
            queue->Subscribe(EngineGraphicsEventType::SET_DOF_TARGET, &Renderer3D::_ON_SET_DOF_TARGET);
        }
    }
}

void longmarch::Renderer3D::Shutdown()
{
    s_init = false;
}

void longmarch::Renderer3D::EditorRenderGraphicsSettings()
{
    if (ImGui::TreeNode("Graphics Settings"))
    {
        auto graphicDebugEventQueue = EventQueue<EngineGraphicsDebugEventType>::GetInstance();
        auto graphicEventQueue = EventQueue<EngineGraphicsEventType>::GetInstance();

        static auto engineConfiguration = FileSystem::GetNewJsonCPP("$root:engine-config.json");
        static auto windowConfig = engineConfiguration["window"];
        static auto graphicsConfig = engineConfiguration["graphics"];
        static auto MotionBlurConfig = graphicsConfig["Motion-blur"];
        static auto EnvMapConfig = graphicsConfig["Env-mapping"];
        static auto SMAAConfig = graphicsConfig["SMAA"];
        static auto SSGIConfig = graphicsConfig["SSGI"];
        static auto SSAOConfig = graphicsConfig["SSAO"];
        static auto SSRConfig = graphicsConfig["SSR"];
        static auto BloomConfig = graphicsConfig["Bloom"];
        static auto DOFConfig = graphicsConfig["DOF"];

        // 1. Window mode
        static const char* windowModes[]{"Fullscreen", "Borderless Windowed", "Windowed"};
        static int selected_windowModes = windowConfig["Full-screen"].asInt();
        // 2. Window resolution
        static const char* windowReso[]{
            "1920x1080", "1600x900", "1366x768", "1280x720", // 16:9
            "1920x900", "1920x720", "1600x720", // Wide screen
            "1600x1050", "1440x900", // 16:10
            "1280x960", "900x600" // 4:3
        };
        static int selected_windowReso = 0;
        // V-sync
        static bool checkVSync = windowConfig["V-sync"].asBool();
        // GPU-sync
        static bool checkGPUSync = windowConfig["GPU-sync"].asBool();
        // G buffer mode
        static const char* gbufferModes[]{
            "Default", "Depth", "Normal", "Albedo", "Emission", "Specular", "Roughness", "Metallic",
            "Ambient occulusion", "SS Velocity"
        };
        static int selected_gbufferModes = 0;

        // Toggle shadow
        static bool checkShadow = graphicsConfig["Shadow"].asBool();
        // Deferred shading
        static bool checkDeferredShading = graphicsConfig["Deferred-shading"].asBool();

        // FXAA
        static bool checkFXAA = graphicsConfig["FXAA"].asBool();
        // TAA
        static bool checkTAA = graphicsConfig["TAA"].asBool();
        // Tone mapping
        static const char* toneMapModes[]{"AcesFilm", "Luminance", "Uncharted2", "Filmic", "Reinhard"};
        static int selected_toneMap = 0;
        // Gamma
        static float valueGamma = 2.2f;

        // Motion Blur
        static bool checkMotionBlur = MotionBlurConfig["Enable"].asBool();
        static int valueMotionblurShutterSpeed = MotionBlurConfig["Motion-blur-shutter-speed"].asInt();

        // Env mapping
        static std::string valueCurrentEnvMapName = EnvMapConfig["Current-Sky-box"].asString();
        static LongMarch_Vector<std::string> valueAllEnvMapName;
        valueAllEnvMapName.clear();
        const auto& skyboxes_name = EnvMapConfig["All-Sky-box"];
        for (auto i(0u); i < skyboxes_name.size(); ++i)
        {
            const auto& name = skyboxes_name[i].asString();
            valueAllEnvMapName.push_back(name);
        }
        static bool checkEvnMapping = EnvMapConfig["Enable"].asBool();

        // SMAA
        static bool checkSMAA = SMAAConfig["Enable"].asBool();
        static int valueSMAAMode = SMAAConfig["Mode"].asInt();

        // SSGI
        static bool checkSSGI = SSGIConfig["Enable"].asBool();
        static int valueSSGISample = SSGIConfig["Num-samples"].asInt();
        static int valueSSGISampleResDownScale = SSGIConfig["Res-down-scale"].asInt();
        static int valueSSGIBlurKernel = SSGIConfig["Gaussian-kernel"].asInt();
        static float valueSSGISampleRadius = SSGIConfig["Radius"].asFloat();
        static float valueSSGIStrength = SSGIConfig["Strength"].asFloat();

        // SSAO
        static bool checkSSAO = SSAOConfig["Enable"].asBool();
        static int valueSSAOSample = SSAOConfig["Num-samples"].asInt();
        static int valueSSAOSampleResDownScale = SSAOConfig["Res-down-scale"].asInt();
        static int valueSSAOBlurKernel = SSAOConfig["Gaussian-kernel"].asInt();
        static float valueSSAOSampleRadius = SSAOConfig["Radius"].asFloat();
        static float valueSSAOScale = SSAOConfig["Scale"].asFloat();
        static float valueSSAOPower = SSAOConfig["Power"].asFloat();

        // SSR
        static bool checkSSR = SSRConfig["Enable"].asBool();
        static int valueSSRBlurKernel = SSRConfig["Gaussian-kernel"].asInt();
        static int valueSSRSampleResDownScale = SSRConfig["Res-down-scale"].asInt();
        static bool checkSSRDebug = false;

        // Bloom
        static bool checkBloom = BloomConfig["Enable"].asBool();
        static float valueBloomThreshold = BloomConfig["Threshold"].asFloat();
        static float valueBloomStrength = BloomConfig["Strength"].asFloat();
        static int valueBloomBlurKernel = BloomConfig["Gaussian-kernel"].asInt();
        static int valueBloomSampleResDownScale = BloomConfig["Res-down-scale"].asFloat();

        // DOF
        static bool checkDOF = DOFConfig["Enable"].asBool();
        static float valueDOFThreshold = DOFConfig["Threshold"].asFloat();
        static float valueDOFStrength = DOFConfig["Strength"].asFloat();
        static int valueDOFBlurKernel = DOFConfig["Gaussian-kernel"].asInt();
        static int valueDOFSampleResDownScale = DOFConfig["Res-down-scale"].asFloat();
        static float valueDOFRefocusRate = DOFConfig["Refocus-rate"].asFloat();
        static bool checkDOFDebug = false;

        constexpr int yoffset_item = 5;

        ImGui::Dummy(ImVec2(0, yoffset_item));
        if (ImGui::Button("Reset graphics settings to default values"))
        {
            checkVSync = windowConfig["V-sync"].asBool();
            checkGPUSync = windowConfig["GPU-sync"].asBool();
            selected_gbufferModes = 0;;
            checkShadow = graphicsConfig["Shadow"].asBool();
            checkDeferredShading = graphicsConfig["Deferred-shading"].asBool();
            checkFXAA = graphicsConfig["FXAA"].asBool();
            checkTAA = graphicsConfig["TAA"].asBool();
            {
                selected_toneMap = 0;
                valueGamma = 2.2f;
            }
            {
                // Motion Blur
                checkMotionBlur = MotionBlurConfig["Enable"].asBool();
                valueMotionblurShutterSpeed = MotionBlurConfig["Motion-blur-shutter-speed"].asInt();
            }
            {
                // Env mapping
                valueCurrentEnvMapName = EnvMapConfig["Current-Sky-box"].asString();
                valueAllEnvMapName.clear();
                const auto& skyboxes_name = EnvMapConfig["All-Sky-box"];
                for (auto i(0u); i < skyboxes_name.size(); ++i)
                {
                    const auto& name = skyboxes_name[i].asString();
                    valueAllEnvMapName.push_back(name);
                }
                checkEvnMapping = EnvMapConfig["Enable"].asBool();
            }
            {
                // SMAA
                checkSMAA = SMAAConfig["Enable"].asBool();
                valueSMAAMode = SMAAConfig["Mode"].asInt();
            }
            {
                // SSGI
                checkSSGI = SSGIConfig["Enable"].asBool();
                valueSSGISample = SSGIConfig["Num-samples"].asInt();
                valueSSGISampleResDownScale = SSGIConfig["Res-down-scale"].asInt();
                valueSSGIBlurKernel = SSGIConfig["Gaussian-kernel"].asInt();
                valueSSGISampleRadius = SSGIConfig["Radius"].asFloat();
                valueSSGIStrength = SSGIConfig["Strength"].asFloat();
            }
            {
                // SSAO
                checkSSAO = SSAOConfig["Enable"].asBool();
                valueSSAOSample = SSAOConfig["Num-samples"].asInt();
                valueSSAOSampleResDownScale = SSAOConfig["Res-down-scale"].asInt();
                valueSSAOBlurKernel = SSAOConfig["Gaussian-kernel"].asInt();
                valueSSAOSampleRadius = SSAOConfig["Radius"].asFloat();
                valueSSAOScale = SSAOConfig["Scale"].asFloat();
                valueSSAOPower = SSAOConfig["Power"].asFloat();
            }
            {
                // SSR
                checkSSR = SSRConfig["Enable"].asBool();
                valueSSRBlurKernel = SSRConfig["Gaussian-kernel"].asFloat();
                valueSSRSampleResDownScale = SSRConfig["Res-down-scale"].asFloat();
                checkSSRDebug = false;
            }
            {
                // Bloom
                checkBloom = BloomConfig["Enable"].asBool();
                valueBloomThreshold = BloomConfig["Threshold"].asFloat();
                valueBloomStrength = BloomConfig["Strength"].asFloat();
                valueBloomBlurKernel = BloomConfig["Gaussian-kernel"].asInt();
                valueBloomSampleResDownScale = BloomConfig["Res-down-scale"].asFloat();
            }
            {
                // DOF
                checkDOF = DOFConfig["Enable"].asBool();
                valueDOFThreshold = DOFConfig["Threshold"].asFloat();
                valueDOFStrength = DOFConfig["Strength"].asFloat();
                valueDOFBlurKernel = DOFConfig["Gaussian-kernel"].asInt();
                valueDOFSampleResDownScale = DOFConfig["Res-down-scale"].asFloat();
                valueDOFRefocusRate = DOFConfig["Refocus-rate"].asFloat();
                checkDOFDebug = false;
            }
            {
                auto e = MemoryManager::Make_shared<ToggleVSyncEvent>(checkVSync);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<ToggleGPUSyncEvent>(checkGPUSync);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SwitchGBufferEvent>(selected_gbufferModes);
                graphicDebugEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<ToggleShadowEvent>(checkShadow);
                graphicDebugEventQueue->Publish(e);
            }

            {
                auto e = MemoryManager::Make_shared<
                    SetEnvironmentMappingEvent>(checkEvnMapping, valueCurrentEnvMapName);
                graphicDebugEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<
                    ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<ToggleTAAEvent>(checkTAA);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<ToggleSMAAEvent>(checkSMAA, valueSMAAMode);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<ToggleFXAAEvent>(checkFXAA);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SwitchToneMappingEvent>(selected_toneMap);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetGammaValueEvent>(valueGamma);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample,
                                                                       valueSSGISampleResDownScale, valueSSGIBlurKernel,
                                                                       valueSSGISampleRadius, valueSSGIStrength);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample,
                                                                       valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                                                                       valueSSAOSampleRadius, valueSSAOScale,
                                                                       valueSSAOPower);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel,
                                                                      valueSSRSampleResDownScale, checkSSRDebug);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength,
                                                                   valueBloomBlurKernel, valueBloomSampleResDownScale);
                graphicEventQueue->Publish(e);
            }
            {
                auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                valueDOFRefocusRate, checkDOFDebug);
                graphicEventQueue->Publish(e);
            }
        }

        if (ImGui::Button("Save graphics settings to config"))
        {
            constexpr auto config_path = "$root:engine-config.json";
            auto engineConfiguration = FileSystem::GetNewJsonCPP(config_path);

            Json::StreamWriterBuilder builder;
            builder["commentStyle"] = "None";
            builder["indentation"] = "    ";
            builder["precision"] = 4;
            builder["precisionType"] = "decimal";
            builder["dropNullPlaceholders"] = false;
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

            {
                auto& graphicsConfig = engineConfiguration["graphics"];
                {
                    auto& MotionBlurConfig = graphicsConfig["Motion-blur"];
                    MotionBlurConfig["Enable"] = checkMotionBlur;
                    MotionBlurConfig["Motion-blur-shutter-speed"] = valueMotionblurShutterSpeed;
                }
                {
                    auto& EnvMapConfig = graphicsConfig["Env-mapping"];
                    EnvMapConfig["Current-Sky-box"] = valueCurrentEnvMapName;
                    EnvMapConfig["Enable"] = checkEvnMapping;
                }
                {
                    auto& SMAAConfig = graphicsConfig["SMAA"];
                    SMAAConfig["Enable"] = checkSMAA;
                    SMAAConfig["Mode"] = valueSMAAMode;
                }
                {
                    auto& SSGIConfig = graphicsConfig["SSGI"];
                    SSGIConfig["Enable"] = checkSSGI;
                    SSGIConfig["Num-samples"] = valueSSGISample;
                    SSGIConfig["Res-down-scale"] = valueSSGISampleResDownScale;
                    SSGIConfig["Gaussian-kernel"] = valueSSGIBlurKernel;
                    SSGIConfig["Radius"] = valueSSGISampleRadius;
                    SSGIConfig["Strength"] = valueSSGIStrength;
                }
                {
                    auto& SSAOConfig = graphicsConfig["SSAO"];
                    SSAOConfig["Enable"] = checkSSAO;
                    SSAOConfig["Num-samples"] = valueSSAOSample;
                    SSAOConfig["Res-down-scale"] = valueSSAOSampleResDownScale;
                    SSAOConfig["Gaussian-kernel"] = valueSSAOBlurKernel;
                    SSAOConfig["Radius"] = valueSSAOSampleRadius;
                    SSAOConfig["Scale"] = valueSSAOScale;
                    SSAOConfig["Power"] = valueSSAOPower;
                }
                {
                    auto& SSRConfig = graphicsConfig["SSR"];
                    SSRConfig["Enable"] = checkSSR;
                    SSRConfig["Gaussian-kernel"] = valueSSRBlurKernel;
                    SSRConfig["Res-down-scale"] = valueSSRSampleResDownScale;
                }
                {
                    auto& BloomConfig = graphicsConfig["Bloom"];
                    BloomConfig["Enable"] = checkBloom;
                    BloomConfig["Threshold"] = valueBloomThreshold;
                    BloomConfig["Strength"] = valueBloomStrength;
                    BloomConfig["Gaussian-kernel"] = valueBloomBlurKernel;
                    BloomConfig["Res-down-scale"] = valueBloomSampleResDownScale;
                }
                {
                    auto& DOFConfig = graphicsConfig["DOF"];
                    DOFConfig["Enable"] = checkDOF;
                    DOFConfig["Threshold"] = valueDOFThreshold;
                    DOFConfig["Strength"] = valueDOFStrength;
                    DOFConfig["Gaussian-kernel"] = valueDOFBlurKernel;
                    DOFConfig["Res-down-scale"] = valueDOFSampleResDownScale;
                    DOFConfig["Refocus-rate"] = valueDOFRefocusRate;
                }
            }

            auto& output = FileSystem::OpenOfstream(config_path, FileSystem::FileType::OPEN_BINARY);
            writer->write(engineConfiguration, &output);
            FileSystem::CloseOfstream(config_path);
            FileSystem::RemoveCachedJsonCPP(config_path);
            //< Remove cached json file so that we can load the one that has just written to
        }
        ImGui::Dummy(ImVec2(0, yoffset_item));

        if (ImGui::BeginTabBar("Graphics Settings"))
        {
            if (ImGui::BeginTabItem("General"))
            {
                constexpr int yoffset_item = 5;
                // 1. Window mode
                {
                    if (ImGui::Combo("Display", &selected_windowModes, windowModes, IM_ARRAYSIZE(windowModes)))
                    {
                        // TODO : Event
                    }
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // 2. Window resolution
                {
                    // if (ImGui::Combo("Resolution", &selected_windowReso, windowReso, IM_ARRAYSIZE(windowReso)))
                    // {
                    //     // TODO : Set window resolution Event
                    // }
                    static int resolution_ratio = s_Data.resolution_ratio * 100;
                    if (ImGui::SliderInt("Resolution Scale", &resolution_ratio, 50, 150))
                    {
                        s_Data.resolution_ratio = resolution_ratio * 0.01f;
                    }
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // V-sync
                {
                    if (ImGui::Checkbox("V-Sync", &checkVSync))
                    {
                        auto e = MemoryManager::Make_shared<ToggleVSyncEvent>(checkVSync);
                        graphicEventQueue->Publish(e);
                    }
                }
                // GPU-sync
                {
                    if (ImGui::Checkbox("GPU Sync", &checkGPUSync))
                    {
                        auto e = MemoryManager::Make_shared<ToggleGPUSyncEvent>(checkGPUSync);
                        graphicEventQueue->Publish(e);
                    }
                }
                // OpenGL Check Error (performance expensive)
                {
                    static bool bGLCheckError = longmarch::g_bGLCheckError;
                    if (ImGui::Checkbox("OpenGL Check Error", &bGLCheckError))
                    {
                        longmarch::g_bGLCheckError = bGLCheckError;
                    }
                    ImGuiUtil::InlineHelpMarker("Enable OpenGL Error Check might cost performance hit.");
                }
                // High precision Frame control
                {
                    static bool checkHighPFPSMode = FramerateController::GetInstance()->GetHighPrecisionMode();
                    if (ImGui::Checkbox("High precision Frame control", &checkHighPFPSMode))
                    {
                        FramerateController::GetInstance()->SetHighPrecisionMode(checkHighPFPSMode);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Lighting/Shadow"))
            {
                constexpr int yoffset_item = 5;
                // G buffer mode
                {
                    if (ImGui::Combo("G-buffer", &selected_gbufferModes, gbufferModes, IM_ARRAYSIZE(gbufferModes)))
                    {
                        auto e = MemoryManager::Make_shared<SwitchGBufferEvent>(selected_gbufferModes);
                        graphicDebugEventQueue->Publish(e);
                    }
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // Set IBL
                if (ImGui::TreeNode("Environment Mapping"))
                {
                    if (ImGui::Checkbox("Enable", &checkEvnMapping))
                    {
                        auto e = MemoryManager::Make_shared<SetEnvironmentMappingEvent>(
                            checkEvnMapping, valueCurrentEnvMapName);
                        graphicDebugEventQueue->Publish(e);
                    }
                    int selected_skybox = LongMarch_findFristIndex(valueAllEnvMapName, valueCurrentEnvMapName) + 1;
                    auto valueAllEnvMapName_char = LongMarch_StrVec2ConstChar(valueAllEnvMapName);
                    valueAllEnvMapName_char.insert(valueAllEnvMapName_char.begin(), "");
                    if (ImGui::Combo("Skyboxes", &selected_skybox, &valueAllEnvMapName_char[0],
                                     valueAllEnvMapName_char.size()))
                    {
                        valueCurrentEnvMapName = valueAllEnvMapName[selected_skybox];
                        auto e = MemoryManager::Make_shared<SetEnvironmentMappingEvent>(
                            checkEvnMapping, valueCurrentEnvMapName);
                        graphicDebugEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // Toggle shadow
                {
                    if (ImGui::Checkbox("Shadow", &checkShadow))
                    {
                        auto e = MemoryManager::Make_shared<ToggleShadowEvent>(checkShadow);
                        graphicDebugEventQueue->Publish(e);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Post processing"))
            {
                constexpr int yoffset_item = 5;
                // Motion blur
                if (ImGui::TreeNode("Motion Blur"))
                {
                    if (ImGui::Checkbox("Enable", &checkMotionBlur))
                    {
                        auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(
                            checkMotionBlur, valueMotionblurShutterSpeed);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Moltion Blur Shutter Speed", &valueMotionblurShutterSpeed, 15, 90))
                    {
                        auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(
                            checkMotionBlur, valueMotionblurShutterSpeed);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // SSGI
                if (ImGui::TreeNode("SSGI"))
                {
                    {
                        float Ia = Renderer3D::s_Data.ambient;
                        if (ImGui::DragFloat("Ia", &Ia, 0.01, 0, 1, "%.2f"))
                        {
                            Renderer3D::s_Data.ambient = Ia;
                        }
                    }
                    if (ImGui::Checkbox("Enable", &checkSSGI))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Resolution Down Scale", &valueSSGISampleResDownScale, 1, 4, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Samples", &valueSSGISample, 5, 80, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Gauss Kernel", &valueSSGIBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN,
                                         LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
                    if (ImGui::SliderFloat("Sample Radius", &valueSSGISampleRadius, 0.01f, 20.0f, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderFloat("Strength", &valueSSGIStrength, 0.1f, 10.0f, "%.1f"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(
                            checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel,
                            valueSSGISampleRadius, valueSSGIStrength);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // SSAO
                if (ImGui::TreeNode("SSAO"))
                {
                    if (ImGui::Checkbox("Enable", &checkSSAO))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Resolution Down Scale", &valueSSAOSampleResDownScale, 1, 4, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Samples", &valueSSAOSample, 5, 80, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Gauss Kernel", &valueSSAOBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN,
                                         LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
                    if (ImGui::SliderFloat("Sample Radius", &valueSSAOSampleRadius, 0.01f, 20.0f, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderFloat("Scale", &valueSSAOScale, 0.1f, 10.0f, "%.1f"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderFloat("Power", &valueSSAOPower, 0.1f, 10.0f, "%.1f"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(
                            checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel,
                            valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // SSR
                if (ImGui::TreeNode("SSR"))
                {
                    if (ImGui::Checkbox("Enable", &checkSSR))
                    {
                        auto e = MemoryManager::Make_shared<SetSSRValueEvent>(
                            checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Resolution Down Scale", &valueSSRSampleResDownScale, 1, 4, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSRValueEvent>(
                            checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Gauss Kernel", &valueSSRBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN,
                                         LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetSSRValueEvent>(
                            checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
                    if (ImGui::Checkbox("Debug", &checkSSRDebug))
                    {
                        auto e = MemoryManager::Make_shared<SetSSRValueEvent>(
                            checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // Bloom
                if (ImGui::TreeNode("Bloom"))
                {
                    if (ImGui::Checkbox("Enable", &checkBloom))
                    {
                        auto e = MemoryManager::Make_shared<SetBloomEvent>(
                            checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel,
                            valueBloomSampleResDownScale);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Resolution Down Scale", &valueBloomSampleResDownScale, 1, 4, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetBloomEvent>(
                            checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel,
                            valueBloomSampleResDownScale);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Gauss Kernel", &valueBloomBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN,
                                         LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetBloomEvent>(
                            checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel,
                            valueBloomSampleResDownScale);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
                    if (ImGui::DragFloat("Threshold", &valueBloomThreshold, 0.05, -60, 1, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetBloomEvent>(
                            checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel,
                            valueBloomSampleResDownScale);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Negative value serves as the power of a exponent s-curve in [-40,0], the greater this value, the greater the bloom coverage. Positive value serves as the cutoff value of a step function in [0,1], the the lower this value, the greater the bloom coverage ");
                    if (ImGui::DragFloat("Strength", &valueBloomStrength, 0.01, 0, 1, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetBloomEvent>(
                            checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel,
                            valueBloomSampleResDownScale);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // DOF
                if (ImGui::TreeNode("Depth of Field"))
                {
                    if (ImGui::Checkbox("Enable", &checkDOF))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Resolution Down Scale", &valueDOFSampleResDownScale, 1, 4, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderInt("Gauss Kernel", &valueDOFBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN,
                                         LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    ImGuiUtil::InlineHelpMarker(
                        "Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
                    if (ImGui::DragFloat("Threshold", &valueDOFThreshold, 0.1, 0.01, 100, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::DragFloat("Blend Strength", &valueDOFStrength, 0.01, 0.01, 1, "%.2f"))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::DragFloat("Refocus Speed", &valueDOFRefocusRate, 0.1, 1, 60, "%.1f"))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::Checkbox("Debug", &checkDOFDebug))
                    {
                        auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength,
                                                                        valueDOFBlurKernel, valueDOFSampleResDownScale,
                                                                        valueDOFRefocusRate, checkDOFDebug);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // TAA
                if (ImGui::TreeNode("TAA"))
                {
                    if (ImGui::Checkbox("Enable", &checkTAA))
                    {
                        auto e = MemoryManager::Make_shared<ToggleTAAEvent>(checkTAA);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // SMAA
                if (ImGui::TreeNode("SMAA"))
                {
                    if (ImGui::Checkbox("Enable", &checkSMAA))
                    {
                        auto e = MemoryManager::Make_shared<ToggleSMAAEvent>(checkSMAA, valueSMAAMode);
                        graphicEventQueue->Publish(e);
                    }
                    static const char* smaa_modes[] = {"SMAA 1X", "SMAA T2X"};
                    if (ImGui::Combo("Method", &valueSMAAMode, smaa_modes, IM_ARRAYSIZE(smaa_modes)))
                    {
                        auto e = MemoryManager::Make_shared<ToggleSMAAEvent>(checkSMAA, valueSMAAMode);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // FXAA
                if (ImGui::TreeNode("FXAA"))
                {
                    if (ImGui::Checkbox("Enable", &checkFXAA))
                    {
                        auto e = MemoryManager::Make_shared<ToggleFXAAEvent>(checkFXAA);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, yoffset_item));
                // Tone mapping
                if (ImGui::TreeNode("Tone Mapping"))
                {
                    if (ImGui::Combo("Method", &selected_toneMap, toneMapModes, IM_ARRAYSIZE(toneMapModes)))
                    {
                        auto e = MemoryManager::Make_shared<SwitchToneMappingEvent>(selected_toneMap);
                        graphicEventQueue->Publish(e);
                    }
                    if (ImGui::SliderFloat("Gamma", &valueGamma, 0.1f, 5.0f, "%.1f"))
                    {
                        auto e = MemoryManager::Make_shared<SetGammaValueEvent>(valueGamma);
                        graphicEventQueue->Publish(e);
                    }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::Separator();
        ImGui::TreePop();
    }
}

void longmarch::Renderer3D::_ON_SET_ENV_MAPPING(EventQueue<EngineGraphicsDebugEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetEnvironmentMappingEvent>(e);
    s_Data.EnvMapping.enable_env_mapping = event->m_enable;
    s_Data.EnvMapping.CurrentEnvMapName = event->m_currentEnvMap;
}

void longmarch::Renderer3D::_ON_TOGGLE_SHADOW(EventQueue<EngineGraphicsDebugEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<ToggleShadowEvent>(e);
    s_Data.enable_shadow = event->m_enable;
}

void longmarch::Renderer3D::_ON_SWITCH_GBUFFER_MODE(EventQueue<EngineGraphicsDebugEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SwitchGBufferEvent>(e);
    s_Data.gBuffer_display_mode = event->m_value;
}

void longmarch::Renderer3D::_ON_TOGGLE_MOTION_BLUR(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<ToggleMotionBlurEvent>(e);
    s_Data.MotionBlur.enable_motionblur = event->m_enable;
    s_Data.MotionBlur.motionblur_shutterSpeed = event->m_shutterSpeed;
}

void longmarch::Renderer3D::_ON_TOGGLE_TAA(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<ToggleTAAEvent>(e);
    s_Data.enable_taa = event->m_enable;
}

void longmarch::Renderer3D::_ON_TOGGLE_FXAA(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<ToggleFXAAEvent>(e);
    s_Data.enable_fxaa = event->m_enable;
}

void longmarch::Renderer3D::_ON_TOGGLE_SMAA(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<ToggleSMAAEvent>(e);
    s_Data.SMAASettings.enable = event->m_enable;
    s_Data.SMAASettings.mode = event->m_mode;
}

void longmarch::Renderer3D::_ON_SWITCH_TONEMAP_MODE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SwitchToneMappingEvent>(e);
    s_Data.toneMapping_mode = event->m_value;
}

void longmarch::Renderer3D::_ON_SET_GAMMA_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetGammaValueEvent>(e);
    s_Data.value_gamma = event->m_value;
}

void longmarch::Renderer3D::_ON_SET_SSGI_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetSSGIValueEvent>(e);
    s_Data.SSGISettings.enable = event->m_enable;
    s_Data.SSGISettings.gi_gaussian_kernal = event->m_gaussKernel;
    s_Data.SSGISettings.gi_gaussian_kernal = (glm::clamp)(s_Data.SSGISettings.gi_gaussian_kernal,
                                                          LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
    s_Data.SSGISettings.gi_samples = event->m_sample;
    s_Data.SSGISettings.gi_sample_radius = event->m_sampleRadius;
    s_Data.SSGISettings.gi_sample_resolution_downScale = event->m_sampleResolutionDownScale;
    s_Data.SSGISettings.gi_strength = event->m_strength;
}

void longmarch::Renderer3D::_ON_SET_SSAO_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetSSAOValueEvent>(e);
    s_Data.SSAOSettings.enable = event->m_enable;
    s_Data.SSAOSettings.ao_gaussian_kernal = event->m_gaussKernel;
    s_Data.SSAOSettings.ao_gaussian_kernal = (glm::clamp)(s_Data.SSAOSettings.ao_gaussian_kernal,
                                                          LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
    s_Data.SSAOSettings.ao_samples = event->m_sample;
    s_Data.SSAOSettings.ao_sample_radius = event->m_sampleRadius;
    s_Data.SSAOSettings.ao_sample_resolution_downScale = event->m_sampleResolutionDownScale;
    s_Data.SSAOSettings.ao_scale = event->m_scale;
    s_Data.SSAOSettings.ao_power = event->m_power;
}

void longmarch::Renderer3D::_ON_SET_SSR_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetSSRValueEvent>(e);
    s_Data.SSRSettings.enable = event->m_enable;
    s_Data.SSRSettings.ssr_gaussian_kernal = event->m_gaussKernel;
    s_Data.SSRSettings.ssr_gaussian_kernal = (glm::clamp)(s_Data.SSRSettings.ssr_gaussian_kernal,
                                                          LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
    s_Data.SSRSettings.ssr_sample_resolution_downScale = event->m_sampleResolutionDownScale;
    s_Data.SSRSettings.enable_debug = event->m_debug;
}

void longmarch::Renderer3D::_ON_SET_BLOOM_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetBloomEvent>(e);
    s_Data.BloomSettings.enable = event->m_enable;
    s_Data.BloomSettings.bloom_gaussian_kernal = event->m_gaussKernel;
    s_Data.BloomSettings.bloom_gaussian_kernal = (glm::clamp)(s_Data.BloomSettings.bloom_gaussian_kernal,
                                                              LongMarch_GUASSIAN_KERNEL_MIN,
                                                              LongMarch_GUASSIAN_KERNEL_MAX);
    s_Data.BloomSettings.bloom_threshold = event->m_threshold;
    s_Data.BloomSettings.bloom_blend_strength = event->m_strength;
    s_Data.BloomSettings.bloom_sample_resolution_downScale = event->m_sampleResolutionDownScale;
}

void longmarch::Renderer3D::_ON_SET_DOF_VALUE(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetDOFvent>(e);
    s_Data.DOFSettings.enable = event->m_enable;
    s_Data.DOFSettings.dof_gaussian_kernal = event->m_gaussKernel;
    s_Data.DOFSettings.dof_gaussian_kernal = (glm::clamp)(s_Data.DOFSettings.dof_gaussian_kernal,
                                                          LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX);
    s_Data.DOFSettings.dof_threshold = event->m_threshold;
    s_Data.DOFSettings.dof_blend_strength = event->m_strength;
    s_Data.DOFSettings.dof_sample_resolution_downScale = event->m_sampleResolutionDownScale;
    s_Data.DOFSettings.dof_refocus_rate = event->m_refocusRate;
    s_Data.DOFSettings.enable_debug = event->m_debug;
}

void longmarch::Renderer3D::_ON_SET_DOF_TARGET(EventQueue<EngineGraphicsEventType>::EventPtr e)
{
    auto event = std::static_pointer_cast<SetDOFTarget>(e);
    s_Data.DOFSettings.use_ss_target = event->m_bUseScreenSpaceTarget;
    s_Data.DOFSettings.ss_target = event->m_ssTarget;
    s_Data.DOFSettings.use_ws_target = event->m_bUseWorldSPaceTarget;
    s_Data.DOFSettings.ws_target = event->m_wsTarget;
}

/**************************************************************
*	Render3D highlevel API
*
**************************************************************/

bool longmarch::Renderer3D::ShouldRendering()
{
    const auto& prop = Engine::GetWindow()->GetWindowProperties();
    if (glm::any(glm::equal(Vec2u(prop.m_width, prop.m_height), Vec2u(0u))))
    {
        // Early quit on, say, minimizing window
        return false;
    }
    return true;
}

/**************************************************************
*	Render3D highlevel API : BeginRendering
*
**************************************************************/
void longmarch::Renderer3D::BeginRendering(const PerspectiveCamera* camera)
{
    {
        // Update viewport size
        const auto& prop = Engine::GetWindow()->GetWindowProperties();
        s_Data.window_size = Vec2u(prop.m_width, prop.m_height);
        s_Data.resolution = Vec2u(Vec2f(camera->cameraSettings.viewportSize) * s_Data.resolution_ratio);
        s_Data.window_size_changed_this_frame = false;
    }
    {
        // Count frame index oddity
        s_Data.frameIndex = (s_Data.frameIndex + 1) % 2;
    }

    // Prepare buffers
    {
        // Resize FrameBuffer if necessary
        if (s_Data.gpuBuffer.PrevFinalFrameBuffer->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.PrevFinalFrameBuffer = FrameBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                        FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }
        if (s_Data.gpuBuffer.CurrentFinalFrameBuffer->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.CurrentFinalFrameBuffer = FrameBuffer::Create(
                s_Data.resolution.x, s_Data.resolution.y, FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }
        if (s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer = FrameBuffer::Create(
                s_Data.resolution.x, s_Data.resolution.y, FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        if (s_Data.gpuBuffer.FrameBuffer_1->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.FrameBuffer_1 = FrameBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                 FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }
        if (s_Data.gpuBuffer.FrameBuffer_2->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.FrameBuffer_2 = FrameBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                 FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }
        if (s_Data.gpuBuffer.FrameBuffer_3->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.FrameBuffer_3 = FrameBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                 FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }
        if (s_Data.gpuBuffer.FrameBuffer_4->GetBufferSize() != s_Data.resolution)
        {
            s_Data.window_size_changed_this_frame = true;
            s_Data.gpuBuffer.FrameBuffer_4 = FrameBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                 FrameBuffer::BUFFER_FORMAT::FLOAT_RGBA16);
        }

        // Enable depth testing and wirte to depth buffer just for the clearing commend
        RenderCommand::DepthTest(true, true);
        RenderCommand::StencilTest(true, true);
        RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));

        // CRITICAL : Don't clear prev frame buffer as we need it in the current frame
        /*s_Data.gpuBuffer.PrevFinalFrameBuffer->Bind();
        RenderCommand::Clear();*/

        // CRITICAL : Don't clear prev frame buffer as we need it in the current frame
        /*s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->Bind();
        RenderCommand::Clear();*/

        s_Data.gpuBuffer.CurrentFinalFrameBuffer->Bind();
        RenderCommand::Clear();

        s_Data.gpuBuffer.FrameBuffer_1->Bind();
        RenderCommand::Clear();

        s_Data.gpuBuffer.FrameBuffer_2->Bind();
        RenderCommand::Clear();

        s_Data.gpuBuffer.FrameBuffer_3->Bind();
        RenderCommand::Clear();

        s_Data.gpuBuffer.FrameBuffer_4->Bind();
        RenderCommand::Clear();

        if (s_Data.RENDER_PIPE == RENDER_PIPE::DEFERRED)
        {
            // Resize GBuffer if necessary
            if (s_Data.gpuBuffer.CurrentGBuffer->GetBufferSize() != s_Data.resolution)
            {
                s_Data.gpuBuffer.CurrentGBuffer = GBuffer::Create(s_Data.resolution.x, s_Data.resolution.y,
                                                                  GBuffer::GBUFFER_TYPE::DEFAULT);
            }
            // CRITICAL GBuffer should be cleared with all zeros
            RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));
            s_Data.gpuBuffer.CurrentGBuffer->Bind();
            RenderCommand::Clear();
        }

        RenderCommand::DepthTest(false, false);
        RenderCommand::StencilTest(false, false);
    }

    {
        // Assign current frame buffer
        s_Data.gpuBuffer.CurrentFrameBuffer = s_Data.gpuBuffer.FrameBuffer_1;

        // Transfer cleared fbo to default frame buffer in order to clear it as well.
        constexpr int default_framebuffer_rendererID = 0;
        RenderCommand::TransferColorBit(
            s_Data.gpuBuffer.CurrentFrameBuffer->GetFrameBufferID(),
            s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().x,
            s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().y,

            default_framebuffer_rendererID,
            s_Data.window_size.x,
            s_Data.window_size.y
        );
    }

    if (s_Data.DOFSettings.enable)
    {
        // DOF effect calculate depth
        if (s_Data.DOFSettings.use_ss_target)
        {
            // Do nothing since we will use screen space coord (in range [-1,1])
            auto input = InputManager::GetInstance();
            auto cursor_pos = input->GetCursorPositionXY();
            camera->CursorSpaceToScreenSpace(cursor_pos, true, true, s_Data.DOFSettings.ss_target);
        }
        else if (s_Data.DOFSettings.use_ws_target)
        {
            // Convert world space to screen space
            camera->WorldSpaceToScreenSpace(s_Data.DOFSettings.ws_target, s_Data.DOFSettings.ss_target);
        }
    }

    // Populate shading parameters
    {
        ENG_TIME("Scene phase (Opaque): Populate Shading Data");
        _PopulateShadingPassUniformsVariables(camera);
    }
}

/**************************************************************
*	Render3D highlevel API : BeginOpaqueShadowing
*
**************************************************************/
void longmarch::Renderer3D::BeginShadowing(
    const PerspectiveCamera* camera,
    const std::function<void()>& f_render_opaque,
    const std::function<void()>& f_render_trasparent,
    const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
    const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
    const std::function<void(const std::string&)>& f_setRenderShaderName)
{
    auto render_pipe_original = s_Data.RENDER_PIPE;
    auto render_mode_original = s_Data.RENDER_MODE;

    s_Data.RENDER_PASS = RENDER_PASS::SHADOW;
    RenderCommand::PolyModeFill();
    RenderCommand::Blend(false);
    RenderCommand::DepthTest(true, true);
    RenderCommand::CullFace(true, false);

    static auto BeginGaussianBlur = []()
    {
        RenderCommand::DepthTest(false, false);
        RenderCommand::CullFace(false, false);
    };

    static auto EndGaussianBlur = []()
    {
        RenderCommand::DepthTest(true, true);
        RenderCommand::CullFace(true, false);
    };

    static auto BeginTransparentShadow = []()
    {
        s_Data.RENDER_PIPE = RENDER_PIPE::FORWARD;
        s_Data.RENDER_MODE = RENDER_MODE::CANONICAL;
        // TODO : implement transparent shadow with transparent color
        /*RenderCommand::Blend(true);
        RenderCommand::BlendFunc(RendererAPI::BlendFuncEnum::MULTIPLICATION);
        RenderCommand::DepthTest(true, false);
        RenderCommand::CullFace(false, false);*/
    };

    auto EndTransparentShadow = [render_pipe_original, render_mode_original]()
    {
        s_Data.RENDER_PIPE = render_pipe_original;
        s_Data.RENDER_MODE = render_mode_original;
        // TODO : implement transparent shadow with transparent color
        /*RenderCommand::Blend(false);
        RenderCommand::DepthTest(true, true);
        RenderCommand::CullFace(true, false);*/
    };

    static auto BeginDirectionLight = []()
    {
        RenderCommand::DepthClamp(true);
        Renderer3D::ToggleReverseZ(false);
    };

    static auto EndDirectionLight = []()
    {
        RenderCommand::DepthClamp(false);
        Renderer3D::ToggleReverseZ(true);
    };

    static auto CSMSplitPlaneHelper = [](float vf_near, float vf_far, float f_i, float f_num_CSM, float lambda) -> float
    {
        return LongMarch_Lerp(vf_near * powf(vf_far / vf_near, f_i / f_num_CSM),
                              (vf_near + (f_i / f_num_CSM) * (vf_far - vf_near)), lambda);
    };

    static auto OvercomeShadowShimmering = [](float shadowSize, const Mat4& ShadowP, const Mat4& ShadowV) -> Mat4
    {
        Mat4 newShadowP = ShadowP;
        // Overcoming shadow shimmering
        // Reference : https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
        // Reference : https://www.youtube.com/watch?v=PxbGUOC_UeA
        const auto shadowMatrix = ShadowP * ShadowV;
        auto shadowOrigin = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
        shadowOrigin = shadowMatrix * shadowOrigin;
        shadowOrigin = shadowOrigin * shadowSize * 0.5f;
        const auto roundedOrigin = glm::round(shadowOrigin);
        auto roundOffset = roundedOrigin - shadowOrigin;
        roundOffset = roundOffset * 2.0f / shadowSize;
        roundOffset.z = 0.0f;
        roundOffset.w = 0.0f;
        newShadowP[3] += roundOffset;
        return newShadowP;
    };

    static auto CreateDirectionalLight = [](const LightBuffer_CPU& currentLight) -> DirectionalLightBuffer_GPU
    {
        DirectionalLightBuffer_GPU light;
        light.Pos_shadowMapIndex.xyz = currentLight.Pos;
        light.Kd_shadowMatrixIndex.xyz = currentLight.Kd;
        light.Dir = Geommath::ToVec4(currentLight.Direction);
        light.Attenuation = currentLight.Attenuation;
        return light;
    };
    static auto CreatePointLight = [](const LightBuffer_CPU& currentLight) -> PointLightBuffer_GPU
    {
        PointLightBuffer_GPU light;
        light.Pos_shadowMapIndex.xyz = currentLight.Pos;
        light.Kd_shadowMatrixIndex.xyz = currentLight.Kd;
        light.Attenuation = currentLight.Attenuation;
        return light;
    };
    static auto CreateSpotLight = [](const LightBuffer_CPU& currentLight) -> SpotLightBuffer_GPU
    {
        SpotLightBuffer_GPU light;
        light.Pos_shadowMapIndex.xyz = currentLight.Pos;
        light.Kd_shadowMatrixIndex.xyz = currentLight.Kd;
        light.Dir = Geommath::ToVec4(currentLight.Direction);
        light.Attenuation = currentLight.Attenuation;
        return light;
    };


    static auto GetShadowBuffer_Opaque = [](const ComponentDecorator<LightCom>& lightCom,
                                            std::shared_ptr<Shader>& shadow_shader, std::string& shadow_shader_name)
    {
        if (lightCom->shadow.shadowAlgorithmMode <= 3)
        {
            shadow_shader_name = "ShadowBuffer";
            shadow_shader = s_Data.ShaderMap["ShadowBuffer"];
        }
        else
        {
            shadow_shader_name = "MSMShadowBuffer";
            shadow_shader = s_Data.ShaderMap["MSMShadowBuffer"];
        }
    };

    static auto GetShadowBuffer_Particle = [](const ComponentDecorator<LightCom>& lightCom,
                                              std::shared_ptr<Shader>& shadow_shader, std::string& shadow_shader_name)
    {
        if (lightCom->shadow.shadowAlgorithmMode <= 3)
        {
            shadow_shader_name = "ShadowBuffer_Particle";
            shadow_shader = s_Data.ShaderMap["ShadowBuffer_Particle"];
        }
        else
        {
            shadow_shader_name = "MSMShadowBuffer_Particle";
            shadow_shader = s_Data.ShaderMap["MSMShadowBuffer_Particle"];
        }
    };

    static auto GetShadowBuffer_Transparent = [](const ComponentDecorator<LightCom>& lightCom,
                                                 std::shared_ptr<Shader>& shadow_shader,
                                                 std::string& shadow_shader_name)
    {
        if (lightCom->shadow.shadowAlgorithmMode <= 3)
        {
            shadow_shader_name = "ShadowBuffer_Transparent";
            shadow_shader = s_Data.ShaderMap["ShadowBuffer_Transparent"];
        }
        else
        {
            shadow_shader_name = "MSMShadowBuffer_Transparent";
            shadow_shader = s_Data.ShaderMap["MSMShadowBuffer_Transparent"];
        }
    };
    //const auto& msm_cube_shader = s_Data.ShaderMap["MSMShadowBuffer_Cube"]; // used when using cubic shadow map for point lights

    const auto& transparent_shader = s_Data.ShaderMap["TransparentForwardShader"];

    const auto& guassian_shader = s_Data.ShaderMap["GaussianBlur"];
    const auto& guassian_CSM_shader = s_Data.ShaderMap["GaussianBlur_CSM"];
    const auto& guassian_cube_shader = s_Data.ShaderMap["GaussianBlur_Cube_PointLight"];

    // Reset buffers
    {
        s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED.clear();
        s_Data.cpuBuffer.POINT_LIGHT_PROCESSED.clear();
        s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED.clear();
        s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.clear();

        s_Data.gpuBuffer.DirectionalLightShadowBuffer.clear();
        s_Data.gpuBuffer.PointLightShadowBuffer.clear();
        s_Data.gpuBuffer.SpotLightShadowBuffer.clear();
    }

    {
        ENG_TIME("Shadow phase: LOOPING");
        s_Data.NUM_LIGHT = MIN(s_Data.cpuBuffer.LIGHTS_BUFFERED.size(), s_Data.MAX_LIGHT);
        for (auto i(0u); i < s_Data.NUM_LIGHT; ++i)
        {
            LightBuffer_CPU currentLight = s_Data.cpuBuffer.LIGHTS_BUFFERED[i];
            auto lightCom = GameWorld::GetCurrent()->GetComponent<LightCom>(currentLight.thisLight);
            auto sceneCom = GameWorld::GetCurrent()->GetComponent<Scene3DCom>(currentLight.thisLight);
            if (!lightCom.Valid() || !sceneCom.Valid())
            {
                continue;
            }

            // Branching by the type of lights
            switch (currentLight.type)
            {
            //0 - directional light
            case LongMarch_ToUnderlying(LIGHT_TYPE::DIRECTIONAL):
                {
                    ENG_TIME("Shadow phase: DIRECTIONAL");
                    BeginDirectionLight();
                    const float vf_near = MAX(camera->cameraSettings.nearZ, 0.1f);
                    const float vf_far = lightCom->shadow.farZ;
                    const auto& p = (s_Data.enable_reverse_z)
                                        ? Geommath::ReverseZProjectionMatrixZeroOne(
                                            camera->cameraSettings.fovy_rad, camera->cameraSettings.aspectRatioWbyH,
                                            vf_near, vf_far)
                                        : Geommath::ProjectionMatrixZeroOne(
                                            camera->cameraSettings.fovy_rad, camera->cameraSettings.aspectRatioWbyH,
                                            vf_near, vf_far);
                    const auto& v = camera->GetViewMatrix();
                    const auto& pv = p * v;
                    const auto& foyz = camera->cameraSettings.fovy_rad;
                    const auto& aspect = camera->cameraSettings.aspectRatioWbyH;
                    const auto& lambda = lightCom->directionalLight.lambdaCSM;
                    const auto& num_CSM = lightCom->directionalLight.numOfCSM;

                    // Calculate camera view frustum centroid
                    Vec3f world_NDCCentroid;
                    auto dummy = ViewFrustumCorners();
                    Geommath::Frustum::GetCornersAndCentroid(pv, s_Data.enable_reverse_z, dummy, world_NDCCentroid);

                    // Move the directional light object
                    const auto& light_dir = currentLight.Direction;
                    const auto& light_pos_tr = (vf_far - vf_near) * (light_dir * Geommath::WorldFront);
                    const auto& light_pos = (light_pos_tr + world_NDCCentroid);

                    // Move the directional light placeholder mesh to its final position (Optional)
                    // It does not affect the direct lighting calculation
                    /*auto trans = GameWorld::GetCurrent()->GetComponent<Transform3DCom>(currentLight.thisLight);
                    trans->SetGlobalPos(light_pos);*/

                    currentLight.Pos = light_pos;
                    currentLight.castShadow = lightCom->shadow.bCastShadow && s_Data.enable_shadow;

                    // Create the gpu light buffer from cpu light buffer
                    auto currentDirectionalLight = CreateDirectionalLight(currentLight);
                    {
                        currentDirectionalLight.numCSM_lambda_near_far.x = num_CSM;
                        currentDirectionalLight.numCSM_lambda_near_far.y = lambda;
                        currentDirectionalLight.numCSM_lambda_near_far.z = vf_near;
                        currentDirectionalLight.numCSM_lambda_near_far.w = vf_far;
                    }

                    // Build shadow
                    if (!currentLight.castShadow)
                    {
                        currentDirectionalLight.Kd_shadowMatrixIndex.w = -1;
                        // set to -1 to indicate not casting shadow that will be checked in the lighting shader
                    }
                    else
                    {
                        // Allocate shadow map
                        lightCom->AllocateShadowBuffer();
                        const auto& shadowBuffer = lightCom->shadow.shadowBuffer;
                        Vec2u traget_resoluation = shadowBuffer->GetBufferSize();

                        for (auto i(0u); i < num_CSM; ++i)
                        {
                            // Build directional's orthographic camera from the corners of the view camera
                            float split_Near = CSMSplitPlaneHelper(vf_near, vf_far, i, num_CSM, lambda);
                            float split_Far = CSMSplitPlaneHelper(vf_near, vf_far, i + 1, num_CSM, lambda);

                            const auto& p = (s_Data.enable_reverse_z)
                                                ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                    foyz, aspect, split_Near, split_Far)
                                                : Geommath::ProjectionMatrixZeroOne(
                                                    foyz, aspect, split_Near, split_Far);
                            const auto& pv = p * v;

                            // Since directional light does not have a world position,
                            // we calculate the pesudo light view matrix for directional light
                            Vec3f split_world_NDCCentroid;
                            ViewFrustumCorners split_world_NDCCorners;
                            Geommath::Frustum::GetCornersAndCentroid(pv, s_Data.enable_reverse_z,
                                                                     split_world_NDCCorners, split_world_NDCCentroid);
                            const auto& light_pos_tr = (light_dir * Geommath::WorldFront);
                            const auto& light_pos = (light_pos_tr + split_world_NDCCentroid);
                            const auto& look_at_pos = split_world_NDCCentroid;
                            const auto& light_view_mat = Geommath::LookAtWorld(light_pos, look_at_pos);

                            // Find view camera's frustum conrners in light's view space for the the ortho bounding
                            Vec3f Min((std::numeric_limits<float>::max)());
                            Vec3f Max((std::numeric_limits<float>::lowest)());
                            for (int i = 0; i < 8; ++i)
                            {
                                const auto& conrer = Geommath::Mat4ProdVec3(light_view_mat, split_world_NDCCorners[i]);
                                Min = (glm::min)(Min, conrer);
                                Max = (glm::max)(Max, conrer);
                            }
                            // Since OpenGL view space Z-positive direction is pointing into the camera, we need to negate the z value to find the near and far plane
                            float Near = -Max.z;
                            float Far = -Min.z;

                            // Build a loosely bounded projection matrix for clipping test
                            auto light_projection_mat_clipping = (s_Data.enable_reverse_z)
                                                                     ? Geommath::OrthogonalReverseZProjectionMatrixZeroOne(
                                                                         Min.x, Max.x, Min.y, Max.y, Near - vf_far, Far)
                                                                     : Geommath::OrthogonalProjectionMatrixZeroOne(
                                                                         Min.x, Max.x, Min.y, Max.y, Near - vf_far,
                                                                         Far);
                            auto clipping_vf = Geommath::Frustum::FromProjection(light_projection_mat_clipping);

                            // Build a tightly bounded projection matrix for shadow mapping
                            auto light_projection_mat = (s_Data.enable_reverse_z)
                                                            ? Geommath::OrthogonalReverseZProjectionMatrixZeroOne(
                                                                Min.x, Max.x, Min.y, Max.y, Near, Far)
                                                            : Geommath::OrthogonalProjectionMatrixZeroOne(
                                                                Min.x, Max.x, Min.y, Max.y, Near, Far);
                            light_projection_mat = OvercomeShadowShimmering(
                                lightCom->shadow.dimension, light_projection_mat, light_view_mat);
                            const auto& light_pv = light_projection_mat * light_view_mat;

                            // Update shadow matrix
                            {
                                ShadowData_GPU data;
                                data.PVMatrix = light_pv;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.x = lightCom->
                                    shadow.shadowAlgorithmMode;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.y = lightCom->
                                    shadow.depthBiasHigherBound;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.z = lightCom->
                                    shadow.depthBiasMultiplier;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.w = lightCom->
                                    shadow.nrmBiasMultiplier;
                                s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.emplace_back(std::move(data));
                                currentDirectionalLight.shadowMatrixIndcies[i] = s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.
                                    size() - 1;
                            }

                            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                            // Bind shadow buffer
                            shadowBuffer->Bind();
                            // Bind shadow map ith array layer
                            shadowBuffer->BindLayer(i);
                            // Clear buffer
                            (s_Data.enable_reverse_z)
                                ? RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0))
                                : RenderCommand::SetClearColor(Vec4f(1, 1, 1, 0));
                            RenderCommand::Clear();

                            std::string shader_name;
                            std::shared_ptr<Shader> shader;
                            GetShadowBuffer_Opaque(lightCom, shader, shader_name);
                            s_Data.CurrentShader = shader;
                            s_Data.CurrentShader->Bind();
                            s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                            s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                            sceneCom->SetShouldDraw(false, true);
                            // Render the scene
                            {
                                ENG_TIME("Shadow phase: DIRECTIONAL LOOPING (opaque)");
                                f_setRenderShaderName(shader_name);
                                f_setVFCullingParam(true, clipping_vf, light_view_mat);
                                f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                                f_render_opaque();
                            }
                            {
                                ENG_TIME("Shadow phase: DIRECTIONAL BATCH RENDER (opaque)");
                                CommitBatchRendering();
                            }
                            if (lightCom->shadow.bEnableTransparentShadow)
                            {
                                BeginTransparentShadow();
                                {
                                    ENG_TIME("Shadow phase: DIRECTIONAL LOOPING (transparent)");
                                    GetShadowBuffer_Particle(lightCom, shader, shader_name);
                                    s_Data.CurrentShader = shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetMat4("u_ProjectionMatrix", light_projection_mat);
                                    s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                    GetShadowBuffer_Transparent(lightCom, shader, shader_name);
                                    s_Data.CurrentShader = shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                                    s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                    f_setRenderShaderName(shader_name);
                                    f_setVFCullingParam(true, clipping_vf, light_view_mat);
                                    f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                                    f_render_trasparent();
                                }
                                EndTransparentShadow();

                                // TODO : implement transparent shadow with transparent color
                                //const auto& shadowBuffer3 = lightCom->shadow.shadowBuffer3;
                                //Vec2u traget_resoluation3 = shadowBuffer3->GetBufferSize();
                                //RenderCommand::SetViewport(0, 0, traget_resoluation3.x, traget_resoluation3.y);
                                //// Bind shadow buffer
                                //shadowBuffer3->Bind();
                                //// Bind shadow map ith array layer
                                //shadowBuffer3->BindLayer(i);
                                //// Clear buffer
                                //RenderCommand::SetClearColor(Vec4f(1, 1, 1, 1));
                                //RenderCommand::Clear();
                                //RenderCommand::TransferDepthBit(
                                //	shadowBuffer->GetFrameBufferID(),
                                //	traget_resoluation.x,
                                //	traget_resoluation.y,

                                //	shadowBuffer3->GetFrameBufferID(),
                                //	traget_resoluation3.x,
                                //	traget_resoluation3.y
                                //);
                                //BeginTransparentShadow();
                                //{
                                //	s_Data.CurrentShader = transparent_shader;
                                //	s_Data.CurrentShader->Bind();
                                //	// Render the scene
                                //	{
                                //		ENG_TIME("Shadow phase: DIRECTIONAL LOOPING (transparent)");
                                //		f_setRenderShaderName("TransparentForwardShader");
                                //		f_setVFCullingParam(true, Geommath::Frustum::FromProjection(light_projection_mat), light_view_mat);
                                //		f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                                //		f_render_trasparent();
                                //	}
                                //}
                                //EndTransparentShadow();
                            }
                            if (lightCom->shadow.bEnableGaussianBlur)
                            {
                                const auto& shadowBuffer2 = lightCom->shadow.shadowBuffer2;
                                Vec2u traget_resoluation2 = shadowBuffer2->GetBufferSize();
                                auto kernel_size = lightCom->shadow.gaussianKernal;
                                auto [length, offsets, weights] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[
                                    kernel_size];
                                BeginGaussianBlur();
                                {
                                    s_Data.CurrentShader = guassian_CSM_shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetInt("u_layer", i);
                                    s_Data.CurrentShader->SetInt("u_length", length);
                                    weights->Bind(1);
                                    offsets->Bind(2);
                                    {
                                        RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                                        // Bind shadow buffer
                                        shadowBuffer2->Bind();
                                        shadowBuffer2->BindLayer(i);
                                        RenderCommand::Clear();
                                        s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                                        shadowBuffer->BindTexture(s_Data.fragTexture_0_slot);
                                        _RenderFullScreenQuad();
                                    }
                                    {
                                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                                        // Bind shadow buffer
                                        shadowBuffer->Bind();
                                        shadowBuffer->BindLayer(i);
                                        RenderCommand::Clear();
                                        s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                                        shadowBuffer2->BindTexture(s_Data.fragTexture_0_slot);
                                        _RenderFullScreenQuad();
                                    }
                                }
                                EndGaussianBlur();
                            }
                        }
                    }
                    s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED.emplace_back(std::move(currentDirectionalLight));
                    s_Data.gpuBuffer.DirectionalLightShadowBuffer.emplace_back(lightCom->shadow.shadowBuffer);
                    EndDirectionLight();
                }
                break;
            //1 - point light
            case LongMarch_ToUnderlying(LIGHT_TYPE::POINT):
                {
                    ENG_TIME("Shadow phase: POINT");
                    const auto& light_pos = currentLight.Pos;
                    const auto& radius = lightCom->pointLight.radius;

                    auto Max = light_pos + 1.42f * radius;
                    auto Min = light_pos - 1.42f * radius;
                    AABB approx_shape{Min, Max};
                    approx_shape.RenderShape();
                    bool culled = approx_shape.VFCTest(camera->GetViewFrustumInViewSpace(), camera->GetViewMatrix());
                    bool exceedDropOff = lightCom->HandleShadowBufferDropOff(
                        glm::distance(camera->GetWorldPosition(), light_pos));
                    currentLight.castShadow = lightCom->shadow.bCastShadow && s_Data.enable_shadow && !culled && !
                        exceedDropOff;

                    auto currentPointLight = CreatePointLight(currentLight);
                    {
                        currentPointLight.Radius_CollisionRadius_SoftEdgeRatio.x = radius;
                        currentPointLight.Radius_CollisionRadius_SoftEdgeRatio.y = lightCom->collisionRadius;
                        currentPointLight.Radius_CollisionRadius_SoftEdgeRatio.z = lightCom->pointLight.softEdgeRatio;
                    }

                    if (!currentLight.castShadow)
                    {
                        currentPointLight.Kd_shadowMatrixIndex.w = -1;
                        // set to -1 to indicate not casting shadow that will be checked in the lighting shader
                    }
                    else
                    {
                        // Build shadow
                        lightCom->AllocateShadowBuffer();
                        const auto& shadowBuffer = lightCom->shadow.shadowBuffer;
                        Vec2u traget_resoluation = shadowBuffer->GetBufferSize();

                        const auto& Near = lightCom->shadow.nearZ;
                        const auto& Far = lightCom->shadow.farZ;

                        // Array texture point light shadow map
                        for (int i = 0; i < 6; ++i)
                        {
                            const auto& light_view_mat = Geommath::LookAt(
                                light_pos, light_pos + s_Data.cube_directions[i].xyz, s_Data.cube_ups[i]);
                            const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                                   ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, Near, Far)
                                                                   : Geommath::ProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, Near, Far);
                            const auto& light_pv = light_projection_mat * light_view_mat;
                            auto clipping_vf = Geommath::Frustum::FromProjection(light_projection_mat);

                            // Update shadow matrix
                            {
                                ShadowData_GPU data;
                                data.PVMatrix = light_pv;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.x = lightCom->
                                    shadow.shadowAlgorithmMode;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.y = lightCom->
                                    shadow.depthBiasHigherBound;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.z = lightCom->
                                    shadow.depthBiasMultiplier;
                                data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.w = lightCom->
                                    shadow.nrmBiasMultiplier;
                                s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.emplace_back(std::move(data));
                                if (i < 3)
                                {
                                    // First three faces
                                    currentPointLight.shadowMatrixIndcies_1_2_3[i] = s_Data.cpuBuffer.
                                        SHADOW_DATA_PROCESSED.size() - 1;
                                }
                                else
                                {
                                    // Last three faces
                                    currentPointLight.shadowMatrixIndcies_4_5_6[i - 3] = s_Data.cpuBuffer.
                                        SHADOW_DATA_PROCESSED.size() - 1;
                                }
                            }

                            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                            // Bind shadow buffer
                            shadowBuffer->Bind();
                            // Bind shadow map ith array layer
                            shadowBuffer->BindLayer(i);
                            // Clear buffer
                            (s_Data.enable_reverse_z)
                                ? RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0))
                                : RenderCommand::SetClearColor(Vec4f(1, 1, 1, 0));
                            RenderCommand::Clear();

                            std::string shader_name;
                            std::shared_ptr<Shader> shader;
                            GetShadowBuffer_Opaque(lightCom, shader, shader_name);
                            s_Data.CurrentShader = shader;
                            s_Data.CurrentShader->Bind();
                            s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                            s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                            sceneCom->SetShouldDraw(false, true);
                            // Render the scene
                            {
                                ENG_TIME("Shadow phase: POINT LOOPING");
                                f_setRenderShaderName(shader_name);
                                f_setVFCullingParam(true, clipping_vf, light_view_mat);
                                f_setDistanceCullingParam(false, light_pos, Near, Far);
                                f_render_opaque();
                            }
                            {
                                ENG_TIME("Shadow phase: POINT BATCH RENDER");
                                CommitBatchRendering();
                            }
                            if (lightCom->shadow.bEnableTransparentShadow)
                            {
                                BeginTransparentShadow();
                                {
                                    ENG_TIME("Shadow phase: POINT LOOPING (transparent)");
                                    GetShadowBuffer_Particle(lightCom, shader, shader_name);
                                    s_Data.CurrentShader = shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetMat4("u_ProjectionMatrix", light_projection_mat);
                                    s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                    GetShadowBuffer_Transparent(lightCom, shader, shader_name);
                                    s_Data.CurrentShader = shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                                    s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                    f_setRenderShaderName(shader_name);
                                    f_setVFCullingParam(true, clipping_vf, light_view_mat);
                                    f_setDistanceCullingParam(true, light_pos, Near, Far);
                                    f_render_trasparent();
                                }
                                EndTransparentShadow();
                            }
                            if (lightCom->shadow.bEnableGaussianBlur)
                            {
                                const auto& shadowBuffer2 = lightCom->shadow.shadowBuffer2;
                                Vec2u traget_resoluation2 = shadowBuffer2->GetBufferSize();
                                auto kernel_size = lightCom->shadow.gaussianKernal;
                                auto [length, offsets, weights] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[
                                    kernel_size];
                                BeginGaussianBlur();
                                {
                                    s_Data.CurrentShader = guassian_CSM_shader;
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetInt("u_layer", i);
                                    s_Data.CurrentShader->SetInt("u_length", length);
                                    weights->Bind(1);
                                    offsets->Bind(2);
                                    {
                                        RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                                        // Bind shadow buffer
                                        shadowBuffer2->Bind();
                                        shadowBuffer2->BindLayer(i);
                                        RenderCommand::Clear();
                                        s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                                        shadowBuffer->BindTexture(s_Data.fragTexture_0_slot);
                                        _RenderFullScreenQuad();
                                    }
                                    {
                                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                                        // Bind shadow buffer
                                        shadowBuffer->Bind();
                                        shadowBuffer->BindLayer(i);
                                        RenderCommand::Clear();
                                        s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                                        shadowBuffer2->BindTexture(s_Data.fragTexture_0_slot);
                                        _RenderFullScreenQuad();
                                    }
                                }
                                EndGaussianBlur();
                            }
                        }
                    }
                    s_Data.cpuBuffer.POINT_LIGHT_PROCESSED.emplace_back(std::move(currentPointLight));
                    s_Data.gpuBuffer.PointLightShadowBuffer.emplace_back(lightCom->shadow.shadowBuffer);
                }
                break;
            //2 - spot light
            case LongMarch_ToUnderlying(LIGHT_TYPE::SPOT):
                {
                    ENG_TIME("Shadow phase: SPOT");
                    const auto fov_in = lightCom->spotLight.innerConeRad;
                    const auto fov_out = lightCom->spotLight.outterConeRad;

                    const auto& Near = lightCom->shadow.nearZ;
                    const auto& Far = lightCom->shadow.farZ;
                    const auto fov = fov_out + 5.0f * DEG2RAD;
                    const auto ratio = lightCom->spotLight.aspectWByH;
                    const auto& light_dir = currentLight.Direction;
                    const auto& light_pos_tr = -1.0f * (light_dir * Geommath::WorldFront);
                    const auto& light_pos = currentLight.Pos;
                    const auto& look_at_pos = (light_pos_tr + light_pos);
                    const auto& light_view_mat = Geommath::LookAtWorld(light_pos, look_at_pos);
                    const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                           ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                               fov, ratio, Near, Far)
                                                           : Geommath::ProjectionMatrixZeroOne(fov, ratio, Near, Far);
                    const auto& light_pv = light_projection_mat * light_view_mat;
                    auto clipping_vf = Geommath::Frustum::FromProjection(light_projection_mat);

                    Vec3f split_world_NDCCentroid;
                    ViewFrustumCorners split_world_NDCCorners;
                    Geommath::Frustum::GetCornersAndCentroid(light_pv, s_Data.enable_reverse_z, split_world_NDCCorners,
                                                             split_world_NDCCentroid);
                    // Find VF conrners in light's view space for the the shadow map bounding
                    Vec3f Min((std::numeric_limits<float>::max)());
                    Vec3f Max((std::numeric_limits<float>::lowest)());
                    for (int i = 0; i < 8; ++i)
                    {
                        const auto& conrer = split_world_NDCCorners[i];
                        Min = (glm::min)(Min, conrer);
                        Max = (glm::max)(Max, conrer);
                    }
                    AABB approx_shape = AABB{Min, Max};
                    approx_shape.RenderShape();
                    bool culled = approx_shape.VFCTest(camera->GetViewFrustumInViewSpace(), camera->GetViewMatrix());
                    bool exceedDropOff = lightCom->HandleShadowBufferDropOff(
                        glm::distance(camera->GetWorldPosition(), light_pos));
                    currentLight.castShadow = lightCom->shadow.bCastShadow && s_Data.enable_shadow && !culled && !
                        exceedDropOff;

                    auto currentSpotLight = CreateSpotLight(currentLight);
                    {
                        currentSpotLight.Radius_CollisionRadius_CosInnerCone_CosOutterCone.x = lightCom->spotLight.
                            distance;
                        currentSpotLight.Radius_CollisionRadius_CosInnerCone_CosOutterCone.y = lightCom->
                            collisionRadius;
                        currentSpotLight.Radius_CollisionRadius_CosInnerCone_CosOutterCone.z = cosf(fov_in * 0.5f);
                        currentSpotLight.Radius_CollisionRadius_CosInnerCone_CosOutterCone.w = cosf(fov_out * 0.5f);
                        currentSpotLight.SoftEdgeRatio.x = lightCom->spotLight.softEdgeRatio;
                    }

                    if (!currentLight.castShadow)
                    {
                        currentSpotLight.Kd_shadowMatrixIndex.w = -1;
                        // set to -1 to indicate not casting shadow that will be checked in the lighting shader
                    }
                    else
                    {
                        // Build shadow
                        lightCom->AllocateShadowBuffer();
                        const auto& shadowBuffer = lightCom->shadow.shadowBuffer;
                        Vec2u traget_resoluation = shadowBuffer->GetBufferSize();

                        // Update shadow matrix
                        {
                            ShadowData_GPU data;
                            data.PVMatrix = light_pv;
                            data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.x = lightCom->shadow.
                                shadowAlgorithmMode;
                            data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.y = lightCom->shadow.
                                depthBiasHigherBound;
                            data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.z = lightCom->shadow.
                                depthBiasMultiplier;
                            data.ShadowAlgorithmMode_DepthBiasHigher_DepthBiasMulti_NrmBiasMulti.w = lightCom->shadow.
                                nrmBiasMultiplier;
                            s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.emplace_back(std::move(data));
                            currentSpotLight.Kd_shadowMatrixIndex.w = s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.size() - 1;
                        }

                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

                        std::string shader_name;
                        std::shared_ptr<Shader> shader;
                        GetShadowBuffer_Opaque(lightCom, shader, shader_name);
                        s_Data.CurrentShader = shader;
                        s_Data.CurrentShader->Bind();
                        s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                        s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                        // Bind shadow buffer
                        shadowBuffer->Bind();
                        // Clear buffer
                        (s_Data.enable_reverse_z)
                            ? RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0))
                            : RenderCommand::SetClearColor(Vec4f(1, 1, 1, 0));
                        RenderCommand::Clear();
                        sceneCom->SetShouldDraw(false, true);
                        // Render the scene
                        {
                            ENG_TIME("Shadow phase: SPOT LOOPING");
                            f_setRenderShaderName(shader_name);
                            f_setVFCullingParam(true, clipping_vf, light_view_mat);
                            f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                            f_render_opaque();
                        }
                        {
                            ENG_TIME("Shadow phase: SPOT BATCH RENDER");
                            CommitBatchRendering();
                        }
                        if (lightCom->shadow.bEnableTransparentShadow)
                        {
                            BeginTransparentShadow();
                            {
                                ENG_TIME("Shadow phase: SPOT LOOPING (transparent)");
                                GetShadowBuffer_Particle(lightCom, shader, shader_name);
                                s_Data.CurrentShader = shader;
                                s_Data.CurrentShader->Bind();
                                s_Data.CurrentShader->SetMat4("u_ProjectionMatrix", light_projection_mat);
                                s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                GetShadowBuffer_Transparent(lightCom, shader, shader_name);
                                s_Data.CurrentShader = shader;
                                s_Data.CurrentShader->Bind();
                                s_Data.CurrentShader->SetMat4("u_PVMatrix", light_pv);
                                s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                                f_setRenderShaderName(shader_name);
                                f_setVFCullingParam(true, clipping_vf, light_view_mat);
                                f_render_trasparent();
                            }
                            EndTransparentShadow();
                        }
                        if (lightCom->shadow.bEnableGaussianBlur)
                        {
                            const auto& shadowBuffer2 = lightCom->shadow.shadowBuffer2;
                            Vec2u traget_resoluation2 = shadowBuffer2->GetBufferSize();
                            auto kernel_size = lightCom->shadow.gaussianKernal;
                            auto [length, offsets, weights] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
                            BeginGaussianBlur();
                            {
                                s_Data.CurrentShader = guassian_shader;
                                s_Data.CurrentShader->Bind();
                                s_Data.CurrentShader->SetInt("u_length", length);
                                weights->Bind(1);
                                offsets->Bind(2);
                                {
                                    RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                                    // Bind shadow buffer
                                    shadowBuffer2->Bind();
                                    RenderCommand::Clear();
                                    s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                                    shadowBuffer->BindTexture(s_Data.fragTexture_0_slot);
                                    _RenderFullScreenQuad();
                                }
                                {
                                    RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                                    // Bind shadow buffer
                                    shadowBuffer->Bind();
                                    RenderCommand::Clear();
                                    s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                                    shadowBuffer2->BindTexture(s_Data.fragTexture_0_slot);
                                    _RenderFullScreenQuad();
                                }
                            }
                            EndGaussianBlur();
                        }
                    }
                    s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED.emplace_back(std::move(currentSpotLight));
                    s_Data.gpuBuffer.SpotLightShadowBuffer.emplace_back(lightCom->shadow.shadowBuffer);
                }
                break;
            }
        }

        s_Data.NUM_DIRECTIONAL_LIGHT = s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED.size();
        s_Data.NUM_POINT_LIGHT = s_Data.cpuBuffer.POINT_LIGHT_PROCESSED.size();
        s_Data.NUM_SPOT_LIGHT = s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED.size();
        s_Data.NUM_SHADOW = s_Data.cpuBuffer.SHADOW_DATA_PROCESSED.size();
    }
}

/**************************************************************
*	Render3D highlevel API : EndOpaqueShadowing
*
**************************************************************/
void longmarch::Renderer3D::EndShadowing()
{
}

/**************************************************************
*	Render3D highlevel API : BeginOpaqueScene
*
**************************************************************/
void longmarch::Renderer3D::BeginOpaqueScene(
    const PerspectiveCamera* camera,
    const std::function<void()>& f_render,
    const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
    const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
    const std::function<void(const std::string&)>& f_setRenderShaderName)
{
    // Populate shading parameters
    {
        ENG_TIME("Scene phase (Opaque): Populate Shadow Data");
        _PopulateShadowPassVariables();
    }

    {
        ENG_TIME("Scene phase (Opaque): BEGIN");
        s_Data.RENDER_PASS = RENDER_PASS::SCENE;

        // Begin geomtry passes
        {
            switch (s_Data.RENDER_PIPE)
            {
            case RENDER_PIPE::DEFERRED:
                Renderer3D::_BeginDeferredGeomtryPass(camera, s_Data.gpuBuffer.CurrentGBuffer);
                break;
            case RENDER_PIPE::FORWARD:
                Renderer3D::_BeginForwardGeomtryPass(camera, s_Data.gpuBuffer.CurrentFrameBuffer);
                break;
            }
        }
    }
    {
        // Bind default shader here to avoid rebinding in the subsequent draw call
        constexpr auto shader_name = "OpaqueRenderShader";
        s_Data.CurrentShader = s_Data.ShaderMap[shader_name];
        s_Data.CurrentShader->Bind();
        // Rendering
        {
            ENG_TIME("Scene phase (Opaque): LOOPING");
            f_setRenderShaderName(shader_name);
            f_setVFCullingParam(true, camera->GetViewFrustumInViewSpace(), camera->GetViewMatrix());
            f_setDistanceCullingParam(false, Vec3f(), 0, 0);
            f_render();
        }
        {
            ENG_TIME("Scene phase (Opaque): BATCH RENDER");
            CommitBatchRendering();
        }
    }
}

void longmarch::Renderer3D::_BeginForwardGeomtryPass(const PerspectiveCamera* camera,
                                                     const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    {
        ENG_TIME("Scene Begin Forward Geomtry Pass");
        RenderCommand::DepthTest(true, true); // Enable depth testing
        RenderCommand::Blend(true); // Enable blending
        RenderCommand::BlendFunc(RendererAPI::BlendFuncEnum::ALPHA_BLEND_1);
        RenderCommand::CullFace(true, false); // Cull back faces

        (s_Data.enable_wireframe) ? RenderCommand::PolyModeLine() : RenderCommand::PolyModeFill();

        // bind framebuffer to store fragment color
        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginDeferredGeomtryPass(const PerspectiveCamera* camera,
                                                      const std::shared_ptr<GBuffer>& gBuffer_out)
{
    {
        ENG_TIME("Scene Begin Deferred Geomtry Pass");
        RenderCommand::DepthTest(true, true); // Enable depth testing
        RenderCommand::Blend(false); // Disable blending (cause gbuffer would use the alpha channel to store data)
        RenderCommand::CullFace(true, false); // Cull back faces
        (s_Data.enable_wireframe) ? RenderCommand::PolyModeLine() : RenderCommand::PolyModeFill();
        {
            gBuffer_out->Bind();
            Vec2u traget_resoluation = gBuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
    }
}

void longmarch::Renderer3D::_PopulateShadingPassUniformsVariables(const PerspectiveCamera* camera)
{
    GPU_TIME(_PopulateShaderUniforms);
    Mat4 v;
    Mat4 v_inv;
    Mat4 p;
    Mat4 p_inv;
    Mat4 pv;
    Mat4 ppv;

    v = camera->GetViewMatrix();
    if (s_Data.enable_reverse_z)
    {
        p = camera->GetReverseZProjectionMatrix();
        pv = camera->GetReverseZViewProjectionMatrix();
        ppv = camera->GetPrevReverseZViewProjectionMatrix();
    }
    else
    {
        p = camera->GetProjectionMatrix();
        pv = camera->GetViewProjectionMatrix();
        ppv = camera->GetPrevViewProjectionMatrix();
    }

    v_inv = Geommath::SmartInverse(v);
    p_inv = Geommath::SmartInverse(p);
    if (s_Data.SMAASettings.enable)
    {
        // Do we need to jitter projection matrix? 
        // Not really. Because the projection matrix is used in many other post processing shader that are irralavent to temporal supersampling.
        // Just jitter the PV matrix and Prev PV matrix is all good to generated temporal supersampling for SMAA T2X
        pv = s_Data.SMAASettings.JitteredMatrix(pv, s_Data.resolution.x, s_Data.resolution.y, s_Data.frameIndex);
        ppv = s_Data.SMAASettings.JitteredMatrix(ppv, s_Data.resolution.x, s_Data.resolution.y, s_Data.frameIndex);
    }

    // Update shader uniforms
    {
        for (auto&& shaderName : s_Data.ListShadersToPopulateData)
        {
            const auto& shaderProg = s_Data.ShaderMap[shaderName];
            shaderProg->Bind();
            shaderProg->SetFloat3("Ia", Vec3f(s_Data.ambient));
            shaderProg->SetInt("hasEnvLighting",
                               s_Data.EnvMapping.enable_env_mapping && s_Data.EnvMapping.CurrentEnvMapName != "");
            shaderProg->SetFloat("u_Time", Engine::GetTotalTime());
            shaderProg->SetInt("u_numLights", s_Data.NUM_LIGHT);
            shaderProg->SetInt("u_numDirectionalLights", s_Data.NUM_DIRECTIONAL_LIGHT);
            shaderProg->SetInt("u_numPointLights", s_Data.NUM_POINT_LIGHT);
            shaderProg->SetInt("u_numSpotLights", s_Data.NUM_SPOT_LIGHT);
            shaderProg->SetMat4("u_ProjectionMatrix", p);
            shaderProg->SetMat4("u_ProjectionMatrixInv", p_inv);
            shaderProg->SetMat4("u_ViewMatrix", v);
            shaderProg->SetMat4("u_ViewMatrixInv", v_inv);
            shaderProg->SetMat4("u_PVMatrix", pv);
            shaderProg->SetMat4("u_PrevPVMatrix", ppv);
            shaderProg->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);

            shaderProg->SetFloat("u_sRGB2LinearGamma", s_Data.value_gamma);

            shaderProg->SetInt("u_GBufferDisplayMode", s_Data.gBuffer_display_mode);
            shaderProg->SetInt("u_DirectionalLightDisplayMode", s_Data.directional_light_display_mode);

            shaderProg->SetFloat("u_zNear", camera->cameraSettings.nearZ);
            shaderProg->SetFloat("u_zFar", camera->cameraSettings.farZ);
        }
    }
}

void longmarch::Renderer3D::_PopulateShadowPassVariables()
{
    GPU_TIME(_PopulateShadowPassVariables);
    // CRITICAL: Create placeholder texture (must have) for directional light
    static const auto placeholder_shadowBuffer_array =
        ShadowBuffer::CreateArray(1, 1, 4, ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE);
    //ShadowBuffer::CreateArray(1, 1, 4, ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);

    // CRITICAL: Create placeholder texture (must have) for point light
    static const auto placeholder_shadowBuffer_cube =
        ShadowBuffer::CreateArray(1, 1, 6, ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4);
    // ShadowBuffer::Create(1, 1, ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE);

    // CRITICAL: Create placeholder texture (must have) for spot light
    static const auto placeholder_shadowBuffer = ShadowBuffer::Create(1, 1, ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4);

    {
        // Shadow texture offset needs to be larger than the sum of empty slots and scene texture slots.
        int init_offset_shadow = s_Data.fragTexture_empty_slot + s_Data.MAX_SCENE_BATCH + 1;
        int total_shaows_count = 0;
        int light_type_count = 0;
        {
            int shadow_index = 0;
            int offset_shadow = init_offset_shadow + (total_shaows_count + light_type_count);
            placeholder_shadowBuffer_array->BindTexture(offset_shadow); // Always bind placeholder texture
            /**************************************************************
            *	CRITICAL: need to fill in this whole list with some dummy slot (e.g. 2) that you have binded with an empty texture
            *	If not, then unset samplers will be initialized as 0. Shaders will crash at run ENG_TIME if you bind 0 to another sampler type (e.g. samplerCube)!
            **************************************************************/
            int* ShadowMapTextureArrayId = new int[s_Data.MAX_DIRECTIONAL_SHADOW];
            for (auto i(0u); i < s_Data.MAX_DIRECTIONAL_SHADOW; ++i)
            {
                ShadowMapTextureArrayId[i] = offset_shadow; // Bind default texture slot to the placeholder texture
            }
            for (auto i(0u); i < s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED.size(); ++i)
            {
                auto& currentLight = s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED[i];
                const auto& shadowBuffer = s_Data.gpuBuffer.DirectionalLightShadowBuffer[i];
                if (currentLight.Kd_shadowMatrixIndex.w != -1 && shadowBuffer)
                {
                    int slot = offset_shadow + (shadow_index + 1);
                    ShadowMapTextureArrayId[shadow_index] = slot;
                    shadowBuffer->BindTexture(slot);
                    currentLight.Pos_shadowMapIndex.w = shadow_index;
                    if (++shadow_index == s_Data.MAX_DIRECTIONAL_SHADOW)
                    {
                        break;
                    }
                }
            }
            for (auto&& shaderName : s_Data.ListRenderShadersToPopulateShadowData)
            {
                const auto& shaderProg = s_Data.ShaderMap[shaderName];
                shaderProg->Bind();
                shaderProg->SetIntV("u_shadowMapTextureArrayList", s_Data.MAX_DIRECTIONAL_SHADOW,
                                    ShadowMapTextureArrayId);
            }
            delete[] ShadowMapTextureArrayId;

            total_shaows_count += shadow_index;
            ++light_type_count;
        }
        {
            int shadow_index = 0;
            int offset_shadow = init_offset_shadow + (total_shaows_count + light_type_count);
            placeholder_shadowBuffer_cube->BindTexture(offset_shadow); // Always bind placeholder texture
            /**************************************************************
            *	CRITICAL: need to fill in this whole list with some dummy slot (e.g. 2) that you have binded with an empty texture
            *	If not, then unset samplers will be initialized as 0. Shaders will crash at run ENG_TIME if you bind 0 to another sampler type (e.g. samplerCube)!
            **************************************************************/
            int* ShadowMapTextureCubeId = new int[s_Data.MAX_POINT_SHADOW];
            for (auto i(0u); i < s_Data.MAX_POINT_SHADOW; ++i)
            {
                ShadowMapTextureCubeId[i] = offset_shadow; // Bind default texture slot to the placeholder texture
            }
            for (auto i(0u); i < s_Data.cpuBuffer.POINT_LIGHT_PROCESSED.size(); ++i)
            {
                auto& currentLight = s_Data.cpuBuffer.POINT_LIGHT_PROCESSED[i];
                const auto& shadowBuffer = s_Data.gpuBuffer.PointLightShadowBuffer[i];
                if (currentLight.Kd_shadowMatrixIndex.w != -1 && shadowBuffer)
                {
                    int slot = offset_shadow + (shadow_index + 1);
                    ShadowMapTextureCubeId[shadow_index] = slot;
                    shadowBuffer->BindTexture(slot);
                    currentLight.Pos_shadowMapIndex.w = shadow_index;
                    if (++shadow_index == s_Data.MAX_POINT_SHADOW)
                    {
                        break;
                    }
                }
            }
            for (auto&& shaderName : s_Data.ListRenderShadersToPopulateShadowData)
            {
                const auto& shaderProg = s_Data.ShaderMap[shaderName];
                shaderProg->Bind();
                shaderProg->SetIntV("u_shadowMapTextureCubeList", s_Data.MAX_POINT_SHADOW, ShadowMapTextureCubeId);
            }
            delete[] ShadowMapTextureCubeId;

            total_shaows_count += shadow_index;
            ++light_type_count;
        }
        {
            int shadow_index = 0;
            int offset_shadow = init_offset_shadow + (total_shaows_count + light_type_count);
            placeholder_shadowBuffer->BindTexture(offset_shadow); // Always bind placeholder texture
            /**************************************************************
            *	CRITICAL: need to fill in this whole list with some dummy slot (e.g. 2) that you have binded with an empty texture
            *	If not, then unset samplers will be initialized as 0. Shaders will crash at run ENG_TIME if you bind 0 to another sampler type (e.g. samplerCube)!
            **************************************************************/
            int* ShadowMapTextureId = new int[s_Data.MAX_SPOT_SHADOW];
            for (auto i(0u); i < s_Data.MAX_SPOT_SHADOW; ++i)
            {
                ShadowMapTextureId[i] = offset_shadow; // Bind default texture slot to the placeholder texture
            }
            for (auto i(0u); i < s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED.size(); ++i)
            {
                auto& currentLight = s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED[i];
                const auto& shadowBuffer = s_Data.gpuBuffer.SpotLightShadowBuffer[i];
                if (currentLight.Kd_shadowMatrixIndex.w != -1 && shadowBuffer)
                {
                    int slot = offset_shadow + (shadow_index + 1);
                    ShadowMapTextureId[shadow_index] = slot;
                    shadowBuffer->BindTexture(slot);
                    currentLight.Pos_shadowMapIndex.w = shadow_index;
                    if (++shadow_index == s_Data.MAX_SPOT_SHADOW)
                    {
                        break;
                    }
                }
            }
            for (auto&& shaderName : s_Data.ListRenderShadersToPopulateShadowData)
            {
                const auto& shaderProg = s_Data.ShaderMap[shaderName];
                shaderProg->Bind();
                shaderProg->SetIntV("u_shadowMapTextureList", s_Data.MAX_SPOT_SHADOW, ShadowMapTextureId);
            }
            delete[] ShadowMapTextureId;

            total_shaows_count += shadow_index;
            ++light_type_count;
        }
    }
    if (s_Data.NUM_DIRECTIONAL_LIGHT > 0)
    {
        s_Data.gpuBuffer.DirectionalLightBuffer->UpdateBufferData(
            s_Data.cpuBuffer.DIRECTIONAL_LIGHT_PROCESSED[0].GetPtr(),
            s_Data.NUM_DIRECTIONAL_LIGHT * sizeof(DirectionalLightBuffer_GPU));
        s_Data.gpuBuffer.DirectionalLightBuffer->Bind(10);
    }
    if (s_Data.NUM_POINT_LIGHT > 0)
    {
        s_Data.gpuBuffer.PointLightBuffer->UpdateBufferData(s_Data.cpuBuffer.POINT_LIGHT_PROCESSED[0].GetPtr(),
                                                            s_Data.NUM_POINT_LIGHT * sizeof(PointLightBuffer_GPU));
        s_Data.gpuBuffer.PointLightBuffer->Bind(11);
    }
    if (s_Data.NUM_SPOT_LIGHT > 0)
    {
        s_Data.gpuBuffer.SpotLightBuffer->UpdateBufferData(s_Data.cpuBuffer.SPOT_LIGHT_PROCESSED[0].GetPtr(),
                                                           s_Data.NUM_SPOT_LIGHT * sizeof(SpotLightBuffer_GPU));
        s_Data.gpuBuffer.SpotLightBuffer->Bind(12);
    }
    if (s_Data.NUM_SHADOW > 0)
    {
        s_Data.gpuBuffer.ShadowPVMatrixBuffer->UpdateBufferData(s_Data.cpuBuffer.SHADOW_DATA_PROCESSED[0].GetPtr(),
                                                                s_Data.NUM_SHADOW * sizeof(ShadowData_GPU));
        s_Data.gpuBuffer.ShadowPVMatrixBuffer->Bind(13);
    }
}

/**************************************************************
*	Render3D highlevel API : EndOpaqueScene
*
**************************************************************/
void longmarch::Renderer3D::EndOpaqueScene()
{
}

/**************************************************************
*	Render3D highlevel API : BeginOpaqueLighting
*
**************************************************************/
void longmarch::Renderer3D::BeginOpaqueLighting(
    const PerspectiveCamera* camera,
    const std::function<void()>& f_render,
    const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
    const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
    const std::function<void(const std::string&)>& f_setRenderShaderName)
{
    switch (s_Data.RENDER_PIPE)
    {
    case RENDER_PIPE::DEFERRED:
        // Perform SSAO/SSDO after rendering all opaques, ignore transparents and particles.
        Renderer3D::_BeginDynamicSSAOPass();
        Renderer3D::_BeginDeferredLightingPass(s_Data.gpuBuffer.CurrentFrameBuffer);
        break;
    case RENDER_PIPE::FORWARD:
        Renderer3D::_BeginForwardLightingPass(s_Data.gpuBuffer.CurrentFrameBuffer);
        break;
    }

    // Fill in prev frame buffer from current frame buffer before skybox pass
    RenderCommand::TransferColorBit(
        s_Data.gpuBuffer.CurrentFrameBuffer->GetFrameBufferID(),
        s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().x,
        s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().y,

        s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->GetFrameBufferID(),
        s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->GetBufferSize().x,
        s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->GetBufferSize().y
    );
    s_Data.gpuBuffer.PrevOpaqueLightingFrameBuffer->GenerateMipMapLevel();

    if (s_Data.RENDER_PIPE == Renderer3D::RENDER_PIPE::DEFERRED)
    {
        Renderer3D::_BeginDynamicSSGIPass(s_Data.gpuBuffer.CurrentFrameBuffer);
        Renderer3D::_BeginSSGIPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.CurrentFrameBuffer);

        // Perform SSR after rendering all opaques, ignore transparents and particles for now
        Renderer3D::_BeginDynamicSSRPass(
            camera,
            f_render,
            f_setVFCullingParam,
            f_setDistanceCullingParam,
            f_setRenderShaderName,
            s_Data.gpuBuffer.CurrentFrameBuffer);
        Renderer3D::_BeginSSRPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.CurrentFrameBuffer);
    }
    {
        Renderer3D::_BeginSkyBoxPass(s_Data.gpuBuffer.CurrentFrameBuffer);
    }
    {
        Renderer3D::_RenderBoundingBox(s_Data.gpuBuffer.CurrentFrameBuffer);
    }
}

void longmarch::Renderer3D::_BeginDynamicSSGIPass(const std::shared_ptr<FrameBuffer>& colorBuffer_in)
{
    if (s_Data.SSGISettings.enable)
    {
        GPU_TIME(DynamicSSGI);
        {
            // Clear SSGI buffer regardless if SSGI is enabled because it will always be used
            if (auto downscale = s_Data.SSGISettings.gi_sample_resolution_downScale;
                s_Data.gpuBuffer.CurrentDynamicSSGIBuffer->GetBufferSize() != s_Data.resolution / downscale)
            // Render the SSGI with potentially downscaled resolution
            {
                s_Data.gpuBuffer.CurrentDynamicSSGIBuffer = FrameBuffer::Create(
                    s_Data.resolution.x / downscale, s_Data.resolution.y / downscale,
                    FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            }

            RenderCommand::DepthTest(true, true);
            RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));
            // Clear w component to 1 as it stores the SSGI value, and 1 stands for no occlusion

            s_Data.gpuBuffer.CurrentDynamicSSGIBuffer->Bind();
            RenderCommand::Clear();
        }

        //------------------------------------------------------------------------------

        // SSGI/SSDO pass
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::Blend(false); // Disable blend
        RenderCommand::DepthTest(false, false); // Disable depth test
        RenderCommand::CullFace(false, false);

        auto& GIBuffer = s_Data.gpuBuffer.CurrentDynamicSSGIBuffer;
        Vec2u traget_resoluation = GIBuffer->GetBufferSize();
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

        const auto& gi_shader = s_Data.ShaderMap["DynamicSSGIShader"];
        s_Data.CurrentShader = gi_shader;
        s_Data.CurrentShader->Bind();
        s_Data.CurrentShader->SetFloat("scale_indirect_bounce", s_Data.SSGISettings.gi_strength);
        s_Data.CurrentShader->SetInt("num_sample", s_Data.SSGISettings.gi_samples);
        s_Data.CurrentShader->SetFloat("sample_radius", s_Data.SSGISettings.gi_sample_radius);
        GIBuffer->Bind();

        // Bind g buffer content
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL,
                GBuffer::GBUFFER_TEXTURE_TYPE::ALBEDO_EMSSIVE,
            },
            s_Data.fragTexture_empty_slot
        );

        // Bind prev frame buffer
        colorBuffer_in->BindTexture(s_Data.fragTexture_3_slot);

        // Render quad
        Renderer3D::_RenderFullScreenQuad();

        //------------------------------------------------------------------------------

        // Bilaterl blurring
        const auto& guassian_shader = s_Data.ShaderMap["GaussianBlur"];
        Vec2u traget_resoluation2(traget_resoluation);
        static auto GIBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.x,
                                                       FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        if (GIBackBuffer->GetBufferSize() != traget_resoluation2)
        {
            GIBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.y,
                                               FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        auto kernel_size = s_Data.SSGISettings.gi_gaussian_kernal;
#ifdef LongMarch_SCALE_GUASSIAN_KERNEL_WITH_RESOLUTION
        auto kernel_ratio = std::min(traget_resoluation2.x, traget_resoluation2.y) / 1080.0f;
        kernel_size *= kernel_ratio;
        kernel_size = std::max(kernel_size, LongMarch_GUASSIAN_KERNEL_MIN);
#endif
        auto [length, offset, weight] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
        {
            s_Data.CurrentShader = guassian_shader;
            s_Data.CurrentShader->Bind();
            s_Data.CurrentShader->SetInt("u_length", length);
            weight->Bind(1);
            offset->Bind(2);
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                // Bind shadow buffer
                GIBackBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                GIBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                // Bind shadow buffer
                GIBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                GIBackBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
        }
    }
}

void longmarch::Renderer3D::_BeginDynamicSSAOPass()
{
    {
        // Clear SSAO buffer regardless if SSAO is enabled because it will always be used
        if (auto downscale = s_Data.SSAOSettings.ao_sample_resolution_downScale;
            s_Data.gpuBuffer.CurrentDynamicSSAOBuffer->GetBufferSize() != s_Data.resolution / downscale)
        // Render the SSAO with potentially downscaled resolution
        {
            s_Data.gpuBuffer.CurrentDynamicSSAOBuffer = FrameBuffer::Create(
                s_Data.resolution.x / downscale, s_Data.resolution.y / downscale, FrameBuffer::BUFFER_FORMAT::UINT_R8);
        }

        RenderCommand::DepthTest(true, true);
        RenderCommand::SetClearColor(Vec4f(1, 1, 1, 1));
        // Clear w component to 1 as it stores the SSAO value, and 1 stands for no occlusion

        s_Data.gpuBuffer.CurrentDynamicSSAOBuffer->Bind();
        RenderCommand::Clear();
    }

    if (s_Data.SSAOSettings.enable)
    {
        GPU_TIME(DynamicSSAO);

        //------------------------------------------------------------------------------

        // SSAO/SSDO pass
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::Blend(false); // Disable blend
        RenderCommand::DepthTest(false, false); // Disable depth test
        RenderCommand::CullFace(false, false);

        auto& AOBuffer = s_Data.gpuBuffer.CurrentDynamicSSAOBuffer;
        Vec2u traget_resoluation = AOBuffer->GetBufferSize();
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

        const auto& ao_shader = s_Data.ShaderMap["DynamicSSAOShader"];
        s_Data.CurrentShader = ao_shader;
        s_Data.CurrentShader->Bind();
        s_Data.CurrentShader->SetInt("num_sample", s_Data.SSAOSettings.ao_samples);
        s_Data.CurrentShader->SetFloat("sample_radius", s_Data.SSAOSettings.ao_sample_radius);
        s_Data.CurrentShader->SetFloat("scale_s", s_Data.SSAOSettings.ao_scale);
        s_Data.CurrentShader->SetFloat("power_k", s_Data.SSAOSettings.ao_power);
        AOBuffer->Bind();

        // Bind g buffer content
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL,
            },
            s_Data.fragTexture_empty_slot
        );

        // Render quad
        Renderer3D::_RenderFullScreenQuad();

        //------------------------------------------------------------------------------

        // Bilaterl blurring
        const auto& guassian_shader = s_Data.ShaderMap["BilateralBlur"];
        Vec2u traget_resoluation2(traget_resoluation);
        static auto AOBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.x,
                                                       FrameBuffer::BUFFER_FORMAT::UINT_R8);
        if (AOBackBuffer->GetBufferSize() != traget_resoluation2)
        {
            AOBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.y,
                                               FrameBuffer::BUFFER_FORMAT::UINT_R8);
        }
        auto kernel_size = s_Data.SSAOSettings.ao_gaussian_kernal;
#ifdef LongMarch_SCALE_GUASSIAN_KERNEL_WITH_RESOLUTION
        auto kernel_ratio = std::min(traget_resoluation2.x, traget_resoluation2.y) / 1080.0f;
        kernel_size *= kernel_ratio;
        kernel_size = std::max(kernel_size, LongMarch_GUASSIAN_KERNEL_MIN);
#endif
        auto [length, offset, weight] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
        {
            s_Data.CurrentShader = guassian_shader;
            s_Data.CurrentShader->Bind();
            s_Data.CurrentShader->SetInt("u_length", length);
            s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                {
                    GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                    GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL,
                },
                s_Data.fragTexture_empty_slot
            );
            weight->Bind(1);
            offset->Bind(2);
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                // Bind shadow buffer
                AOBackBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                AOBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                // Bind shadow buffer
                AOBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                AOBackBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
        }
    }
}

void longmarch::Renderer3D::_BeginDynamicSSRPass(
    const PerspectiveCamera* camera,
    const std::function<void()>& f_render,
    const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
    const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
    const std::function<void(const std::string&)>& f_setRenderShaderName,
    const std::shared_ptr<FrameBuffer>& colorBuffer_in)
{
    if (s_Data.SSRSettings.enable)
    {
        GPU_TIME(DynamicSSR);
        {
            // Clear buffer
            if (auto downscale = s_Data.SSRSettings.ssr_sample_resolution_downScale;
                s_Data.gpuBuffer.CurrentDynamicSSRBuffer->GetBufferSize() != s_Data.resolution / downscale)
            // Render the SSR with potentially downscaled resolution
            {
                s_Data.gpuBuffer.CurrentDynamicSSRBuffer = FrameBuffer::Create(
                    s_Data.resolution.x / downscale, s_Data.resolution.y / downscale,
                    FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
            }

            if (auto downscale = s_Data.SSRSettings.ssr_sample_resolution_downScale;
                s_Data.gpuBuffer.CurrentBackFaceDepthBuffer->GetBufferSize() != s_Data.resolution / downscale)
            // Render the SSR with potentially downscaled resolution
            {
                s_Data.gpuBuffer.CurrentBackFaceDepthBuffer = ShadowBuffer::Create(
                    s_Data.resolution.x / downscale, s_Data.resolution.y / downscale,
                    ShadowBuffer::SHADOW_MAP_TYPE::BASIC);
            }

            RenderCommand::DepthTest(true, true);
            RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));

            s_Data.gpuBuffer.CurrentDynamicSSRBuffer->Bind();
            RenderCommand::Clear();

            s_Data.gpuBuffer.CurrentBackFaceDepthBuffer->Bind();
            RenderCommand::Clear();
        }

        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::Blend(false); // Disable blend

        //------------------------------------------------------------------------------

        {
            // Back face depth pass
            RenderCommand::DepthTest(true, true); // Enable depth test
            RenderCommand::CullFace(true, true); // Cull front face to draw back face depth

            auto render_pass_original = s_Data.RENDER_PASS;
            s_Data.RENDER_PASS = RENDER_PASS::SHADOW;

            auto& BackFaceDepthBuffer = s_Data.gpuBuffer.CurrentBackFaceDepthBuffer;
            Vec2u traget_resoluation = BackFaceDepthBuffer->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            BackFaceDepthBuffer->Bind();
            {
                // Bind default shader here to avoid rebinding in the subsequent draw call
                constexpr auto shader_name = "ShadowBuffer";
                s_Data.CurrentShader = s_Data.ShaderMap[shader_name];
                s_Data.CurrentShader->Bind();
                s_Data.CurrentShader->SetMat4("u_PVMatrix",
                                              (s_Data.enable_reverse_z)
                                                  ? camera->GetReverseZViewProjectionMatrix()
                                                  : camera->GetViewProjectionMatrix());
                s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                // Rendering
                {
                    ENG_TIME("SSR pass (Back face depth): LOOPING");
                    f_setRenderShaderName(shader_name);
                    f_setVFCullingParam(true, camera->GetViewFrustumInViewSpace(), camera->GetViewMatrix());
                    f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                    f_render();
                }

                {
                    ENG_TIME("SSR pass (Back face depth): BATCH RENDER");
                    CommitBatchRendering();
                }
            }

            s_Data.RENDER_PASS = render_pass_original;
        }

        //------------------------------------------------------------------------------

        // SSR ray march
        RenderCommand::DepthTest(false, false); // Disable depth test
        RenderCommand::CullFace(false, false);

        auto& SSRBuffer = s_Data.gpuBuffer.CurrentDynamicSSRBuffer;
        Vec2u traget_resoluation = SSRBuffer->GetBufferSize();
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

        const auto& ssr_shader = s_Data.ShaderMap["DynamicSSRShader"];
        s_Data.CurrentShader = ssr_shader;
        s_Data.CurrentShader->Bind();
        s_Data.CurrentShader->SetInt("enabled_debug", s_Data.SSRSettings.enable_debug);
        s_Data.CurrentShader->SetFloat2("u_ScreenSize", Vec2f(SSRBuffer->GetBufferSize()));
        SSRBuffer->Bind();

        // Bind Skybox
        _BindSkyBoxTexture();
        // Bind g buffer content
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL,
                GBuffer::GBUFFER_TEXTURE_TYPE::ALBEDO_EMSSIVE,
                GBuffer::GBUFFER_TEXTURE_TYPE::BAKEDAO_METALLIC_ROUGHNESS,
            },
            s_Data.fragTexture_empty_slot
        );
        // Bind prev frame buffer
        colorBuffer_in->BindTexture(s_Data.fragTexture_3_slot);
        // Bind back face depth buffer
        s_Data.gpuBuffer.CurrentBackFaceDepthBuffer->BindTexture(s_Data.fragTexture_4_slot);

        // Render quad
        Renderer3D::_RenderFullScreenQuad();

        //------------------------------------------------------------------------------

        // Bilaterl blurring
        const auto& guassian_shader = s_Data.ShaderMap["GaussianBlur"];
        Vec2u traget_resoluation2(traget_resoluation);
        static auto SSRBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.x,
                                                        FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        if (SSRBackBuffer->GetBufferSize() != traget_resoluation2)
        {
            SSRBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.y,
                                                FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        auto kernel_size = s_Data.SSRSettings.ssr_gaussian_kernal;
#ifdef LongMarch_SCALE_GUASSIAN_KERNEL_WITH_RESOLUTION
        auto kernel_ratio = std::min(traget_resoluation2.x, traget_resoluation2.y) / 1080.0f;
        kernel_size *= kernel_ratio;
        kernel_size = std::max(kernel_size, LongMarch_GUASSIAN_KERNEL_MIN);
#endif
        auto [length, offset, weight] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
        {
            s_Data.CurrentShader = guassian_shader;
            s_Data.CurrentShader->Bind();
            s_Data.CurrentShader->SetInt("u_length", length);
            weight->Bind(1);
            offset->Bind(2);
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                // Bind shadow buffer
                SSRBackBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                SSRBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                // Bind shadow buffer
                SSRBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                SSRBackBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
        }
    }
}

void longmarch::Renderer3D::_BeginDeferredLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    GPU_TIME(DeferredLighting_Pass);
    {
        // bind framebuffer to store frag color
        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
    }
    {
        // Opaque phase
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::Blend(true); // Enable blending
        RenderCommand::BlendFunc(RendererAPI::BlendFuncEnum::ADDITION);
        RenderCommand::DepthTest(false, false); // Disable depth test
        RenderCommand::CullFace(false, false);

        {
            s_Data.CurrentShader = s_Data.ShaderMap["DeferredShader"];
            s_Data.CurrentShader->Bind();
            // Bind env mapping
            _BindSkyBoxTexture();
            // Bind GBuffer
            s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                {
                    GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                    GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL,
                    GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY,
                    GBuffer::GBUFFER_TEXTURE_TYPE::ALBEDO_EMSSIVE,
                    GBuffer::GBUFFER_TEXTURE_TYPE::BAKEDAO_METALLIC_ROUGHNESS,
                },
                s_Data.fragTexture_empty_slot // offsets to be after all frame buffers
            );
            s_Data.gpuBuffer.CurrentDynamicSSAOBuffer->BindTexture(
                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NUM));
            // Render quad
            Renderer3D::_RenderFullScreenQuad();
        }
    }
    {
        // Populate depth buffer from GBuffer to FBO
        RenderCommand::DepthTest(true, true); // Write to depth buffer

        // Transfer depth buffer to framebuffer by using a depth copy shader
        // as long as the format is not GL_DEPTH24_STENCIL8
        // This shader requires the depth gbuffer to be binded, which is done in above
        s_Data.CurrentShader = s_Data.ShaderMap["DepthCopyShader"];
        s_Data.CurrentShader->Bind();
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH
            },
            s_Data.fragTexture_empty_slot // offsets to be after all frame buffers
        );

        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginForwardLightingPass(const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    {
        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BindSkyBoxTexture()
{
    // Bind Skybox
    if (s_Data.EnvMapping.CurrentEnvMapName != "")
    {
        const auto& irradiance = s_Data.gpuBuffer.EnvCubeMaps[s_Data.EnvMapping.CurrentEnvMapName]["irradiance"];
        const auto& radiance = s_Data.gpuBuffer.EnvCubeMaps[s_Data.EnvMapping.CurrentEnvMapName]["radiance"];
        s_Data.CurrentShader->SetFloat("u_max_radiance_map_lod", radiance->GetMaxMipMapLevel());
        // Bind irradiance map
        irradiance->BindTexture(s_Data.fragTexture_0_slot);
        // Bind radiance map
        radiance->BindTexture(s_Data.fragTexture_1_slot);
        // Bind brdf LUT
        s_Data.gpuBuffer.BrdfIntegrateLUT->BindTexture(s_Data.fragTexture_2_slot);
    }
}

void longmarch::Renderer3D::_BeginSkyBoxPass(const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    GPU_TIME(SkyBox_Pass);
    {
        ENG_TIME("SkyBox phase");

        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::Blend(false); // Disable blend
        RenderCommand::CullFace(true, true); // Cull front face (showing inside faces of the cube)
        RenderCommand::DepthTest(true, false); // Do depth test but do not write to the depth buffer
        RenderCommand::DepthFunc(RendererAPI::CompareEnum::EQUAL); //Only draw the background

        // bind framebuffer to store frag color
        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["SkyboxShader"];
        s_Data.CurrentShader->Bind();
        if (s_Data.EnvMapping.CurrentEnvMapName != "")
        {
            auto& skybox = s_Data.gpuBuffer.EnvCubeMaps[s_Data.EnvMapping.CurrentEnvMapName]["original"];
            skybox->BindTexture(s_Data.fragTexture_0_slot);
            _RenderFullScreenCube();
        }
        (s_Data.enable_reverse_z)
            ? RenderCommand::DepthFunc(RendererAPI::CompareEnum::GREATER)
            : RenderCommand::DepthFunc(RendererAPI::CompareEnum::LESS);
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_RenderBoundingBox(const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    size_t num_instance = s_Data.cpuBuffer.InstancedDraw_BVModelTr.size();
    if (s_Data.enable_drawingBoundingVolume && num_instance > 0)
    {
        RenderCommand::PolyModeFill(); // Draw full mode;
        RenderCommand::CullFace(true, false); // Cull back faces
        RenderCommand::Blend(true); // Enable blending
        RenderCommand::BlendFunc(RendererAPI::BlendFuncEnum::ALPHA_BLEND_2);
        RenderCommand::DepthTest(true, true); // Enable depth testing and writing

        {
            if (framebuffer_out)
            {
                framebuffer_out->Bind();
                Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
            else
            {
                RenderCommand::BindDefaultFrameBuffer();
                Vec2u traget_resoluation = s_Data.window_size;
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
        }

        s_Data.CurrentShader = s_Data.ShaderMap["BBoxShader"];
        s_Data.CurrentShader->Bind();
        s_Data.gpuBuffer.BBoxInstBO->UpdateBufferData(&(s_Data.cpuBuffer.InstancedDraw_BVModelTr[0][0][0]),
                                                      num_instance * sizeof(Mat4));
        s_Data.gpuBuffer.BBoxVAO->Bind();
        RenderCommand::PolyLineWidth(2);
        RenderCommand::DrawLineIndexedInstanced(s_Data.gpuBuffer.BBoxVAO, num_instance);
        RenderCommand::PolyLineWidth(1);
    }
    if (num_instance > 0)
    {
        s_Data.cpuBuffer.InstancedDraw_BVModelTr.clear();
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

/**************************************************************
*	Render3D highlevel API : EndOpaqueLighting
*
**************************************************************/
void longmarch::Renderer3D::EndOpaqueLighting()
{
}

/**************************************************************
*	Render3D highlevel API : BeginTransparentSceneAndLighting
*
**************************************************************/
void longmarch::Renderer3D::BeginTransparentSceneAndLighting(
    const PerspectiveCamera* camera,
    const std::function<void()>& f_render,
    const std::function<void(bool, const ViewFrustum&, const Mat4&)>& f_setVFCullingParam,
    const std::function<void(bool, const Vec3f&, float, float)>& f_setDistanceCullingParam,
    const std::function<void(const std::string&)>& f_setRenderShaderName)
{
    {
        ENG_TIME("Scene phase (Transparent): BEGIN");

        auto render_pass_original = s_Data.RENDER_PASS;
        auto render_pipe_original = s_Data.RENDER_PIPE;
        auto render_mode_original = s_Data.RENDER_MODE;
        s_Data.RENDER_PASS = RENDER_PASS::SCENE;
        s_Data.RENDER_PIPE = RENDER_PIPE::FORWARD;
        s_Data.RENDER_MODE = RENDER_MODE::CANONICAL;

        // Forward geomtry pass for translucent scene objects
        {
            Renderer3D::_BeginForwardGeomtryPass(camera, s_Data.gpuBuffer.CurrentFrameBuffer);
            RenderCommand::DepthTest(true, false); // Enable depth testing
            RenderCommand::CullFace(false, false); // Cull back faces
        }

        {
            // Bind default shader here to avoid rebinding in the subsequent draw call
            constexpr auto shader_name = "TransparentForwardShader";
            s_Data.CurrentShader = s_Data.ShaderMap[shader_name];
            s_Data.CurrentShader->Bind();
            // Rendering
            {
                ENG_TIME("Scene phase (Transparent): LOOPING");
                f_setRenderShaderName(shader_name);
                f_setVFCullingParam(true, camera->GetViewFrustumInViewSpace(), camera->GetViewMatrix());
                f_setDistanceCullingParam(false, Vec3f(), 0, 0);
                f_render();
            }
        }

        s_Data.RENDER_PASS = render_pass_original;
        s_Data.RENDER_PIPE = render_pipe_original;
        s_Data.RENDER_MODE = render_mode_original;
    }
}

/**************************************************************
*	Render3D highlevel API : EndTransparentSceneAndLighting
*
**************************************************************/
void longmarch::Renderer3D::EndTransparentSceneAndLighting()
{
}

/**************************************************************
*	Render3D highlevel API : BeginPostProcessing
*
**************************************************************/
void longmarch::Renderer3D::BeginPostProcessing()
{
    RenderCommand::PolyModeFill(); // Draw full model
    RenderCommand::DepthTest(false, false); // Disable depth testing
    RenderCommand::CullFace(false, false); // Disable face culling
    RenderCommand::Blend(false); // Disable blending

    Renderer3D::_BeginBloomPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);

    if (s_Data.enable_deferredShading)
    {
        Renderer3D::_BeginDOFPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);
        Renderer3D::_BeginMotionBlurPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);
        Renderer3D::_BeginTAAPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);
        Renderer3D::_BeginSMAAPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_3,
                                   s_Data.gpuBuffer.FrameBuffer_4, s_Data.gpuBuffer.FrameBuffer_1);
        Renderer3D::_BeginFXAAPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);
    }
    else
    {
        Renderer3D::_BeginSMAAPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_3,
                                   s_Data.gpuBuffer.FrameBuffer_4, s_Data.gpuBuffer.FrameBuffer_1);
        Renderer3D::_BeginFXAAPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.FrameBuffer_1);
    }


    {
        GPU_NAMED_SCOPE(CopyToPreviousFrameBuffer_pass);
        // Fill in prev frame buffer from current frame buffer
        RenderCommand::TransferColorBit(
            s_Data.gpuBuffer.CurrentFrameBuffer->GetFrameBufferID(),
            s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().x,
            s_Data.gpuBuffer.CurrentFrameBuffer->GetBufferSize().y,

            s_Data.gpuBuffer.PrevFinalFrameBuffer->GetFrameBufferID(),
            s_Data.gpuBuffer.PrevFinalFrameBuffer->GetBufferSize().x,
            s_Data.gpuBuffer.PrevFinalFrameBuffer->GetBufferSize().y
        );
        s_Data.gpuBuffer.PrevFinalFrameBuffer->GenerateMipMapLevel();
    }

    Renderer3D::_BeginToneMappingPass(s_Data.gpuBuffer.CurrentFrameBuffer, s_Data.gpuBuffer.CurrentFinalFrameBuffer);
}

void longmarch::Renderer3D::_BeginSSGIPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                           const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.SSGISettings.enable)
    {
        GPU_TIME(SSGI_Pass);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["DynamicSSGIBlendShader"];
        s_Data.CurrentShader->Bind();
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
        s_Data.gpuBuffer.CurrentDynamicSSAOBuffer->BindTexture(
            s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NUM));
        s_Data.gpuBuffer.CurrentDynamicSSGIBuffer->BindTexture(
            s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NUM) + 1);
        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

//void longmarch::Renderer3D::_BeginSSAOPass(const std::shared_ptr<FrameBuffer>& framebuffer_in, const std::shared_ptr<FrameBuffer>& framebuffer_out)
//{
//	if (s_Data.SSRSettings.enable)
//	{
//		RenderCommand::PolyModeFill();			// Draw full model
//		RenderCommand::DepthTest(false, false);	// Disable depth testing
//		RenderCommand::CullFace(false, false);	// Disable face culling
//		RenderCommand::Blend(false);
//
//		if (framebuffer_out)
//		{
//			framebuffer_out->Bind();
//			Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
//			RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
//		}
//		else
//		{
//			RenderCommand::BindDefaultFrameBuffer();
//			Vec2u traget_resoluation = s_Data.window_size;
//			RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
//		}
//
//		s_Data.CurrentShader = s_Data.ShaderMap["DynamicSSAOColorShader"];
//		s_Data.CurrentShader->Bind();
//		framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
//		s_Data.gpuBuffer.CurrentDynamicSSAOBuffer->BindTexture(s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NUM));
//		// Render quad
//		Renderer3D::_RenderFullScreenQuad();
//	}
//	else
//	{
//		RenderCommand::TransferColorBit(
//			framebuffer_in->GetFrameBufferID(),
//			framebuffer_in->GetBufferSize().x,
//			framebuffer_in->GetBufferSize().y,
//
//			framebuffer_out->GetFrameBufferID(),
//			framebuffer_out->GetBufferSize().x,
//			framebuffer_out->GetBufferSize().y
//		);
//	}
//	s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
//}

void longmarch::Renderer3D::_BeginSSRPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                          const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.SSRSettings.enable)
    {
        GPU_TIME(SSR_Pass);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["DynamicSSRBlendShader"];
        s_Data.CurrentShader->Bind();
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
        s_Data.gpuBuffer.CurrentDynamicSSRBuffer->BindTexture(
            s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(GBuffer::GBUFFER_TEXTURE_TYPE::NUM) + 1);
        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginTAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                          const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.enable_taa)
    {
        GPU_NAMED_SCOPE(TAA_pass);
        GPU_TIME(TAA);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["TAAShader"];
        s_Data.CurrentShader->Bind();

        s_Data.CurrentShader->SetFloat2("u_ScreenSize", framebuffer_in->GetBufferSize());
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
        s_Data.gpuBuffer.PrevFinalFrameBuffer->BindTexture(s_Data.fragTexture_1_slot);
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY,
            },
            s_Data.fragTexture_empty_slot
        );
        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginFXAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                           const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.enable_fxaa)
    {
        GPU_NAMED_SCOPE(FXAA_pass);
        GPU_TIME(FXAA);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["FXAAShader"];
        s_Data.CurrentShader->Bind();

        s_Data.CurrentShader->SetInt("enabled", true);
        s_Data.CurrentShader->SetFloat2("u_ScreenSize", framebuffer_in->GetBufferSize());
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);

        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginSMAAPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                           const std::shared_ptr<FrameBuffer>& framebuffer_edge,
                                           const std::shared_ptr<FrameBuffer>& framebuffer_blend,
                                           const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.SMAASettings.enable)
    {
        GPU_NAMED_SCOPE(SMAA_pass);
        GPU_TIME(SMAA);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        // Edge detection
        {
            RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::ALWAYS);
            RenderCommand::StencilTest(true, true); // Write to stencil for edge pass
            framebuffer_edge->Bind();
            Vec2u traget_resoluation = framebuffer_edge->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            {
                s_Data.CurrentShader = s_Data.ShaderMap["SMAAShader_edge"];
                s_Data.CurrentShader->Bind();

                s_Data.CurrentShader->SetFloat2("resolution", Vec2f(framebuffer_in->GetBufferSize()));
                framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
                // Render quad
                Renderer3D::_RenderFullScreenQuad();
            }
        }
        // Wieght calculation
        {
            RenderCommand::TransferStencilBit(
                framebuffer_edge->GetFrameBufferID(),
                framebuffer_edge->GetBufferSize().x,
                framebuffer_edge->GetBufferSize().y,

                framebuffer_blend->GetFrameBufferID(),
                framebuffer_blend->GetBufferSize().x,
                framebuffer_blend->GetBufferSize().y
            );

            RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::EQUAL);
            RenderCommand::StencilTest(true, false); // Only process fragments that were processed in the edge pass
            framebuffer_blend->Bind();
            Vec2u traget_resoluation = framebuffer_blend->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            {
                s_Data.CurrentShader = s_Data.ShaderMap["SMAAShader_weight"];
                s_Data.CurrentShader->Bind();

                s_Data.CurrentShader->SetFloat2("resolution", Vec2f(framebuffer_edge->GetBufferSize()));
                s_Data.CurrentShader->SetFloat4("subsampleIndices",
                                                s_Data.SMAASettings.GetSampleIndcies(s_Data.frameIndex));

                framebuffer_edge->BindTexture(s_Data.fragTexture_1_slot);
                s_Data.gpuBuffer.SMAAAreaLUT->BindTexture(s_Data.fragTexture_empty_slot + 1);
                s_Data.gpuBuffer.SMAASearchLUT->BindTexture(s_Data.fragTexture_empty_slot + 2);
                // Render quad
                Renderer3D::_RenderFullScreenQuad();
            }
        }
        // Blending
        {
            RenderCommand::StencilTest(false, false); // Process all fragments in the blend pass, disable stencil test
            switch (s_Data.SMAASettings.mode)
            {
            case 0: // SMAA 1X
                {
                    if (framebuffer_out)
                    {
                        framebuffer_out->Bind();
                        Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                    }
                    else
                    {
                        RenderCommand::BindDefaultFrameBuffer();
                        Vec2u traget_resoluation = s_Data.window_size;
                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                    }

                    s_Data.CurrentShader = s_Data.ShaderMap["SMAAShader_blend"];
                    s_Data.CurrentShader->Bind();

                    s_Data.CurrentShader->SetInt("enable_smaaT2X", false);
                    s_Data.CurrentShader->SetFloat2("resolution", Vec2f(framebuffer_in->GetBufferSize()));
                    framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
                    framebuffer_blend->BindTexture(s_Data.fragTexture_2_slot);

                    // Render quad
                    Renderer3D::_RenderFullScreenQuad();
                }
                break;
            case 1: // SMAA T2X
                {
                    {
                        // Write to edge buffer as temporary buffer
                        framebuffer_edge->Bind();
                        Vec2u traget_resoluation = framebuffer_edge->GetBufferSize();
                        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

                        s_Data.CurrentShader = s_Data.ShaderMap["SMAAShader_blend"];
                        s_Data.CurrentShader->Bind();

                        s_Data.CurrentShader->SetInt("enable_smaaT2X", true);
                        s_Data.CurrentShader->SetFloat2("resolution", Vec2f(framebuffer_in->GetBufferSize()));
                        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
                        framebuffer_blend->BindTexture(s_Data.fragTexture_2_slot);
                        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                            {
                                GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY,
                            },
                            s_Data.fragTexture_empty_slot
                        );

                        // Render quad
                        Renderer3D::_RenderFullScreenQuad();
                    }

                    {
                        // Blend current frame SMAA result with previous frame
                        if (framebuffer_out)
                        {
                            framebuffer_out->Bind();
                            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
                            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                        }
                        else
                        {
                            RenderCommand::BindDefaultFrameBuffer();
                            Vec2u traget_resoluation = s_Data.window_size;
                            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                        }

                        s_Data.CurrentShader = s_Data.ShaderMap["SMAAShader_blend_T2X"];
                        s_Data.CurrentShader->Bind();

                        s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
                        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                            {
                                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                                GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY,
                            },
                            s_Data.fragTexture_empty_slot
                        );

                        framebuffer_edge->BindTexture(s_Data.fragTexture_0_slot);
                        s_Data.gpuBuffer.PrevFinalFrameBuffer->BindTexture(s_Data.fragTexture_1_slot);
                        // Render quad
                        Renderer3D::_RenderFullScreenQuad();
                    }
                }
                break;
            default:
                ENGINE_EXCEPT(L"Unsupported SMAA mode!");
                break;
            }
        }
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginMotionBlurPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                                 const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.MotionBlur.enable_motionblur)
    {
        GPU_NAMED_SCOPE(MotionBlur_pass);
        GPU_TIME(Motion_Blur);
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        if (framebuffer_out)
        {
            framebuffer_out->Bind();
            Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }
        else
        {
            RenderCommand::BindDefaultFrameBuffer();
            Vec2u traget_resoluation = s_Data.window_size;
            RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
        }

        s_Data.CurrentShader = s_Data.ShaderMap["MotionBlur"];
        s_Data.CurrentShader->Bind();
        s_Data.CurrentShader->SetInt("enable_ReverseZ", s_Data.enable_reverse_z);
        s_Data.CurrentShader->SetFloat("u_VelocityScale",
                                       1.0f / s_Data.MotionBlur.motionblur_shutterSpeed / Engine::GetFrameTime());
        s_Data.CurrentShader->SetFloat2("u_ScreenSize", framebuffer_in->GetBufferSize());
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
        s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
            {
                GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                GBuffer::GBUFFER_TEXTURE_TYPE::VELOCITY,
            },
            s_Data.fragTexture_empty_slot
        );

        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginBloomPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                            const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.BloomSettings.enable)
    {
        GPU_NAMED_SCOPE(Bloom_pass);
        GPU_TIME(Bloom);
        if (auto downscale = s_Data.BloomSettings.bloom_sample_resolution_downScale;
            s_Data.gpuBuffer.CurrentDynamicBloomBuffer->GetBufferSize() != s_Data.resolution / downscale)
        // Render the Bloom with potentially downscaled resolution
        {
            s_Data.gpuBuffer.CurrentDynamicBloomBuffer = FrameBuffer::Create(
                s_Data.resolution.x / downscale, s_Data.resolution.y / downscale,
                FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        RenderCommand::DepthTest(true, true);
        RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));

        s_Data.gpuBuffer.CurrentDynamicBloomBuffer->Bind();
        RenderCommand::Clear();

        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        // Extract brightness map
        auto& BrightnessBuffer = s_Data.gpuBuffer.CurrentDynamicBloomBuffer;
        Vec2u traget_resoluation = BrightnessBuffer->GetBufferSize();
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

        BrightnessBuffer->Bind();
        {
            s_Data.CurrentShader = s_Data.ShaderMap["Bloom_Brightness_Filter"];
            s_Data.CurrentShader->Bind();

            s_Data.CurrentShader->SetFloat("u_Threshold", s_Data.BloomSettings.bloom_threshold);
            framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);

            // Render quad
            Renderer3D::_RenderFullScreenQuad();
        }

        // Blur brightness map
        const auto& guassian_shader = s_Data.ShaderMap["GaussianBlur"];
        Vec2u traget_resoluation2(traget_resoluation);
        static auto BrightnessBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.x,
                                                               FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        if (BrightnessBackBuffer->GetBufferSize() != traget_resoluation2)
        {
            BrightnessBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.y,
                                                       FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        auto kernel_size = s_Data.BloomSettings.bloom_gaussian_kernal;
#ifdef LongMarch_SCALE_GUASSIAN_KERNEL_WITH_RESOLUTION
        auto kernel_ratio = std::min(traget_resoluation2.x, traget_resoluation2.y) / 1080.0f;
        kernel_size *= kernel_ratio;
        kernel_size = std::max(kernel_size, LongMarch_GUASSIAN_KERNEL_MIN);
#endif
        auto [length, offset, weight] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
        {
            s_Data.CurrentShader = guassian_shader;
            s_Data.CurrentShader->Bind();
            s_Data.CurrentShader->SetInt("u_length", length);
            weight->Bind(1);
            offset->Bind(2);
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                // Bind shadow buffer
                BrightnessBackBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                BrightnessBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                // Bind shadow buffer
                BrightnessBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                BrightnessBackBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
        }

        // Blend original map with blurred brightness map
        {
            if (framebuffer_out)
            {
                framebuffer_out->Bind();
                Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
            else
            {
                RenderCommand::BindDefaultFrameBuffer();
                Vec2u traget_resoluation = s_Data.window_size;
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
            {
                s_Data.CurrentShader = s_Data.ShaderMap["Bloom_Blend"];
                s_Data.CurrentShader->Bind();

                s_Data.CurrentShader->SetFloat("u_Strength", s_Data.BloomSettings.bloom_blend_strength);
                framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
                BrightnessBuffer->BindTexture(s_Data.fragTexture_1_slot);

                // Render quad
                Renderer3D::_RenderFullScreenQuad();
            }
        }
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}


void longmarch::Renderer3D::_BeginDOFPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                          const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    if (s_Data.DOFSettings.enable)
    {
        GPU_NAMED_SCOPE(DOF_pass);
        GPU_TIME(DOF);
        if (auto downscale = s_Data.DOFSettings.dof_sample_resolution_downScale;
            s_Data.gpuBuffer.CurrentDynamicDOFBuffer->GetBufferSize() != s_Data.resolution / downscale)
        // Render the Bloom with potentially downscaled resolution
        {
            s_Data.gpuBuffer.CurrentDynamicDOFBuffer = FrameBuffer::Create(
                s_Data.resolution.x / downscale, s_Data.resolution.y / downscale,
                FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        RenderCommand::DepthTest(true, true);
        RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));

        s_Data.gpuBuffer.CurrentDynamicDOFBuffer->Bind();
        RenderCommand::Clear();

        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth testing
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false);

        // Transfer current color frame DOF color buffer
        auto& DOFBuffer = s_Data.gpuBuffer.CurrentDynamicDOFBuffer;
        Vec2u traget_resoluation = DOFBuffer->GetBufferSize();

        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            DOFBuffer->GetFrameBufferID(),
            traget_resoluation.x,
            traget_resoluation.y
        );

        // Update current focus depth with target depth
        {
            // Alternate current and prev depth buffer so we can always use _1 and _2 pointers for ease
            auto prevDOFDepthBuffer = s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_1;
            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_1 = s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_2;
            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_2 = prevDOFDepthBuffer;

            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_1->Bind();
            RenderCommand::SetViewport(0, 0, 1, 1);
            s_Data.CurrentShader = s_Data.ShaderMap["DOF_Dpeth"];
            s_Data.CurrentShader->Bind();

            s_Data.CurrentShader->SetFloat2("u_ss_target", s_Data.DOFSettings.ss_target);
            s_Data.CurrentShader->SetFloat("u_Refocus_rate", s_Data.DOFSettings.dof_refocus_rate);
            s_Data.CurrentShader->SetFloat("u_dt", Engine::GetFrameTime());
            s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_2->BindTexture(s_Data.fragTexture_0_slot);
            // use 2 as the prev focus depth buffer

            s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                {
                    GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                },
                s_Data.fragTexture_empty_slot
            );

            // Render quad
            Renderer3D::_RenderFullScreenQuad();
        }

        // Blur DOF map
        const auto& guassian_shader = s_Data.ShaderMap["GaussianBlur"];
        Vec2u traget_resoluation2(traget_resoluation);
        static auto BrightnessBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.x,
                                                               FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        if (BrightnessBackBuffer->GetBufferSize() != traget_resoluation2)
        {
            BrightnessBackBuffer = FrameBuffer::Create(traget_resoluation2.x, traget_resoluation2.y,
                                                       FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);
        }
        auto kernel_size = s_Data.DOFSettings.dof_gaussian_kernal;
#ifdef LongMarch_SCALE_GUASSIAN_KERNEL_WITH_RESOLUTION
        auto kernel_ratio = std::min(traget_resoluation2.x, traget_resoluation2.y) / 1080.0f;
        kernel_size *= kernel_ratio;
        kernel_size = std::max(kernel_size, LongMarch_GUASSIAN_KERNEL_MIN);
#endif
        auto [length, offset, weight] = s_Data.gpuBuffer.GuassinKernelHalfBilinear[kernel_size];
        {
            s_Data.CurrentShader = guassian_shader;
            s_Data.CurrentShader->Bind();
            s_Data.CurrentShader->SetInt("u_length", length);
            weight->Bind(1);
            offset->Bind(2);
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation2.x, traget_resoluation2.y);
                // Bind shadow buffer
                BrightnessBackBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 1);
                DOFBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
            {
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
                // Bind shadow buffer
                DOFBuffer->Bind();
                s_Data.CurrentShader->SetInt("u_Horizontal", 0);
                BrightnessBackBuffer->BindTexture(s_Data.fragTexture_0_slot);
                _RenderFullScreenQuad();
            }
        }
        DOFBuffer->GenerateMipMapLevel();

        // Blend original map with blurred brightness map
        {
            if (framebuffer_out)
            {
                framebuffer_out->Bind();
                Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
            else
            {
                RenderCommand::BindDefaultFrameBuffer();
                Vec2u traget_resoluation = s_Data.window_size;
                RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
            }
            {
                s_Data.CurrentShader = s_Data.ShaderMap["DOF_Blend"];
                s_Data.CurrentShader->Bind();
                s_Data.CurrentShader->SetInt("enabled_debug", s_Data.DOFSettings.enable_debug);
                s_Data.CurrentShader->SetFloat("u_Threashold", s_Data.DOFSettings.dof_threshold);
                s_Data.CurrentShader->SetFloat("u_Strength", s_Data.DOFSettings.dof_blend_strength);
                framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);
                DOFBuffer->BindTexture(s_Data.fragTexture_1_slot);
                s_Data.gpuBuffer.CurrentDynamicDOFDepthBuffer_1->BindTexture(s_Data.fragTexture_2_slot);

                s_Data.gpuBuffer.CurrentGBuffer->BindTextures(
                    {
                        GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH,
                    },
                    s_Data.fragTexture_empty_slot
                );

                // Render quad
                Renderer3D::_RenderFullScreenQuad();
            }
        }
    }
    else
    {
        RenderCommand::TransferColorBit(
            framebuffer_in->GetFrameBufferID(),
            framebuffer_in->GetBufferSize().x,
            framebuffer_in->GetBufferSize().y,

            framebuffer_out->GetFrameBufferID(),
            framebuffer_out->GetBufferSize().x,
            framebuffer_out->GetBufferSize().y
        );
    }
    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

void longmarch::Renderer3D::_BeginToneMappingPass(const std::shared_ptr<FrameBuffer>& framebuffer_in,
                                                  const std::shared_ptr<FrameBuffer>& framebuffer_out)
{
    GPU_NAMED_SCOPE(ToneMapping_pass);
    GPU_TIME(Tone_Mapping);
    RenderCommand::PolyModeFill(); // Draw full model
    RenderCommand::DepthTest(false, false); // Disable depth testing
    RenderCommand::CullFace(false, false); // Disable face culling
    RenderCommand::Blend(false);

    if (framebuffer_out)
    {
        framebuffer_out->Bind();
        Vec2u traget_resoluation = framebuffer_out->GetBufferSize();
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
    }
    else
    {
        RenderCommand::BindDefaultFrameBuffer();
        Vec2u traget_resoluation = s_Data.window_size;
        RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
    }
    {
        s_Data.CurrentShader = s_Data.ShaderMap["ToneMapping"];
        s_Data.CurrentShader->Bind();

        s_Data.CurrentShader->SetInt("u_ToneMappingMethodMode", s_Data.toneMapping_mode);
        framebuffer_in->BindTexture(s_Data.fragTexture_0_slot);

        // Render quad
        Renderer3D::_RenderFullScreenQuad();
    }

    s_Data.gpuBuffer.CurrentFrameBuffer = framebuffer_out;
}

/**************************************************************
*	Render3D highlevel API : EndPostProcessing
*
**************************************************************/
void longmarch::Renderer3D::EndPostProcessing()
{
}

/**************************************************************
*	Render3D highlevel API : EndRendering
*
**************************************************************/
void longmarch::Renderer3D::EndRendering()
{
}

void longmarch::Renderer3D::SubmitFrameBufferToScreen()
{
    /*
        Blit CurrentFinalFrameBuffer to have the same size as the back buffer.
        This allows us to render with a smaller resolution thant the screen
    */
    constexpr int default_framebuffer_rendererID = 0;
    RenderCommand::TransferColorBit(
        s_Data.gpuBuffer.CurrentFinalFrameBuffer->GetFrameBufferID(),
        s_Data.gpuBuffer.CurrentFinalFrameBuffer->GetBufferSize().x,
        s_Data.gpuBuffer.CurrentFinalFrameBuffer->GetBufferSize().y,

        default_framebuffer_rendererID,
        s_Data.window_size.x,
        s_Data.window_size.y
    );
}

void longmarch::Renderer3D::_RenderFullScreenQuad()
{
    // // Render quad
    // s_Data.gpuBuffer.FullScreenQuadVAO->Bind();
    // RenderCommand::DrawTriangleIndexed(s_Data.gpuBuffer.FullScreenQuadVAO);

    // Render tri instead of quad
    s_Data.gpuBuffer.FullScreenTriVAO->Bind();
    RenderCommand::DrawTriangleIndexed(s_Data.gpuBuffer.FullScreenTriVAO);
}

void longmarch::Renderer3D::_RenderFullScreenCube()
{
    // Render cube
    s_Data.gpuBuffer.FullScreenCubeVAO->Bind();
    RenderCommand::DrawTriangleIndexed(s_Data.gpuBuffer.FullScreenCubeVAO);
}

/**************************************************************
*	Render3D lowleve API : RenderBoundingBox
*
**************************************************************/
void longmarch::Renderer3D::RenderBoundingBox(const Mat4& transform)
{
    LOCK_GUARD_S();
    // Storing transformation matrix for delayed renderering
    s_Data.cpuBuffer.InstancedDraw_BVModelTr.emplace_back(transform);
}

/**************************************************************
*   Render3D lowleve API : Draw
*   This Draw function is for custom render pass to involve in
*   the multidraw pipeline
**************************************************************/
void longmarch::Renderer3D::Draw(const RenderData_CPU& data)
{
    switch (Renderer3D::s_Data.RENDER_MODE)
    {
    case Renderer3D::RENDER_MODE::MULTIDRAW:
        {
            auto& _multiDrawBuffer = Renderer3D::s_Data.multiDrawBuffer;
            _multiDrawBuffer.MultiDraw_MeshDataToDraw[data.mesh].emplace_back(
                _multiDrawBuffer.MultiDraw_MeshDataToDrawIndexCounter++);
            _multiDrawBuffer.MultiDraw_BoneBaseOffset.emplace_back(0);
            // TODO : need update RenderData_CPU to include animation
        }
        break;
    case Renderer3D::RENDER_MODE::CANONICAL:
        {
            data.mesh->Draw();
        }
        break;
    }
}

/**************************************************************
*   Render3D lowleve API : Draw
*   This Draw function is for component to drive scene drawing
*   the multidraw pipeline
**************************************************************/
void longmarch::Renderer3D::Draw(Entity entity, const std::shared_ptr<Scene3DNode>& sceneNode,
                                 const Mat4& transform,
                                 const Mat4& PrevTransform, const std::string& shaderName)
{
    const auto& _sceneData = *sceneNode;
    const auto& boneTransform = _sceneData.GetInverseFinalBoneTransform();
    switch (s_Data.RENDER_MODE)
    {
    case RENDER_MODE::MULTIDRAW:
        {
            auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
            const auto offset = _multiDrawBuffer.MultiDraw_BoneTransformMatrix.size();
            for (const auto& [level, data] : _sceneData)
            {
                const auto& mesh = data->meshData;
                const auto& mat = data->material;
                Renderer3D::Draw(entity, mesh, mat, transform, PrevTransform, shaderName);

                // Bone offset need to placed after Draw() since there is a chance for multidraw to commit early within Draw()
                _multiDrawBuffer.MultiDraw_BoneBaseOffset.emplace_back(offset);
            }

            // Bone offset need to placed after Draw() since there is a chance for multidraw to commit early within Draw() 
            std::ranges::copy(boneTransform.begin(), boneTransform.end(),
                              std::back_inserter(_multiDrawBuffer.MultiDraw_BoneTransformMatrix));
        }
        break;
    case RENDER_MODE::CANONICAL:
        {
            for (const auto& [level, data] : _sceneData)
            {
                const auto& mesh = data->meshData;
                const auto& mat = data->material;
                {
                    // upload bone ssbo
                    auto& gpubuffer = s_Data.gpuBuffer;
                    if (!boneTransform.empty())
                    {
                        auto ptr = &(boneTransform[0][0][0]);
                        auto size = boneTransform.size() * sizeof(Mat4);
                        gpubuffer.BoneTransformMatrixBuffer->UpdateBufferData(ptr, size);
                        gpubuffer.BoneTransformMatrixBuffer->Bind(5); // TODO use serialized binding location
                    }
                    else
                    {
                        auto ptr = &(GPUBuffer::s_default_bone_transform[0][0][0]);
                        auto size = GPUBuffer::s_default_bone_transform.size() * sizeof(Mat4);
                        gpubuffer.BoneTransformMatrixBuffer->UpdateBufferData(ptr, size);
                        gpubuffer.BoneTransformMatrixBuffer->Bind(5); // TODO use serialized binding location
                    }
                }
                Renderer3D::Draw(entity, mesh, mat, transform, PrevTransform, shaderName);
            }
        }
        break;
    };
}

/**************************************************************
*	Render3D lowleve API : Draw
*   This Draw function is for mesh drawing with materials 
*	and default shaders
**************************************************************/
void longmarch::Renderer3D::Draw(Entity entity, const std::shared_ptr<MeshData>& Mesh,
                                 const std::shared_ptr<Material>& Mat, const Mat4& transform, const Mat4& PrevTransform,
                                 const std::string& shaderName)
{
    LOCK_GUARD_S(); // Lock the drawing or pushing draw commends
    if (auto it = s_Data.ShaderMap.find(shaderName); it == s_Data.ShaderMap.end())
    [[unlikely]]
    {
        ENGINE_EXCEPT(L"Shader called " + wStr(shaderName) + L" has not been managed!");
    }
    else if (s_Data.CurrentShader != it->second)
    {
        s_Data.CurrentShader = it->second;
        s_Data.CurrentShader->Bind();
    }
    switch (s_Data.RENDER_PASS)
    {
    case RENDER_PASS::SHADOW:
        {
            switch (s_Data.RENDER_MODE)
            {
            case RENDER_MODE::MULTIDRAW:
                {
                    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
                    if (_multiDrawBuffer.MultiDraw_ShadowModelTr.size() >= s_Data.MAX_SHADOW_BATCH)
                    {
                        _RenderBatch();
                    }
                    _multiDrawBuffer.MultiDraw_ShadowModelTr.emplace_back(transform);
                    _multiDrawBuffer.MultiDraw_MeshDataToDraw[Mesh].emplace_back(
                        _multiDrawBuffer.MultiDraw_MeshDataToDrawIndexCounter++); // Push instance ID
                }
                break;
            case RENDER_MODE::CANONICAL:
                {
                    s_Data.CurrentShader->SetMat4("u_ModleTr", transform);
                    Mesh->Draw();
                }
                break;
            }
        }
        break;
    case RENDER_PASS::SCENE:
        {
            ModelBuffer_GPU model_Buffer(PrevTransform, transform, Geommath::SmartInverseTranspose(transform));
            MaterialBuffer_GPU material_Buffer(Mat);

            switch (s_Data.RENDER_MODE)
            {
            case RENDER_MODE::MULTIDRAW:
                {
                    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
                    auto& textureIds = _multiDrawBuffer.MultiDraw_TextureId;
                    auto scene_limit = s_Data.MAX_SCENE_BATCH - LongMarch_ToUnderlying(Material::MAT_TEXTURE_TYPE::NUM);
                    // The scene pass batch limit is set by the fixed array of texture lists, we need to substract a few number to make sure never hit that hard limit
                    if (textureIds.size() >= scene_limit || _multiDrawBuffer.MultiDraw_ModelBuffer.size() >= TWO_11)
                    {
                        _RenderBatch();
                    }
                    uint32_t texture_offset = textureIds.size();
                    ASSERT(texture_offset != 0,
                           "MultiDraw_TextureId should have at least size of 1 (sudo empty texture) on rendering!");
                    auto& uniqueTextureLut = _multiDrawBuffer.MultiDraw_UniqueTextureLUT;
                    // Multidraw allows to draw multiple materials with the same shader in the same time, and we only need unique materials to be emplaced.
                    LongMarch_Vector<std::pair<uint32_t, Material::MAT_TEXTURE_TYPE>> textures_to_bind;
                    if (Mat->textures.has_albedo())
                    {
                        auto& offset = uniqueTextureLut[Mat->textures.albedo_texture->Get()];
                        if (offset == 0) // only emplace new unique texture
                        {
                            offset = texture_offset++;
                            textureIds.emplace_back(s_Data.fragTexture_empty_slot + offset);
                            textures_to_bind.emplace_back(s_Data.fragTexture_empty_slot + offset,
                                                          Material::MAT_TEXTURE_TYPE::ALBEDO);
                        }
                        material_Buffer.AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.x = offset;
                    }
                    if (Mat->textures.has_normal())
                    {
                        auto& offset = uniqueTextureLut[Mat->textures.normal_texture->Get()];
                        if (offset == 0) // only emplace new unique texture
                        {
                            offset = texture_offset++;
                            textureIds.emplace_back(s_Data.fragTexture_empty_slot + offset);
                            textures_to_bind.emplace_back(s_Data.fragTexture_empty_slot + offset,
                                                          Material::MAT_TEXTURE_TYPE::NORMAL);
                        }
                        material_Buffer.AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.y = offset;
                    }
                    if (Mat->textures.has_metallic())
                    {
                        auto& offset = uniqueTextureLut[Mat->textures.metallic_texture->Get()];
                        if (offset == 0) // only emplace new unique texture
                        {
                            offset = texture_offset++;
                            textureIds.emplace_back(s_Data.fragTexture_empty_slot + offset);
                            textures_to_bind.emplace_back(s_Data.fragTexture_empty_slot + offset,
                                                          Material::MAT_TEXTURE_TYPE::METALLIC);
                        }
                        material_Buffer.AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.z = offset;
                    }
                    if (Mat->textures.has_roughness())
                    {
                        auto& offset = uniqueTextureLut[Mat->textures.roughness_texture->Get()];
                        if (offset == 0) // only emplace new unique texture
                        {
                            offset = texture_offset++;
                            textureIds.emplace_back(s_Data.fragTexture_empty_slot + offset);
                            textures_to_bind.emplace_back(s_Data.fragTexture_empty_slot + offset,
                                                          Material::MAT_TEXTURE_TYPE::ROUGHNESS);
                        }
                        material_Buffer.AlbedoTexIndex_NormalTexIndex_MetallicTexIndex_RoughnessTexIndex.w = offset;
                    }
                    if (Mat->textures.has_ao())
                    {
                        auto& offset = uniqueTextureLut[Mat->textures.ao_texture->Get()];
                        if (offset == 0) // only emplace new unique texture
                        {
                            offset = texture_offset++;
                            textureIds.emplace_back(s_Data.fragTexture_empty_slot + offset);
                            textures_to_bind.emplace_back(s_Data.fragTexture_empty_slot + offset,
                                                          Material::MAT_TEXTURE_TYPE::BACKEDAO);
                        }
                        material_Buffer.Metallic_Roughness_IsAlbedoSRBG_AOTextureIndex.w = offset;
                    }
                    // Only emplace non empty texture_to_bind
                    if (!textures_to_bind.empty())
                    {
                        _multiDrawBuffer.MultiDraw_MaterialTexToBind.emplace_back(Mat, std::move(textures_to_bind));
                    }
                    _multiDrawBuffer.MultiDraw_ModelBuffer.emplace_back(std::move(model_Buffer)); // Push model buffer
                    _multiDrawBuffer.MultiDraw_MaterialBuffer.emplace_back(std::move(material_Buffer));
                    // Push material buffer
                    _multiDrawBuffer.MultiDraw_MeshDataToDraw[Mesh].emplace_back(
                        _multiDrawBuffer.MultiDraw_MeshDataToDrawIndexCounter++); // Push instance ID
                }
                break;
            case RENDER_MODE::CANONICAL:
                {
                    {
                        // bind all texture
                        Mat->BindAllTexture({
                            {
                                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                    Material::MAT_TEXTURE_TYPE::ALBEDO),
                                Material::MAT_TEXTURE_TYPE::ALBEDO
                            },
                            {
                                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                    Material::MAT_TEXTURE_TYPE::NORMAL),
                                Material::MAT_TEXTURE_TYPE::NORMAL
                            },
                            {
                                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                    Material::MAT_TEXTURE_TYPE::BACKEDAO),
                                Material::MAT_TEXTURE_TYPE::BACKEDAO
                            },
                            {
                                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                    Material::MAT_TEXTURE_TYPE::METALLIC),
                                Material::MAT_TEXTURE_TYPE::METALLIC
                            },
                            {
                                s_Data.fragTexture_empty_slot + LongMarch_ToUnderlying(
                                    Material::MAT_TEXTURE_TYPE::ROUGHNESS),
                                Material::MAT_TEXTURE_TYPE::ROUGHNESS
                            },
                        });
                    }
                    auto& gpubuffer = s_Data.gpuBuffer;
                    // upload model and material ubo
                    gpubuffer.CurrentModelBuffer->UpdateBufferData(model_Buffer.GetPtr(), sizeof(model_Buffer));
                    gpubuffer.CurrentModelBuffer->Bind(1);
                    gpubuffer.CurrentMaterialBuffer->
                              UpdateBufferData(material_Buffer.GetPtr(), sizeof(material_Buffer));
                    gpubuffer.CurrentMaterialBuffer->Bind(2);
                    // Bind Skybox
                    _BindSkyBoxTexture();
                    {
                        // set transparent alpha
                        s_Data.CurrentShader->SetFloat("u_Alpha", Mat->alpha);
                    }
                    Mesh->Draw();
                }
                break;
            }
        }
        break;
    }
}

/**************************************************************
*	Render3D lowleve API : DrawMesh
*
**************************************************************/
void longmarch::Renderer3D::DrawMesh(const std::shared_ptr<VertexArray>& MeshVertexArray)
{
    // Canoical rendering method that allow each mesh to store its own VAO
    MeshVertexArray->Bind();
    RenderCommand::DrawTriangleIndexed(MeshVertexArray);
}

/**************************************************************
*	Render3D lowleve API : DrawParticles
*
**************************************************************/
void longmarch::Renderer3D::DrawParticles(const ParticleInstanceDrawData& particleData)
{
    switch (s_Data.RENDER_PASS)
    {
    case RENDER_PASS::SHADOW:
        if (s_Data.CurrentShader == s_Data.ShaderMap["ShadowBuffer_Transparent"])
        {
            s_Data.CurrentShader = s_Data.ShaderMap["ShadowBuffer_Particle"];
        }
        else if (s_Data.CurrentShader == s_Data.ShaderMap["MSMShadowBuffer_Transparent"])
        {
            s_Data.CurrentShader = s_Data.ShaderMap["MSMShadowBuffer_Particle"];
        }
        else
        {
            return;
        }
        break;
    case RENDER_PASS::SCENE:
        s_Data.CurrentShader = s_Data.ShaderMap["ParticleShader"];
        break;
    default:
        ENGINE_EXCEPT(L"Particles are not avaiable for other render passes.");
        break;
    }
    s_Data.CurrentShader->Bind();

    for (auto& [texture, instanceData] : particleData)
    {
        texture->BindTexture(0);
        s_Data.CurrentShader->SetFloat("rows", instanceData.textureRows);

        int pointer = 0;
        float* data = new float[instanceData.models.size() * Renderer3D::PARTICLE_INSTANCED_DATA_LENGTH];
        for (size_t i = 0; i < instanceData.models.size(); ++i)
        {
            const auto& matrix = instanceData.models[i];
            data[pointer++] = matrix[0][0];
            data[pointer++] = matrix[0][1];
            data[pointer++] = matrix[0][2];
            data[pointer++] = matrix[0][3];
            data[pointer++] = matrix[1][0];
            data[pointer++] = matrix[1][1];
            data[pointer++] = matrix[1][2];
            data[pointer++] = matrix[1][3];
            data[pointer++] = matrix[2][0];
            data[pointer++] = matrix[2][1];
            data[pointer++] = matrix[2][2];
            data[pointer++] = matrix[2][3];
            data[pointer++] = matrix[3][0];
            data[pointer++] = matrix[3][1];
            data[pointer++] = matrix[3][2];
            data[pointer++] = matrix[3][3];

            const Vec4f& offsets = instanceData.textureOffsets[i];
            data[pointer++] = offsets.x;
            data[pointer++] = offsets.y;
            data[pointer++] = offsets.z;
            data[pointer++] = offsets.w;

            const float blendFactor = instanceData.blendFactors[i];
            data[pointer++] = blendFactor;
        }
        DrawParticlesInstance(data, instanceData.models.size());
        delete[] data;
    }
}

/**************************************************************
*	Render3D lowleve API : DrawParticlesInstance
*
**************************************************************/
void longmarch::Renderer3D::DrawParticlesInstance(float* data, const int instanceCount)
{
    s_Data.gpuBuffer.particleInstBO->UpdateBufferData(
        data, instanceCount * PARTICLE_INSTANCED_DATA_LENGTH * sizeof(float));
    s_Data.gpuBuffer.particleVAO->Bind();
    RenderCommand::DrawTriangleIndexedInstanced(s_Data.gpuBuffer.particleVAO, instanceCount);
}

/**************************************************************
*	Render3D private method : Render batch
*
**************************************************************/
void longmarch::Renderer3D::_RenderBatch()
{
    bool shouldDraw;
    {
        ENG_TIME("BATCH Prepare");
        shouldDraw = _BeginRenderBatch();
    }
    if (shouldDraw)
    {
        ENG_TIME("BATCH Draw");
        auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
        _multiDrawBuffer.MultiDraw_CBO->Bind();
        _multiDrawBuffer.MultiDraw_VAO->Bind();
        RenderCommand::MultiDrawTriangleIndexedIndirect(_multiDrawBuffer.MultiDraw_VAO, _multiDrawBuffer.MultiDraw_CBO);
    }
    _EndRenderBatch();
}

/**************************************************************
*	Render3D private API : Begin batch
*
**************************************************************/
bool longmarch::Renderer3D::_BeginRenderBatch()
{
    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
    if (auto num_instances = _multiDrawBuffer.MultiDraw_MeshDataToDrawIndexCounter;
        num_instances == 0)
    {
        // Early quit with nothing to draw
        return false;
    }
    auto& cmd_map = _multiDrawBuffer.MeshData_CmdBuffer_Map;
    auto& mesh_list = _multiDrawBuffer.MultiDraw_MeshDataToDraw;
    static LongMarch_Vector<uint32_t> instance_indices;
    instance_indices.clear();
    instance_indices.reserve(TWO_11);
    uint32_t instance_count = 0;
    {
        //ENG_TIME("Prepare instance buffer");
        for (auto&& [mesh, mesh_instance_indices] : mesh_list)
        {
            auto it = cmd_map.find(mesh);
            ENGINE_EXCEPT_IF(it == cmd_map.end(),
                             L" MeshData is not registered to commend buffer! Call Renderer3D::BuildAllMesh() before drawing after loading new meshes!");
            // Push draw command
            auto& cmd = it->second;
            uint32_t num_instances = mesh_instance_indices.size();
            _multiDrawBuffer.MultiDraw_CmdBuffer.emplace_back(cmd.indexCount, num_instances, cmd.firstIndex,
                                                              cmd.baseVertex, instance_count);
            // Push instance indicies
            instance_count += num_instances;
            std::move(mesh_instance_indices.begin(), mesh_instance_indices.end(), std::back_inserter(instance_indices));
        }
    }
    {
        // Update instance buffer
        auto ptr = &(instance_indices[0]);
        auto size = instance_indices.size() * sizeof(uint32_t);
        _multiDrawBuffer.MultiDraw_InstBO->UpdateBufferData(ptr, size);
    }
    {
        // Update bone offset buffer
        auto ptr = &(_multiDrawBuffer.MultiDraw_BoneBaseOffset[0]);
        auto size = _multiDrawBuffer.MultiDraw_BoneBaseOffset.size() * sizeof(uint32_t);
        _multiDrawBuffer.MultiDraw_ssbo_BoneBaseOffset->UpdateBufferData(ptr, size);
        _multiDrawBuffer.MultiDraw_ssbo_BoneBaseOffset->Bind(6); // TODO use serialized binding location
    }
    {
        // Update bone transform buffer
        auto& gpubuffer = s_Data.gpuBuffer;
        if (!_multiDrawBuffer.MultiDraw_BoneTransformMatrix.empty())
        {
            auto ptr = &(_multiDrawBuffer.MultiDraw_BoneTransformMatrix[0][0][0]);
            auto size = _multiDrawBuffer.MultiDraw_BoneTransformMatrix.size() * sizeof(Mat4);
            gpubuffer.BoneTransformMatrixBuffer->UpdateBufferData(ptr, size);
            gpubuffer.BoneTransformMatrixBuffer->Bind(5); // TODO use serialized binding location
        }
        else
        {
            auto ptr = &(GPUBuffer::s_default_bone_transform[0][0][0]);
            auto size = GPUBuffer::s_default_bone_transform.size() * sizeof(Mat4);
            gpubuffer.BoneTransformMatrixBuffer->UpdateBufferData(ptr, size);
            gpubuffer.BoneTransformMatrixBuffer->Bind(5); // TODO use serialized binding location
        }
    }
    {
        // Update draw command buffer
        auto ptr = &(_multiDrawBuffer.MultiDraw_CmdBuffer[0]);
        auto size = _multiDrawBuffer.MultiDraw_CmdBuffer.size() * sizeof(DrawIndexedIndirectCommand);
        _multiDrawBuffer.MultiDraw_CBO->UpdateBufferData(ptr, size);
    }
    {
        // update and bind SSBO
        switch (s_Data.RENDER_PASS)
        {
        case RENDER_PASS::SHADOW:
            {
                auto ptr = &(_multiDrawBuffer.MultiDraw_ShadowModelTr[0]);
                auto size = _multiDrawBuffer.MultiDraw_ShadowModelTr.size() * sizeof(Mat4);
                _multiDrawBuffer.MultiDraw_ssbo_ShadowModelTrsBuffer->UpdateBufferData(ptr, size);
                _multiDrawBuffer.MultiDraw_ssbo_ShadowModelTrsBuffer->Bind(1); // TODO use serialized binding location
            }
            break;
        case RENDER_PASS::SCENE:
            {
                {
                    auto ptr = &(_multiDrawBuffer.MultiDraw_ModelBuffer[0]);
                    auto size = _multiDrawBuffer.MultiDraw_ModelBuffer.size() * sizeof(ModelBuffer_GPU);
                    _multiDrawBuffer.MultiDraw_ssbo_ModelTrsBuffer->UpdateBufferData(ptr, size);
                    _multiDrawBuffer.MultiDraw_ssbo_ModelTrsBuffer->Bind(3); // TODO use serialized binding location
                }
                {
                    auto ptr = &(_multiDrawBuffer.MultiDraw_MaterialBuffer[0]);
                    auto size = _multiDrawBuffer.MultiDraw_MaterialBuffer.size() * sizeof(MaterialBuffer_GPU);
                    _multiDrawBuffer.MultiDraw_ssbo_MaterialsBuffer->UpdateBufferData(ptr, size);
                    _multiDrawBuffer.MultiDraw_ssbo_MaterialsBuffer->Bind(4); // TODO use serialized binding location
                }
                {
                    for (const auto& [Mat, texture_index_type] : _multiDrawBuffer.MultiDraw_MaterialTexToBind)
                    {
                        Mat->BindAllTexture(texture_index_type);
                    }
                    Material::s_placeholder_Texture2D->BindTexture(s_Data.fragTexture_empty_slot);
                    {
                        auto size = _multiDrawBuffer.MultiDraw_TextureId.size();
                        /**************************************************************
                        *	CRITICAL: need to fill in all empty texture id with some dummy texture slot (e.g. 2) with an empty texture
                        *	If not, then unset samplers will be initialized as 0. Shaders will crash at run time if you bind 0 to another sampler type (e.g. samplerCube)!
                        **************************************************************/
                        if (size < s_Data.MAX_SCENE_BATCH)
                        [[likely]]
                        {
                            std::fill_n(std::back_inserter(_multiDrawBuffer.MultiDraw_TextureId),
                                        s_Data.MAX_SCENE_BATCH - size, s_Data.fragTexture_empty_slot);
                        }
                        else if (size > s_Data.MAX_SCENE_BATCH)
                        [[unlikely]]
                        {
                            throw EngineException(
                                _CRT_WIDE(__FILE__), __LINE__,
                                L"Drawing scene batch larger " + wStr(size) + L" than the maxmum size!");
                        }
                    }
                    auto ptr = &(_multiDrawBuffer.MultiDraw_TextureId[0]);
                    auto size = s_Data.MAX_SCENE_BATCH;
                    s_Data.CurrentShader->SetIntV("u_materialTextureList", size, ptr);
                }
                {
                    // Bind Skybox
                    _BindSkyBoxTexture();
                }
            }
            break;
        }
        if (_multiDrawBuffer.multiDrawRenderPassCallback.submitCallback)
        {
            (*_multiDrawBuffer.multiDrawRenderPassCallback.submitCallback)();
        }
    }
    return true;
}

/**************************************************************
*	Render3D private API : End batch
*
**************************************************************/
void longmarch::Renderer3D::_EndRenderBatch()
{
    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
    // Reset
    switch (s_Data.RENDER_PASS)
    {
    case RENDER_PASS::SHADOW:
        {
            _multiDrawBuffer.MultiDraw_ShadowModelTr.clear();
        }
        break;
    case RENDER_PASS::SCENE:
        {
            _multiDrawBuffer.MultiDraw_ModelBuffer.clear();
            _multiDrawBuffer.MultiDraw_MaterialBuffer.clear();
            _multiDrawBuffer.MultiDraw_MaterialTexToBind.clear();
            _RestBatchTextureList();
        }
        break;
    }
    if (_multiDrawBuffer.multiDrawRenderPassCallback.clearCallback)
    {
        (*_multiDrawBuffer.multiDrawRenderPassCallback.clearCallback)();
    }
    // Shared multidraw
    _multiDrawBuffer.MultiDraw_BoneBaseOffset.clear();
    _multiDrawBuffer.MultiDraw_BoneTransformMatrix.clear();
    _multiDrawBuffer.MultiDraw_CmdBuffer.clear();
    _multiDrawBuffer.MultiDraw_MeshDataToDraw.clear();
    _multiDrawBuffer.MultiDraw_MeshDataToDrawIndexCounter = 0u;
}

void longmarch::Renderer3D::_RestBatchTextureList()
{
    s_Data.multiDrawBuffer.MultiDraw_TextureId.clear();
    s_Data.multiDrawBuffer.MultiDraw_TextureId.emplace_back(s_Data.fragTexture_empty_slot);
    //! Always emplace an emppty slot for an empty texture
    s_Data.multiDrawBuffer.MultiDraw_UniqueTextureLUT.clear();
}

void longmarch::Renderer3D::BuildAllMesh()
{
    switch (s_Data.RENDER_MODE)
    {
    case RENDER_MODE::MULTIDRAW:
        {
            const auto& allnodes = ResourceManager<Scene3DNode>::GetInstance()->GetAllResources();
            LongMarch_Vector<std::shared_ptr<MeshData>> allMeshData;
            allMeshData.reserve(allnodes.size() * 2);
            for (const auto& node : allnodes)
            {
                auto meshes = node->GetAllMesh();
                std::move(meshes.begin(), meshes.end(), std::back_inserter(allMeshData));
            }
            UpdateMeshToMultiDraw(allMeshData);
        }
        break;
    case RENDER_MODE::CANONICAL:
        {
            /*Nothing to init, use lazy init instead*/
        }
        break;
    }
}

void longmarch::Renderer3D::BuildAllMaterial()
{
}

void longmarch::Renderer3D::BuildAllTexture()
{
    // Environment cubemaps and their irradiance maps
    {
        RenderCommand::PolyModeFill(); // Draw full model
        RenderCommand::DepthTest(false, false); // Disable depth
        RenderCommand::CullFace(false, false); // Disable face culling
        RenderCommand::Blend(false); // Disable blending
        RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0)); // Clear to black background

        // SMAA
        {
            if (!s_Data.gpuBuffer.SMAAAreaLUT)
            {
                auto tex = ResourceManager<Texture2D>::GetInstance()->TryGet("smaa_area")->TryGet();
                if (tex)
                {
                    s_Data.gpuBuffer.SMAAAreaLUT = tex;
                }
                else
                {
                    ENGINE_EXCEPT(L"smaa_area should be loaded here!");
                }
            }
            if (!s_Data.gpuBuffer.SMAASearchLUT)
            {
                auto tex = ResourceManager<Texture2D>::GetInstance()->TryGet("smaa_search")->TryGet();
                if (tex)
                {
                    s_Data.gpuBuffer.SMAASearchLUT = tex;
                }
                else
                {
                    ENGINE_EXCEPT(L"smaa_search should be loaded here!");
                }
            }
        }

        /**************************************************************
        *	 1. Load equirectangular env maps
        **************************************************************/
        {
            for (auto& item : s_Data.gpuBuffer.EnvEquiMaps)
            {
                const auto& name = item.first;
                auto& env_map = item.second;
                if (auto tex = ResourceManager<Texture2D>::GetInstance()->TryGet(name)->TryGet(); tex)
                {
                    env_map = tex;
                }
                else
                {
                    ENGINE_EXCEPT(wStr(name) + L" skybox should be loaded here!");
                }
            }
        }
        /**************************************************************
        *	 2. Convert env maps to env cubemaps
        **************************************************************/
        {
            // x - right, y - up, z - front (opengl coordinate) used in SampleSphericalMap() in Equirectangular_To_Cubemap shader
            // const auto& tr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, -PI * 0.5)* Geommath::World2OpenGLTr* Geommath::FlipX* Geommath::FlipY;
            // x - right, y - front, z - up (my world coordinate) used in SampleSphericalMap() in Equirectangular_To_Cubemap shader
            const auto& tr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI * 0.5) * Geommath::FlipZ;
            PointLightPVMatrix_GPU matrices;
            for (int i = 0; i < 6; ++i)
            {
                const auto& light_view_mat = Geommath::LookAt(Vec3f(0), Vec3f(0) + s_Data.cube_directions[i].xyz,
                                                              s_Data.cube_ups[i]) * tr;
                const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                       ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                           90 * DEG2RAD, 1, 0.1, 10)
                                                       : Geommath::ProjectionMatrixZeroOne(90 * DEG2RAD, 1, 0.1, 10);
                matrices.PVMatrices[i] = std::move(light_projection_mat * light_view_mat);
            }

            auto pvMatrixBuffer = UniformBuffer::Create(matrices.GetPtr(), sizeof(PointLightPVMatrix_GPU));
            s_Data.CurrentShader = s_Data.ShaderMap["Equirectangular_To_Cubemap"];
            s_Data.CurrentShader->Bind();
            pvMatrixBuffer->Bind(0);

            for (auto& item : s_Data.gpuBuffer.EnvCubeMaps)
            {
                const std::string& name = item.first;
                auto& skybox_buffer_map = item.second;
                const auto& equirectangle_image = s_Data.gpuBuffer.EnvEquiMaps[name];
                uint32_t cube_width = glm::roundEven(sqrtf(3.0f) / 6.0f * equirectangle_image->GetWidth());
                cube_width = NEAREST_POW2(cube_width);
                // use nearest power of 2, even though the total pixels after convertion is not equal
                skybox_buffer_map["original"] = SkyBoxBuffer::Create(cube_width, cube_width,
                                                                     SkyBoxBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                // Render from equirectangular to cubemap
                auto cmd = [&]()
                {
                    RenderCommand::SetViewport(0, 0, cube_width, cube_width);
                    skybox_buffer_map["original"]->Bind();
                    RenderCommand::Clear();
                    equirectangle_image->BindTexture(s_Data.fragTexture_0_slot);
                };
                RenderFullScreenCube(cmd);

                // Generate skybox mipmaps
                skybox_buffer_map["original"]->GenerateMipMapLevel();
            }
        }
        /**************************************************************
        *	 3. Load env diffuse irradiance cubemaps
        **************************************************************/
        {
            for (auto& item : s_Data.gpuBuffer.EnvCubeMaps)
            {
                const std::string& name = item.first;
                const std::string& diffuse_name1 = name + "_diffuse";
                const std::string& diffuse_name2 = name + "_diffuse_SH";
                auto& skybox_buffer_map = item.second;

                bool has_diffuse1 = ResourceManager<Texture2D>::GetInstance()->Has(diffuse_name1);
                bool has_diffuse2 = ResourceManager<Texture2D>::GetInstance()->Has(diffuse_name2);
                if (!has_diffuse2)
                {
                    {
                        // SH method of generating irradiance map
                        PRINT("Building irradiance map by Spherical Harmonics!");
                        auto cmd = [name, diffuse_name2]()
                        {
                            uint32_t texture_width = 128;
                            uint32_t texture_height = texture_width / 2;
                            const auto& equirectangle_image = ResourceManager<Image2D>::GetInstance()->TryGet(name)->
                                Get();
                            Image2D::Setting setting;
                            setting.width = texture_width;
                            setting.height = texture_height;
                            setting.channels = 3;
                            setting.float_type = true;
                            auto diffuse_irradiance_equirectangle_image = MemoryManager::Make_shared<Image2D>(setting);
                            SphericalHarmonics::SHB RB(9);
                            SphericalHarmonics::SHB GB(9);
                            SphericalHarmonics::SHB BB(9);
                            {
                                SphericalHarmonics::SphericalHarmonicsOptimized(equirectangle_image, RB, GB, BB);
                                SphericalHarmonics::InverseSphericalHarmonics(
                                    diffuse_irradiance_equirectangle_image, RB, GB, BB);
                                SphericalHarmonics::Serialize(RB, GB, BB);
                            }
                            auto path = FileSystem::ResolveProtocol("$texture:" + diffuse_name2 + ".hdr");
                            diffuse_irradiance_equirectangle_image->WriteToHDR(path);
                        };
                        // Running in a background thread.
                        LongMarch_UseDeamonThreadWaitAsyncJob(ThreadPool::GetInstance()->enqueue_task(cmd));
                    }
                }
                if (!has_diffuse1)
                {
                    {
                        // Brute force sampling method of generating irradiance map
                        PRINT("Building Diffuse Irradiance map by GPU sampling!");
                        const auto& tr = Mat4(1.0f);
                        PointLightPVMatrix_GPU matrices;
                        for (int i = 0; i < 6; ++i)
                        {
                            const auto& light_view_mat = Geommath::LookAt(
                                Vec3f(0), Vec3f(0) + s_Data.cube_directions[i].xyz, s_Data.cube_ups[i]) * tr;
                            const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                                   ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, 0.1, 10)
                                                                   : Geommath::ProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, 0.1, 10);
                            matrices.PVMatrices[i] = std::move(light_projection_mat * light_view_mat);
                        }
                        auto pvMatrixBuffer = UniformBuffer::Create(matrices.GetPtr(), sizeof(PointLightPVMatrix_GPU));

                        uint32_t cube_width = 32;
                        skybox_buffer_map["irradiance"] = SkyBoxBuffer::Create(
                            cube_width, cube_width, SkyBoxBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                        // If the diffuse irradiance equirectangular map is not present,
                        // we create it first and then write it to a equirectangular map
                        {
                            auto cmd = [&]()
                            {
                                s_Data.CurrentShader = s_Data.ShaderMap["Cubemap_Irradiance"];
                                s_Data.CurrentShader->Bind();
                                pvMatrixBuffer->Bind(0);

                                RenderCommand::SetViewport(0, 0, cube_width, cube_width);
                                skybox_buffer_map["irradiance"]->Bind();
                                RenderCommand::Clear();
                                skybox_buffer_map["original"]->BindTexture(s_Data.fragTexture_0_slot);
                            };
                            RenderFullScreenCube(cmd);
                        }

                        // Render diffuse irradiance cubemap to a equirectangular map
                        {
                            uint32_t texture_width = glm::roundEven(cube_width * 6.0f / sqrtf(3.0f));
                            texture_width = NEAREST_POW2(texture_width);
                            // use nearest power of 2, even though the total pixels after convertion is not equal
                            uint32_t texture_height = texture_width / 2;
                            Texture::Setting setting;
                            setting.width = texture_width;
                            setting.height = texture_height;
                            setting.channels = 3;
                            setting.has_mipmap = false;
                            setting.linear_filter = false;
                            setting.float_type = true;
                            auto hdr_irradiance_texture = Texture2D::Create(setting);
                            auto temp_fbo = FrameBuffer::Create(texture_width, texture_height,
                                                                FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                            auto cmd = [&]()
                            {
                                auto coordTr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI * 0.5) * Geommath::FlipZ;
                                s_Data.CurrentShader = s_Data.ShaderMap["Cubemap_To_Equirectangular"];
                                s_Data.CurrentShader->Bind();
                                s_Data.CurrentShader->SetMat4("u_CoordTr", coordTr);
                                RenderCommand::SetViewport(0, 0, texture_width, texture_height);
                                temp_fbo->Bind();
                                hdr_irradiance_texture->AttachToFrameBuffer();
                                RenderCommand::Clear();
                                skybox_buffer_map["irradiance"]->BindTexture(s_Data.fragTexture_0_slot);
                            };
                            RenderFullScreenQuad(cmd);
                            auto path = FileSystem::ResolveProtocol("$texture:" + diffuse_name1 + ".hdr");
                            hdr_irradiance_texture->WriteToHDR(path);
                        }
                    }
                }
                // Load irradiance map
                if (has_diffuse1 || has_diffuse2)
                {
                    const auto& tr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI * 0.5) * Geommath::FlipZ;
                    PointLightPVMatrix_GPU matrices;
                    for (int i = 0; i < 6; ++i)
                    {
                        const auto& light_view_mat = Geommath::LookAt(
                            Vec3f(0), Vec3f(0) + s_Data.cube_directions[i].xyz, s_Data.cube_ups[i]) * tr;
                        const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                               ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                                   90 * DEG2RAD, 1, 0.1, 10)
                                                               : Geommath::ProjectionMatrixZeroOne(
                                                                   90 * DEG2RAD, 1, 0.1, 10);
                        matrices.PVMatrices[i] = std::move(light_projection_mat * light_view_mat);
                    }
                    auto pvMatrixBuffer = UniformBuffer::Create(matrices.GetPtr(), sizeof(PointLightPVMatrix_GPU));

                    std::string diffuse_name;
                    if (has_diffuse1)
                    {
                        diffuse_name = diffuse_name1;
                    }
                    if (has_diffuse2)
                    {
                        diffuse_name = diffuse_name2;
                    }
                    // Simply render the equirectangluar diffuse irradiance map to the cubemap
                    const auto& equirectangle_image = ResourceManager<Texture2D>::GetInstance()->TryGet(diffuse_name)->
                        Get();
                    uint32_t cube_width = glm::roundEven(sqrtf(3.0f) / 6.0f * equirectangle_image->GetWidth());
                    cube_width = NEAREST_POW2(cube_width);
                    // use nearest power of 2, even though the total pixels after convertion is not equal
                    skybox_buffer_map["irradiance"] = SkyBoxBuffer::Create(
                        cube_width, cube_width, SkyBoxBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                    // Render from equirectangular to cubemap
                    auto cmd = [&]()
                    {
                        s_Data.CurrentShader = s_Data.ShaderMap["Equirectangular_To_Cubemap"];
                        s_Data.CurrentShader->Bind();
                        pvMatrixBuffer->Bind(0);

                        RenderCommand::SetViewport(0, 0, cube_width, cube_width);
                        skybox_buffer_map["irradiance"]->Bind();
                        RenderCommand::Clear();
                        equirectangle_image->BindTexture(s_Data.fragTexture_0_slot);
                    };
                    RenderFullScreenCube(cmd);
                }
            }
        }

        /**************************************************************
        *	 4. BRDF integration look up texture
        **************************************************************/
        {
            const auto& brdf_texture_name = s_Data.EnvMapping.brdf_lut_name;
            bool has_texture = ResourceManager<Texture2D>::GetInstance()->Has(brdf_texture_name);
            if (!has_texture)
            {
                {
                    PRINT("Building BRDF Split-sum LUT by GPU sampling!");
                    // Brute force generating brdf texture
                    auto BRDF_ISM_Generator_Shader = Shader::Create("$shader:brdf_importance_sampling_geneartor.vert",
                                                                    "$shader:brdf_importance_sampling_geneartor.frag");
                    uint32_t texture_width = 256;
                    uint32_t texture_height = 256;
                    Texture::Setting setting;
                    setting.width = texture_width;
                    setting.height = texture_height;
                    setting.channels = 3;
                    setting.has_mipmap = false;
                    setting.linear_filter = false;
                    setting.float_type = true;

                    auto BRDF_ISM_texture = Texture2D::Create(setting);
                    auto temp_fbo = FrameBuffer::Create(texture_width, texture_height,
                                                        FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                    auto cmd = [&]()
                    {
                        BRDF_ISM_Generator_Shader->Bind();
                        RenderCommand::SetViewport(0, 0, texture_width, texture_height);
                        temp_fbo->Bind();
                        BRDF_ISM_texture->AttachToFrameBuffer();
                        RenderCommand::Clear();
                    };
                    RenderFullScreenQuad(cmd);
                    auto path = FileSystem::ResolveProtocol("$texture:" + brdf_texture_name + ".hdr");
                    BRDF_ISM_texture->WriteToHDR(path);

                    // Add BRDF ISM to managed resource allocator
                    ResourceManager<Texture2D>::GetInstance()->AddResource(brdf_texture_name, path, BRDF_ISM_texture);
                }
            }
            s_Data.gpuBuffer.BrdfIntegrateLUT = ResourceManager<Texture2D>::GetInstance()->TryGet(brdf_texture_name)->
                Get();
        }
        /**************************************************************
        *	 5. Load env specular radiance cubemaps
        **************************************************************/
        {
            for (auto& item : s_Data.gpuBuffer.EnvCubeMaps)
            {
                const std::string& name = item.first;
                const std::string& specular_name = name + "_specular_LOD0";
                auto& skybox_buffer_map = item.second;

                uint32_t cube_width = 512;
                skybox_buffer_map["radiance"] = SkyBoxBuffer::Create(cube_width, cube_width,
                                                                     SkyBoxBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                const auto& skybox_map_buffer = skybox_buffer_map["original"];
                auto& radiance_map_buffer = skybox_buffer_map["radiance"];
                uint32_t skybox_cube_width = skybox_map_buffer->GetBufferSize().x;
                uint32_t max_skybox_mipmap = skybox_map_buffer->GetMaxMipMapLevel();
                uint32_t max_radiance_mipmap = radiance_map_buffer->GetMaxMipMapLevel();

                bool has_specular = ResourceManager<Texture2D>::GetInstance()->Has(specular_name);
                if (!has_specular)
                {
                    {
                        // Brute force sampling method of generating specular map
                        PRINT("Building Specular Rradiance map by GPU sampling!");
                        const auto& tr = Mat4(1.0f);
                        PointLightPVMatrix_GPU matrices;
                        for (int i = 0; i < 6; ++i)
                        {
                            const auto& light_view_mat = Geommath::LookAt(
                                Vec3f(0), Vec3f(0) + s_Data.cube_directions[i].xyz, s_Data.cube_ups[i]) * tr;
                            const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                                   ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, 0.1, 10)
                                                                   : Geommath::ProjectionMatrixZeroOne(
                                                                       90 * DEG2RAD, 1, 0.1, 10);
                            matrices.PVMatrices[i] = std::move(light_projection_mat * light_view_mat);
                        }
                        auto pvMatrixBuffer = UniformBuffer::Create(matrices.GetPtr(), sizeof(PointLightPVMatrix_GPU));

                        for (uint32_t i(0); i <= max_radiance_mipmap; ++i)
                        {
                            uint32_t mipmap_cube_width = (uint32_t)glm::floor(cube_width * powf(0.5, i));

                            float roughness = glm::clamp((float)i / (float)(max_radiance_mipmap), 0.001f, 0.999f);
                            // Render from equirectangular to cubemap
                            auto cmd = [&]()
                            {
                                s_Data.CurrentShader = s_Data.ShaderMap["Cubemap_Radiance"];
                                s_Data.CurrentShader->Bind();
                                s_Data.CurrentShader->SetFloat("u_Resolution", float(skybox_cube_width));
                                s_Data.CurrentShader->SetFloat("u_MaxMipMap", float(max_skybox_mipmap));
                                s_Data.CurrentShader->SetFloat("u_Roughness", roughness);
                                pvMatrixBuffer->Bind(0);

                                RenderCommand::SetViewport(0, 0, mipmap_cube_width, mipmap_cube_width);
                                radiance_map_buffer->Bind();
                                radiance_map_buffer->BindMipMapLevel(i);
                                RenderCommand::Clear();
                                skybox_map_buffer->BindTexture(s_Data.fragTexture_0_slot);
                            };
                            RenderFullScreenCube(cmd);
                        }

                        // Render specular radiance cubemap to a equirectangular map
                        {
                            uint32_t max_mipmap = radiance_map_buffer->GetMaxMipMapLevel();
                            uint32_t cube_width = radiance_map_buffer->GetBufferSize().x;

                            for (uint32_t i(0); i <= max_mipmap; ++i)
                            {
                                const auto& specular_name1 = name + "_specular_LOD" + Str(i);
                                uint32_t texture_width = glm::roundEven((cube_width >> i) * 6.0f / sqrtf(3.0f));
                                texture_width = NEXT_POW2(texture_width);
                                // use nearest power of 2, even though the total pixels after convertion is not equal
                                uint32_t texture_height = texture_width / 2;
                                Texture::Setting setting;
                                setting.width = texture_width;
                                setting.height = texture_height;
                                setting.channels = 3;
                                setting.has_mipmap = false;
                                setting.linear_filter = false;
                                setting.float_type = true;
                                auto hdr_radiance_texture = Texture2D::Create(setting);
                                auto temp_fbo = FrameBuffer::Create(texture_width, texture_height,
                                                                    FrameBuffer::BUFFER_FORMAT::FLOAT_RGB16);

                                auto cmd = [&]()
                                {
                                    auto coordTr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI * 0.5) *
                                        Geommath::FlipZ;
                                    s_Data.CurrentShader = s_Data.ShaderMap["Cubemap_To_Equirectangular"];
                                    s_Data.CurrentShader->Bind();
                                    s_Data.CurrentShader->SetInt("u_LOD", i);
                                    s_Data.CurrentShader->SetMat4("u_CoordTr", coordTr);
                                    RenderCommand::SetViewport(0, 0, texture_width, texture_height);
                                    temp_fbo->Bind();
                                    hdr_radiance_texture->AttachToFrameBuffer();
                                    RenderCommand::Clear();
                                    radiance_map_buffer->BindTexture(s_Data.fragTexture_0_slot);
                                };
                                RenderFullScreenQuad(cmd);
                                auto path = FileSystem::ResolveProtocol("$texture:" + specular_name1 + ".hdr");
                                hdr_radiance_texture->WriteToHDR(path);
                            }
                        }
                    }
                }
                else
                {
                    const auto& tr = Geommath::RotationMat(Geommath::ROT_AXIS::Z, PI * 0.5) * Geommath::FlipZ;
                    PointLightPVMatrix_GPU matrices;
                    for (int i = 0; i < 6; ++i)
                    {
                        const auto& light_view_mat = Geommath::LookAt(
                            Vec3f(0), Vec3f(0) + s_Data.cube_directions[i].xyz, s_Data.cube_ups[i]) * tr;
                        const auto& light_projection_mat = (s_Data.enable_reverse_z)
                                                               ? Geommath::ReverseZProjectionMatrixZeroOne(
                                                                   90 * DEG2RAD, 1, 0.1, 10)
                                                               : Geommath::ProjectionMatrixZeroOne(
                                                                   90 * DEG2RAD, 1, 0.1, 10);
                        matrices.PVMatrices[i] = std::move(light_projection_mat * light_view_mat);
                    }
                    auto pvMatrixBuffer = UniformBuffer::Create(matrices.GetPtr(), sizeof(PointLightPVMatrix_GPU));
                    auto cube_width = radiance_map_buffer->GetBufferSize().x;

                    for (uint32_t i(0); i <= max_radiance_mipmap; ++i)
                    {
                        const auto& specular_name1 = name + "_specular_LOD" + Str(i);
                        // Simply render the equirectangluar diffuse irradiance map to the cubemap
                        const auto& equirectangle_image = ResourceManager<Texture2D>::GetInstance()->TryGet(
                            specular_name1)->Get();
                        uint32_t mipmap_cube_width = (uint32_t)glm::floor(cube_width * powf(0.5, i));

                        // Render from equirectangular to cubemap
                        auto cmd = [&]()
                        {
                            s_Data.CurrentShader = s_Data.ShaderMap["Equirectangular_To_Cubemap"];
                            s_Data.CurrentShader->Bind();
                            pvMatrixBuffer->Bind(0);

                            RenderCommand::SetViewport(0, 0, mipmap_cube_width, mipmap_cube_width);
                            radiance_map_buffer->Bind();
                            radiance_map_buffer->BindMipMapLevel(i);
                            RenderCommand::Clear();
                            equirectangle_image->BindTexture(s_Data.fragTexture_0_slot);
                        };
                        RenderFullScreenCube(cmd);
                    }
                }
            }
        }
    }
}

void longmarch::Renderer3D::UpdateMeshToMultiDraw(const LongMarch_Vector<std::shared_ptr<MeshData>>& Meshs)
{
    // Prevent uploading the same mesh data sequences
    static LongMarch_Vector<std::shared_ptr<MeshData>> _Mesh_duplicate_guard;
    if (_Mesh_duplicate_guard == Meshs)
    {
        return;
    }
    else
    {
        _Mesh_duplicate_guard = Meshs;
    }

    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
    size_t total_vbo_size = 0, total_ibo_size = 0;
    for (const auto& item : Meshs)
    {
        // Find total sizes
        size_t _vbo_size = item->GetVertexDataSize();
        size_t _ibo_size = item->GetIndexDataSize();

        total_vbo_size += _vbo_size;
        total_ibo_size += _ibo_size;
    }

    // Resize vbo, ibo if necessary
    _multiDrawBuffer.MultiDraw_VBO->UpdateBufferData(nullptr, total_vbo_size);
    _multiDrawBuffer.MultiDraw_IBO->UpdateBufferData(nullptr, total_ibo_size);
    // Clear cmd buffer map before appending all mesh datas
    _multiDrawBuffer.MeshData_CmdBuffer_Map.clear();

    for (const auto& item : Meshs)
    {
        AppendMeshToMultiDraw(item);
    }
}

void longmarch::Renderer3D::AppendMeshToMultiDraw(std::shared_ptr<MeshData> Mesh)
{
    auto& _multiDrawBuffer = s_Data.multiDrawBuffer;
    auto& cmd_map = _multiDrawBuffer.MeshData_CmdBuffer_Map;
    if (LongMarch_contains(cmd_map, Mesh))
    {
        throw EngineException(
            _CRT_WIDE(__FILE__), __LINE__,
            L"Appending an already stored MeshData to the universal buffer! This should be avoided!");
    }

    // Copy mesh vbo, ibo to multi draw vbo, ibo
    auto& vao = _multiDrawBuffer.MultiDraw_VAO;
    auto& vbo = _multiDrawBuffer.MultiDraw_VBO;
    auto& ibo = _multiDrawBuffer.MultiDraw_IBO;;
    {
        size_t _vbo_size = Mesh->GetVertexDataSize();
        size_t _ibo_size = Mesh->GetIndexDataSize();

        {
            size_t vbo_size = (vbo->GetCount() * vbo->GetElementSize());
            ASSERT(vbo_size % MeshData::GetVertexStrucSize() == 0,
                   "Mesh VBO must be integer multiples of vertex layout");

            auto& cmd = cmd_map[Mesh];
            cmd.indexCount = _ibo_size / ibo->GetElementSize();
            cmd.firstIndex = ibo->GetCount();
            cmd.baseVertex = vbo_size / MeshData::GetVertexStrucSize();
        }

        vbo->AppendBufferData(Mesh->GetVertexDataPtr(), _vbo_size);
        ibo->AppendBufferData(Mesh->GetIndexDataPtr(), _ibo_size);
    }
}

void longmarch::Renderer3D::ToggleReverseZ(bool enable)
{
    LOCK_GUARD_S();
    s_Data.enable_reverse_z = enable;
    RenderCommand::Reverse_Z(enable);
}
