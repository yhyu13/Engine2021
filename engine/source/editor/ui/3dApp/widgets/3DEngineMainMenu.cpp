#include "engine-precompiled-header.h"
#include "3DEngineMainMenu.h"
#include "../3DEngineWidgetManager.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include <imgui/addons/implot/implot_items.cpp>
#include <imgui/addons/implot/implot_demo.cpp>

void longmarch::_3DEngineMainMenu::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize);
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
			bool b = manager->GetVisible("ProfilerPage");
			manager->SetVisible("ProfilerPage", !b);
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

	manager->CaptureMouseAndKeyboardOnMenu();
	manager->PopWidgetStyle();
	ImGui::End();
}

void longmarch::_3DEngineMainMenu::RenderEngineSettingMenu()
{
	if (ImGui::TreeNode("Engine Settings"))
	{
		constexpr int yoffset_item = 2;

		auto settingEventQueue = EventQueue<EngineSettingEventType>::GetInstance();

		static bool checkInterrutionHandle = true;

		if (ImGui::Button("Reset engine settings to default values"))
		{
			ImGuiUtil::bStyleDark = true;
			ImGuiUtil::alpha = 1.0f;
			checkInterrutionHandle = true;
			{
				auto e = MemoryManager::Make_shared<ToggleWindowInterrutpionEvent>(checkInterrutionHandle);
				settingEventQueue->Publish(e);
			}
		}
		ImGui::Dummy(ImVec2(0, yoffset_item));

		{
			ImGui::Checkbox("Dark Theme", &ImGuiUtil::bStyleDark);
			ImGui::SliderFloat("Alpha", &ImGuiUtil::alpha, 0.01f, 1.0f, "%.2f");
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
		static auto AOConfig = graphicsConfig["AO"];
		static auto SSRConfig = graphicsConfig["SSR"];
		static auto BloomConfig = graphicsConfig["Bloom"];

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
		// Env mapping
		static bool checkEvnMapping = graphicsConfig["Env-mapping"].asBool();
		// Toggle shadow
		static bool checkShadow = graphicsConfig["Shadow"].asBool();
		// Deferred shading
		static bool checkDeferredShading = graphicsConfig["Deferred-shading"].asBool();
		// Clustered shading
		static bool checkClusteredShading = graphicsConfig["Clustered-shading"].asBool();
		// Motion Blur
		static bool checkMotionBlur = graphicsConfig["Motion-blur"].asBool();
		static int valueMotionblurShutterSpeed = graphicsConfig["Motion-blur-shutter-speed"].asInt();
		// SMAA
		static bool checkSMAA = graphicsConfig["SMAA"].asBool();
		// FXAA
		static bool checkFXAA = graphicsConfig["FXAA"].asBool();
		// TAA
		static bool checkTAA = graphicsConfig["TAA"].asBool();
		// Tone mapping
		static const char* toneMapModes[]{ "AcesFilm", "Uncharted2", "Filmic","Reinhard" };
		static int selected_toneMap = 0;
		// Gamma
		static float valueGamma = 2.2f;

		// AO
		static bool checkAO = AOConfig["Enable"].asBool();
		static int valueAOSample = AOConfig["Num-samples"].asInt();
		static int valueAOSampleResDownScale = AOConfig["Res-down-scale"].asInt();
		static int valueAOBlurKernel = AOConfig["Gaussian-kernel"].asInt();
		static float valueAOSampleRadius = AOConfig["Radius"].asFloat();
		static float valueAOScale = AOConfig["Scale"].asFloat();
		static float valueAOPower = AOConfig["Power"].asFloat();
		// Indirect Bounce of light
		static bool checkIndBonLit = AOConfig["Indirect-bounce"].asBool();
		static float valueIndBonLitScale = AOConfig["Indirect-bounce-strength"].asFloat();

		// SSR
		static bool checkSSR = SSRConfig["Enable"].asBool();
		static int valueSSRBlurKernel = SSRConfig["Gaussian-kernel"].asInt();
		static int valueSSRSampleResDownScale = SSRConfig["Res-down-scale"].asInt();

		// Bloom
		static bool checkBloom = BloomConfig["Enable"].asBool();
		static float valueBloomThreshold = BloomConfig["Threshold"].asFloat();
		static float valueBloomStrength = BloomConfig["Strength"].asFloat();
		static int valueBloomBlurKernel = BloomConfig["Gaussian-kernel"].asInt();
		static int valueBloomSampleResDownScale = BloomConfig["Res-down-scale"].asFloat();

		constexpr int yoffset_item = 5;

		if (ImGui::Button("Reset graphics settings to default values"))
		{
			checkVSync = windowConfig["V-sync"].asBool();
			checkGPUSync = windowConfig["GPU-sync"].asBool();
			selected_gbufferModes = 0;
			checkEvnMapping = graphicsConfig["Env-mapping"].asBool();
			checkShadow = graphicsConfig["Shadow"].asBool();
			checkDeferredShading = graphicsConfig["Deferred-shading"].asBool();
			checkClusteredShading = graphicsConfig["Clustered-shading"].asBool();
			checkFXAA = graphicsConfig["FXAA"].asBool();
			checkSMAA = graphicsConfig["SMAA"].asBool();
			checkMotionBlur = graphicsConfig["Motion-blur"].asBool();
			valueMotionblurShutterSpeed = graphicsConfig["Motion-blur-shutter-speed"].asInt();
			checkTAA = graphicsConfig["TAA"].asBool();
			{
				selected_toneMap = 0;
				valueGamma = 2.2f;
			}
			{
				// AO
				checkAO = AOConfig["Enable"].asBool();
				valueAOSample = AOConfig["Num-samples"].asInt();
				valueAOSampleResDownScale = AOConfig["Res-down-scale"].asInt();
				valueAOBlurKernel = AOConfig["Gaussian-kernel"].asInt();
				valueAOSampleRadius = AOConfig["Radius"].asFloat();
				valueAOScale = AOConfig["Scale"].asFloat();
				valueAOPower = AOConfig["Power"].asFloat();
				// Indirect Bounce of light
				checkIndBonLit = AOConfig["Indirect-bounce"].asBool();
				valueIndBonLitScale = AOConfig["Indirect-bounce-strength"].asFloat();
			}
			{
				// SSR
				checkSSR = SSRConfig["Enable"].asBool();
				valueSSRBlurKernel = SSRConfig["Gaussian-kernel"].asFloat();
				valueSSRSampleResDownScale = SSRConfig["Res-down-scale"].asFloat();
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
				auto e = MemoryManager::Make_shared<ToggleEnvironmentMappingEvent>(checkEvnMapping);
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
				auto e = MemoryManager::Make_shared<ToggleSMAAEvent>(checkSMAA);
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
				auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale);
				graphicEventQueue->Publish(e);
			}
			{
				auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
				graphicEventQueue->Publish(e);
			}
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
				// Toggle shadow
				{
					if (ImGui::Checkbox("Environment Mapping", &checkEvnMapping))
					{
						auto e = MemoryManager::Make_shared<ToggleEnvironmentMappingEvent>(checkEvnMapping);
						graphicDebugEventQueue->Publish(e);
					}
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
				{
					if (ImGui::Checkbox("Moltion Blur", &checkMotionBlur))
					{
						auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Moltion Blur Shutter Speed", &valueMotionblurShutterSpeed, 15, 90))
					{
						auto e = MemoryManager::Make_shared<ToggleMotionBlurEvent>(checkMotionBlur, valueMotionblurShutterSpeed);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// AO
				if (ImGui::TreeNode("SSAO"))
				{
					if (ImGui::Checkbox("Enable", &checkAO))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueAOSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Samples", &valueAOSample, 5, 80, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueAOBlurKernel, 3, 30, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Sample Radius", &valueAOSampleRadius, 0.01f, 20.0f, "%.2f"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Scale", &valueAOScale, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Power", &valueAOPower, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::Checkbox("Indirect Bounce", &checkIndBonLit))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderFloat("Indirect Bounce Strength", &valueIndBonLitScale, 0.1f, 10.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetAOValueEvent>(checkAO, valueAOSample, valueAOSampleResDownScale, valueAOBlurKernel, valueAOSampleRadius, valueAOScale, valueAOPower, checkIndBonLit, valueIndBonLitScale);
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
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Resolution Down Scale", &valueSSRSampleResDownScale, 1, 4, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
					if (ImGui::SliderInt("Gauss Kernel", &valueSSRBlurKernel, 3, 30, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetSSRValueEvent>(checkSSR, valueSSRBlurKernel, valueSSRSampleResDownScale);
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
					if (ImGui::SliderInt("Gauss Kernel", &valueBloomBlurKernel, 3, 30, "%d"))
					{
						auto e = MemoryManager::Make_shared<SetBloomEvent>(checkBloom, valueBloomThreshold, valueBloomStrength, valueBloomBlurKernel, valueBloomSampleResDownScale);
						graphicEventQueue->Publish(e);
					}
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
				// TAA
				{
					if (ImGui::Checkbox("TAA", &checkTAA))
					{
						auto e = MemoryManager::Make_shared<ToggleTAAEvent>(checkTAA);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// FXAA
				{
					if (ImGui::Checkbox("SMAA", &checkSMAA))
					{
						auto e = MemoryManager::Make_shared<ToggleSMAAEvent>(checkSMAA);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// FXAA
				{
					if (ImGui::Checkbox("FXAA", &checkFXAA))
					{
						auto e = MemoryManager::Make_shared<ToggleFXAAEvent>(checkFXAA);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// Tone mapping
				{
					if (ImGui::Combo("Tone Mapping", &selected_toneMap, toneMapModes, IM_ARRAYSIZE(toneMapModes)))
					{
						auto e = MemoryManager::Make_shared<SwitchToneMappingEvent>(selected_toneMap);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::Dummy(ImVec2(0, yoffset_item));
				// Gamma slider
				{
					if (ImGui::SliderFloat("Gamma", &valueGamma, 0.1f, 5.0f, "%.1f"))
					{
						auto e = MemoryManager::Make_shared<SetGammaValueEvent>(valueGamma);
						graphicEventQueue->Publish(e);
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}