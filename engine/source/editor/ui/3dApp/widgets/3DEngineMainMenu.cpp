#include "engine-precompiled-header.h"
#include "3DEngineMainMenu.h"
#include "../3DEngineWidgetManager.h"
#include "engine/renderer/Renderer3D.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include "editor/ui/common/widgets/EngineEditorHUD.h"

#include <imgui/addons/implot/implot_items.cpp>
#include <imgui/addons/implot/implot_demo.cpp>

void longmarch::_3DEngineMainMenu::Render()
{
	WIDGET_TOGGLE(KEY_F11);
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);
	if (!ImGui::Begin("Engine Main Menu", &m_IsVisible))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::End();
		return;
	}

	constexpr int yoffset_item = 5;
	{
		static bool bDemoPage = false;
		if (ImGui::Button("ImGui Demo Page"))
		{
			bDemoPage = !bDemoPage;
		}
		if (bDemoPage)
		{
			ImGui::ShowDemoWindow();
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		static bool bplotDemoPage = false;
		if (ImGui::Button("ImPlot Demo Page"))
		{
			bplotDemoPage = !bplotDemoPage;
		}
		if (bplotDemoPage)
		{
			ImPlot::ShowDemoWindow();
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));
	{
		if (ImGui::Button("Engine Profiler page"))
		{
			bool visible = manager->GetVisible("ProfilerPage");
			visible = !visible;
			manager->SetVisible("ProfilerPage", visible);
		}
	}
	ImGui::Dummy(ImVec2(0, yoffset_item));

	{
		// Game setting
		RenderEngineSettingMenu();
	}

	ImGui::Dummy(ImVec2(0, yoffset_item));

	{
		// Graphics setting
		RenderEngineGraphicSettingMenu();
	}

	manager->CaptureMouseAndKeyboardOnHover(true);
	manager->PopWidgetStyle();
	ImGui::End();
}

