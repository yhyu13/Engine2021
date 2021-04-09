#pragma once

#include "EngineEventType.h"
#include "engine/events/EventQueue.h"
#include "engine/ecs/EntityDecorator.h"
#include "engine/math/Geommath.h"

namespace longmarch
{
	struct EngineCollisionEvent : public Event<EngineEventType> {
		explicit EngineCollisionEvent(const EntityDecorator& entity1, const EntityDecorator& entity2, void* collisionData)
			:
			Event(EngineEventType::COLLISION),
			m_entity1(entity1),
			m_entity2(entity2),
			m_collisionData(collisionData)
		{
		}
		EntityDecorator m_entity1;
		EntityDecorator m_entity2;
		void* m_collisionData;
	};

	struct EngineGCEvent : public Event<EngineEventType> {
		explicit EngineGCEvent(const EntityDecorator& entity)
			:
			Event(EngineEventType::GC),
			m_entity(entity)
		{
		}
		EntityDecorator m_entity;
	};

	struct EngineGCRecursiveEvent : public Event<EngineEventType> {
		explicit EngineGCRecursiveEvent(const EntityDecorator& entity)
			:
			Event(EngineEventType::GC_RECURSIVE),
			m_entity(entity)
		{
		}
		EntityDecorator m_entity;
	};

	struct EngineWindowInterruptionEvent : public Event<EngineEventType> {
		explicit EngineWindowInterruptionEvent(bool isFocused)
			:
			Event(EngineEventType::ENG_WINDOW_INTERRUTPTION),
			m_isFocused(isFocused)
		{
		}
		bool m_isFocused;
	};

	struct EngineWindowQuitEvent : public Event<EngineEventType> {
		explicit EngineWindowQuitEvent()
			:
			Event(EngineEventType::ENG_WINDOW_QUIT)
		{
		}
	};

	struct EngineCursorSwitchModeEvent : public Event<EngineEventType> {
		explicit EngineCursorSwitchModeEvent(int mode)
			:
			Event(EngineEventType::ENG_WINDOW_CURSOR_SWITCH_MODE),
			m_mode(mode)
		{
		}
		int m_mode;
	};

	struct EngineSaveSceneBeginEvent : public Event<EngineIOEventType> {
		explicit EngineSaveSceneBeginEvent(const std::string& filepath, void* world)
			:
			Event(EngineIOEventType::SAVE_SCENE_BEGIN),
			m_filepath(filepath),
			m_gameworld(world)
		{
		}
		std::string m_filepath;
		void* m_gameworld;
	};

	struct EngineSaveSceneEvent : public Event<EngineIOEventType> {
		explicit EngineSaveSceneEvent(const std::string& filepath, void* world)
			:
			Event(EngineIOEventType::SAVE_SCENE),
			m_filepath(filepath),
			m_gameworld(world)
		{
		}
		std::string m_filepath;
		void* m_gameworld;
	};

	struct EngineSaveSceneEndEvent : public Event<EngineIOEventType> {
		explicit EngineSaveSceneEndEvent(const std::string& filepath, void* world)
			:
			Event(EngineIOEventType::SAVE_SCENE_END),
			m_filepath(filepath),
			m_gameworld(world)
		{
		}
		std::string m_filepath;
		void* m_gameworld;
	};

	struct EngineLoadSceneBeginEvent : public Event<EngineIOEventType> {
		explicit EngineLoadSceneBeginEvent(const std::string& filepath, bool makeCurrent)
			:
			Event(EngineIOEventType::LOAD_SCENE_BEGIN),
			m_filepath(filepath),
			m_makeCurrent(makeCurrent)
		{
		}
		std::string m_filepath;
		bool m_makeCurrent;
	};

	struct EngineLoadSceneEvent : public Event<EngineIOEventType> {
		explicit EngineLoadSceneEvent(const std::string& filepath, bool makeCurrent)
			:
			Event(EngineIOEventType::LOAD_SCENE),
			m_filepath(filepath),
			m_makeCurrent(makeCurrent)
		{
		}
		std::string m_filepath;
		bool m_makeCurrent;
	};

	struct EngineLoadSceneEndEvent : public Event<EngineIOEventType> {
		explicit EngineLoadSceneEndEvent(const std::string& filepath, bool makeCurrent)
			:
			Event(EngineIOEventType::LOAD_SCENE_END),
			m_filepath(filepath),
			m_makeCurrent(makeCurrent)
		{
		}
		std::string m_filepath;
		bool m_makeCurrent;
	};

	struct ToggleWindowInterrutpionEvent : public Event<EngineSettingEventType> {
		ToggleWindowInterrutpionEvent(bool b)
			:
			Event(EngineSettingEventType::TOGGLE_WINDOW_INTERRUTPTION_HANDLE),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct SwitchGBufferEvent : public Event<EngineGraphicsDebugEventType> {
		explicit SwitchGBufferEvent(int v)
			:
			Event(EngineGraphicsDebugEventType::SWITCH_G_BUFFER_DISPLAY),
			m_value(v)
		{
		}
		int m_value;
	};

	struct ToggleSlicesEvent : public Event<EngineGraphicsDebugEventType> {
		explicit ToggleSlicesEvent(bool b)
			:
			Event(EngineGraphicsDebugEventType::TOGGLE_DEBUG_CLUSTER),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	
	struct SetEnvironmentMappingEvent : public Event<EngineGraphicsDebugEventType> {
		explicit SetEnvironmentMappingEvent(bool b, const std::string& name)
			:
			Event(EngineGraphicsDebugEventType::SET_ENV_MAPPING),
			m_enable(b),
			m_currentEnvMap(name)
		{
		}
		bool m_enable;
		std::string m_currentEnvMap;
	};

