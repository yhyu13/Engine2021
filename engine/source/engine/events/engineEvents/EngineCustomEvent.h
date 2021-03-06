#pragma once

#include "EngineEventType.h"
#include "engine/events/EventQueue.h"
#include "engine/ecs/EntityDecorator.h"

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

	
	struct ToggleEnvironmentMappingEvent : public Event<EngineGraphicsDebugEventType> {
		explicit ToggleEnvironmentMappingEvent(bool b)
			:
			Event(EngineGraphicsDebugEventType::TOGGLE_ENV_MAPPING),
			m_enable(b)
		{
		}
		bool m_enable;
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
		explicit ToggleSMAAEvent(bool b)
			:
			Event(EngineGraphicsEventType::TOGGLE_SMAA),
			m_enable(b)
		{
		}
		bool m_enable;
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

	struct SetAOValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetAOValueEvent(bool enable, int sample, int sameleResDownSacle, int gaussKernel, float radius, float scale, float power, bool bounce)
			:
			Event(EngineGraphicsEventType::SET_AO_VALUE),
			m_enable(enable),
			m_sample(sample),
			m_sampleResolutionDownScale(sameleResDownSacle),
			m_gaussKernel(gaussKernel),
			m_sampleRadius(radius),
			m_scale(scale),
			m_power(power),
			m_enable_indirect_bounce(bounce)
		{
		}
		bool m_enable;
		int m_sample;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
		float m_sampleRadius;
		float m_scale;
		float m_power;
		bool m_enable_indirect_bounce;
	};

	struct SetSSRValueEvent : public Event<EngineGraphicsEventType> {
		explicit SetSSRValueEvent(bool enable, int gaussKernel, float sameleResDownSacle)
			:
			Event(EngineGraphicsEventType::SET_SSR_VALUE),
			m_enable(enable),
			m_gaussKernel(gaussKernel),
			m_sampleResolutionDownScale(sameleResDownSacle)
		{
		}
		bool m_enable;
		int m_gaussKernel;
		int m_sampleResolutionDownScale;
	};

	struct SetBloomEvent : public Event<EngineGraphicsEventType> {
		explicit SetBloomEvent(bool b, float threshold, float strength)
			:
			Event(EngineGraphicsEventType::SET_BLOOM_VALUE),
			m_enable(b),
			m_threshold(threshold),
			m_strength(strength)
		{
		}
		bool m_enable;
		float m_threshold;
		float m_strength;
	};

}