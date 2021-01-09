#pragma once
#include "ecs/EntityType.h"
#include "events/EventType.h"
#include "engine/events/EventQueue.h"

namespace AAAAgames
{
	struct CS560GenRndPathEvent : public Event<CS560EventType> {
		explicit CS560GenRndPathEvent(const EntityDecorator& entity)
			:
			Event(CS560EventType::GEN_RND_PATH),
			m_entity(entity)
		{
		}
		EntityDecorator m_entity;
	};

	struct CS560PatherControlEvent : public Event<CS560EventType> {
		explicit CS560PatherControlEvent(const EntityDecorator& entity, bool restart, bool pause)
			:
			Event(CS560EventType::SET_PATHER_CONTROL),
			m_entity(entity),
			m_restart(restart),
			m_pause(pause)
		{
		}
		EntityDecorator m_entity;
		bool m_restart;
		bool m_pause;
	};

	struct CS560PatherMiscEvent : public Event<CS560EventType> {
		explicit CS560PatherMiscEvent(const EntityDecorator& entity, float speed, float speed_ramp_up_time, float speed_ramp_down_time)
			:
			Event(CS560EventType::SET_PATHER_MISC),
			m_entity(entity),
			m_speed(speed),
			m_speed_ramp_up_time(speed_ramp_up_time),
			m_speed_ramp_down_time(speed_ramp_down_time)
		{
		}
		EntityDecorator m_entity;
		float m_speed;
		float m_speed_ramp_up_time;
		float m_speed_ramp_down_time;
	};

	struct CS560PatherSmoothSettingEvent : public Event<CS560EventType> {
		explicit CS560PatherSmoothSettingEvent(const EntityDecorator& entity, float angle_rad_tolerance, float delta_distance_tolerance)
			:
			Event(CS560EventType::SET_PATHER_SMOOTH_SETTING),
			m_entity(entity),
			m_angle(angle_rad_tolerance),
			m_distance(delta_distance_tolerance)
		{
		}
		EntityDecorator m_entity;
		float m_angle;
		float m_distance;
	};


	struct Prototype2PlayerGenerateProjectile : public Event<Prototype2EventType> {
		explicit Prototype2PlayerGenerateProjectile(const Quaternion& player_orientation, const Vec3f& player_position)
			:
			Event(Prototype2EventType::GEN_PROJECTILE),
			m_player_orientation(player_orientation),
			m_player_position(player_position)
		{
		}
		Quaternion m_player_orientation;
		Vec3f m_player_position;
	};

	struct Prototype2PlayerGenerateSpaceShip : public Event<Prototype2EventType> {
		explicit Prototype2PlayerGenerateSpaceShip(const Quaternion& player_orientation, const Vec3f& player_position)
			:
			Event(Prototype2EventType::GEN_SPACE_SHIP),
			m_player_orientation(player_orientation),
			m_player_position(player_position)
		{
		}
		Quaternion m_player_orientation;
		Vec3f m_player_position;
	};

	struct Prototype2SpaceShipGenerateProjectile : public Event<Prototype2EventType> {
		explicit Prototype2SpaceShipGenerateProjectile(EntityDecorator e, const Quaternion& player_orientation, const Vec3f& player_position)
			:
			Event(Prototype2EventType::GEN_PROJECTILE),
			m_entity(e),
			m_player_orientation(player_orientation),
			m_player_position(player_position)
		{
		}
		EntityDecorator m_entity;
		Quaternion m_player_orientation;
		Vec3f m_player_position;
	};
}