void longmarch::_3DEngineMainMenu::RenderEngineSettingMenu()
{
	if (ImGui::TreeNode("Engine Settings"))
	{
		constexpr int yoffset_item = 5;

		auto settingEventQueue = EventQueue<EngineSettingEventType>::GetInstance();

		static auto engineConfiguration = FileSystem::GetNewJsonCPP("$root:engine-config.json");
		static bool checkInterrutionHandle = engineConfiguration["engine"]["Pause-on-unfocused"].asBool();

		if (ImGui::Button("Reset engine settings to default values"))
		{
			auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
			auto editor_hud = static_cast<EngineEditorHUD*>(manager->GetWidget("0_HUD"));
			editor_hud->bStyleDark = true;
			editor_hud->fStyleAlpha = 1.0f;

			checkInterrutionHandle = engineConfiguration["engine"]["Pause-on-unfocused"].asBool();
			{
				auto e = MemoryManager::Make_shared<ToggleWindowInterrutpionEvent>(checkInterrutionHandle);
				settingEventQueue->Publish(e);
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));
		{
			auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
			auto editor_hud = static_cast<EngineEditorHUD*>(manager->GetWidget("0_HUD"));

			auto style = editor_hud->bStyleDark;
			if (ImGui::Checkbox("Dark Theme", &style))
			{
				editor_hud->bStyleDark = style;
			}
			auto alpha = editor_hud->fStyleAlpha;
			if (ImGui::SliderFloat("Alpha", &alpha, 0.01f, 1.0f, "%.2f"))
			{
				editor_hud->fStyleAlpha = alpha;
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));

		if (ImGui::Checkbox("Enable pause on window unfocused", &checkInterrutionHandle))
		{
			{
				auto e = MemoryManager::Make_shared<ToggleWindowInterrutpionEvent>(checkInterrutionHandle);
				settingEventQueue->Publish(e);
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));

		if (ImGui::BeginTabBar("Engine Settings"))
		{
			ImGui::EndTabBar();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}

void longmarch::_3DEngineMainMenu::RenderEngineGraphicSettingMenu()
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
		static const char* windowModes[]{ "Fullscreen", "Borderless Windowed", "Windowed" };
		static int selected_windowModes = windowConfig["Full-screen"].asInt();
		// 2. Window resolution
		static const char* windowReso[]{ "1920x1080", "1600x900", "1366x768","1280x720", // 16:9
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
		static const char* gbufferModes[]{ "Default", "Depth", "Normal","Albedo","Emission","Specular","Roughness","Metallic" ,"Ambient occulusion", "SS Velocity" };
		static int selected_gbufferModes = 0;

		// Debug Cluster Light Mode
		static bool enable_debug_cluster_light_mode = windowConfig["Debug_cluster_light"].asBool();
		// Toggle shadow
		static bool checkShadow = graphicsConfig["Shadow"].asBool();
		// Deferred shading
		static bool checkDeferredShading = graphicsConfig["Deferred-shading"].asBool();
		// Clustered shading
		static bool checkClusteredShading = graphicsConfig["Clustered-shading"].asBool();

		// FXAA
		static bool checkFXAA = graphicsConfig["FXAA"].asBool();
		// TAA
		static bool checkTAA = graphicsConfig["TAA"].asBool();
		// Tone mapping
		static const char* toneMapModes[]{ "AcesFilm", "Luminance", "Uncharted2", "Filmic","Reinhard" };
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
			checkClusteredShading = graphicsConfig["Clustered-shading"].asBool();
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
				auto e = MemoryManager::Make_shared<SetEnvironmentMappingEvent>(checkEvnMapping, valueCurrentEnvMapName);
				graphicDebugEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
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
				auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
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
			FileSystem::RemoveCachedJsonCPP(config_path); //< Remove cached json file so that we can load the one that has just written to
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
					if (ImGui::Combo("Resolution", &selected_windowReso, windowReso, IM_ARRAYSIZE(windowReso)))
					{
						// TODO : Event
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
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// GPU-sync
				{
					if (ImGui::Checkbox("GPU Sync", &checkGPUSync))
					{
						auto e = MemoryManager::Make_shared<ToggleGPUSyncEvent>(checkGPUSync);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// Debug Cluster Light
				{
					if (ImGui::Checkbox("Cluster Light Debug", &enable_debug_cluster_light_mode))
					{
						auto e = MemoryManager::Make_shared<ToggleSlicesEvent>(enable_debug_cluster_light_mode);
						graphicDebugEventQueue->Publish(e);
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
						auto e = MemoryManager::Make_shared<SetEnvironmentMappingEvent>(checkEvnMapping, valueCurrentEnvMapName);
						graphicDebugEventQueue->Publish(e);
					}
					int selected_skybox = LongMarch_findFristIndex(valueAllEnvMapName, valueCurrentEnvMapName) + 1;
					auto valueAllEnvMapName_char = LongMarch_StrVec2ConstChar(valueAllEnvMapName);
					valueAllEnvMapName_char.insert(valueAllEnvMapName_char.begin(),"");
					if (ImGui::Combo("Skyboxes", &selected_skybox, &valueAllEnvMapName_char[0], valueAllEnvMapName_char.size()))
					{
						valueCurrentEnvMapName = valueAllEnvMapName[selected_skybox];
						auto e = MemoryManager::Make_shared<SetEnvironmentMappingEvent>(checkEvnMapping, valueCurrentEnvMapName);
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
						auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Moltion Blur Shutter Speed", &valueMotionblurShutterSpeed, 15, 90))
					{
						auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
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
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueSSGISampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Samples", &valueSSGISample, 5, 80, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueSSGIBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
						graphicEventQueue->Publish(e);
					}
					ImGuiUtil::InlineHelpMarker("Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
					if (ImGui::SliderFloat("Sample Radius", &valueSSGISampleRadius, 0.01f, 20.0f, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Strength", &valueSSGIStrength, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetSSGIValueEvent>(checkSSGI, valueSSGISample, valueSSGISampleResDownScale, valueSSGIBlurKernel, valueSSGISampleRadius, valueSSGIStrength);
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
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueSSAOSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Samples", &valueSSAOSample, 5, 80, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueSSAOBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					ImGuiUtil::InlineHelpMarker("Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
					if (ImGui::SliderFloat("Sample Radius", &valueSSAOSampleRadius, 0.01f, 20.0f, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Scale", &valueSSAOScale, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Power", &valueSSAOPower, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetSSAOValueEvent>(checkSSAO, valueSSAOSample, valueSSAOSampleResDownScale, valueSSAOBlurKernel, valueSSAOSampleRadius, valueSSAOScale, valueSSAOPower);
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
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueSSRSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueSSRBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
						graphicEventQueue->Publish(e);
					}
					ImGuiUtil::InlineHelpMarker("Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
					if (ImGui::Checkbox("Debug", &checkSSRDebug))
					{
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale, checkSSRDebug);
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
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueBloomSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueBloomBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
					ImGuiUtil::InlineHelpMarker("Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
					if (ImGui::DragFloat("Threshold", &valueBloomThreshold, 0.05, -60, 1, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
						graphicEventQueue->Publish(e);
					} ImGuiUtil::InlineHelpMarker("Negative value serves as the power of a exponent s-curve in [-40,0], the greater this value, the greater the bloom coverage. Positive value serves as the cutoff value of a step function in [0,1], the the lower this value, the greater the bloom coverage ");
					if (ImGui::DragFloat("Strength", &valueBloomStrength, 0.01, 0, 1, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
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
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueDOFSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueDOFBlurKernel, LongMarch_GUASSIAN_KERNEL_MIN, LongMarch_GUASSIAN_KERNEL_MAX, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					ImGuiUtil::InlineHelpMarker("Kernel size is scaled with buffer size against 1080p (e.g. 540p buffer size will have half the size of the kernel)");
					if (ImGui::DragFloat("Threshold", &valueDOFThreshold, 0.1, 0.01, 100, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::DragFloat("Blend Strength", &valueDOFStrength, 0.01, 0.01, 1, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::DragFloat("Refocus Speed", &valueDOFRefocusRate, 0.1, 1, 60, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::Checkbox("Debug", &checkDOFDebug))
					{
						auto e = MemoryManager::Make_shared<SetDOFvent>(checkDOF, valueDOFThreshold, valueDOFStrength, valueDOFBlurKernel, valueDOFSampleResDownScale, valueDOFRefocusRate, checkDOFDebug);
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