	struct ToggleShadowEvent : public Event<EngineGraphicsDebugEventType> {
		explicit ToggleShadowEvent(bool b)
			:
			Event(EngineGraphicsDebugEventType::TOGGLE_SHADOW),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct ToggleVSyncEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleVSyncEvent(bool b)
			:
			Event(EngineGraphicsEventType::TOGGLE_VSYNC),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct ToggleGPUSyncEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleGPUSyncEvent(bool b)
			:
			Event(EngineGraphicsEventType::TOGGLE_GPUSYNC),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct ToggleMotionBlurEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleMotionBlurEvent(bool b, int shutterSpeed)
			:
			Event(EngineGraphicsEventType::TOGGLE_MOTION_BLUR),
			m_enable(b),
			m_shutterSpeed(shutterSpeed)
		{
		}
		bool m_enable;
		int m_shutterSpeed;
	};

	struct ToggleTAAEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleTAAEvent(bool b)
			:
			Event(EngineGraphicsEventType::TOGGLE_TAA),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct ToggleSMAAEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleSMAAEvent(bool b, int mode)
			:
			Event(EngineGraphicsEventType::TOGGLE_SMAA),
			m_enable(b),
			m_mode(mode)
		{
		}
		bool m_enable;
		int m_mode;
	};

	struct ToggleFXAAEvent : public Event<EngineGraphicsEventType> {
		explicit ToggleFXAAEvent(bool b)
			:
			Event(EngineGraphicsEventType::TOGGLE_FXAA),
			m_enable(b)
		{
		}
		bool m_enable;
	};

	struct SwitchToneMappingEvent : public Event<EngineGraphicsEventType> {
		explicit SwitchToneMappingEvent(int v)
			:
			Event(EngineGraphicsEventType::SWITCH_TONE_MAPPING),
			m_value(v)
		{
		}
		int m_value;
	};

	struct SetGammaValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetGammaValueEvent(float v)
			:
			Event(EngineGraphicsEventType::SET_GAMMA_VALUE),
			m_value(v)
		{
		}
		float m_value;
	};

	struct SetSSGIValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetSSGIValueEvent(bool enable, int sample, int sameleResDownSacle, int gaussKernel, float radius, float strength)
			:
			Event(EngineGraphicsEventType::SET_SSGI_VALUE),
			m_enable(enable),
			m_sample(sample),
			m_sampleResolutionDownScale(sameleResDownSacle),
			m_gaussKernel(gaussKernel),
			m_sampleRadius(radius),
			m_strength(strength)
		{
		}
		bool m_enable;
		int m_sample;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
		float m_sampleRadius;
		float m_strength;
	};

	struct SetSSAOValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetSSAOValueEvent(bool enable, int sample, int sameleResDownSacle, int gaussKernel, float radius, float scale, float power)
			:
			Event(EngineGraphicsEventType::SET_SSAO_VALUE),
			m_enable(enable),
			m_sample(sample),
			m_sampleResolutionDownScale(sameleResDownSacle),
			m_gaussKernel(gaussKernel),
			m_sampleRadius(radius),
			m_scale(scale),
			m_power(power)
		{
		}
		bool m_enable;
		int m_sample;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
		float m_sampleRadius;
		float m_scale;
		float m_power;
	};

	struct SetSSRValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetSSRValueEvent(bool enable, int gaussKernel, int sameleResDownSacle, bool debug)
			:
			Event(EngineGraphicsEventType::SET_SSR_VALUE),
			m_enable(enable),
			m_gaussKernel(gaussKernel),
			m_sampleResolutionDownScale(sameleResDownSacle),
			m_debug(debug)
		{
		}
		bool m_enable;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
		bool m_debug;
	};

	struct SetBloomEvent : public Event<EngineGraphicsEventType> {
		explicit SetBloomEvent(bool b, float threshold, float strength, int gaussKernel, int sameleResDownSacle)
			:
			Event(EngineGraphicsEventType::SET_BLOOM_VALUE),
			m_enable(b),
			m_threshold(threshold),
			m_strength(strength),
			m_gaussKernel(gaussKernel),
			m_sampleResolutionDownScale(sameleResDownSacle)
		{
		}
		bool m_enable;
		float m_threshold;
		float m_strength;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
	};

	struct SetDOFvent : public Event<EngineGraphicsEventType> {
		explicit SetDOFvent(bool b, float threshold, float strength, int gaussKernel, int sameleResDownSacle, float refocusRate, bool debug)
			:
			Event(EngineGraphicsEventType::SET_DOF_VALUE),
			m_enable(b),
			m_threshold(threshold),
			m_strength(strength),
			m_gaussKernel(gaussKernel),
			m_sampleResolutionDownScale(sameleResDownSacle),
			m_refocusRate(refocusRate),
			m_debug(debug)
		{
		}
		bool m_enable;
		float m_threshold;
		float m_strength;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
		float m_refocusRate;
		bool m_debug;
	};

	struct SetDOFTarget : public Event<EngineGraphicsEventType> {
		explicit SetDOFTarget(bool bUseScreenSpaceTarget, const Vec2f& ssTarget, bool bUseWorldSPaceTarget, const Vec3f& wsTarget)
			:
			Event(EngineGraphicsEventType::SET_DOF_TARGET),
			m_bUseScreenSpaceTarget(bUseScreenSpaceTarget),
			m_ssTarget(ssTarget), 
			m_bUseWorldSPaceTarget(bUseWorldSPaceTarget),
			m_wsTarget(wsTarget)
		{
		}
		bool m_bUseScreenSpaceTarget;
		Vec2f m_ssTarget;
		bool m_bUseWorldSPaceTarget;
		Vec3f m_wsTarget;
	};
}