#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/math/Geommath.h"
#include "events/EventType.h"

namespace longmarch
{
	struct PathMoverData
	{
		LongMarch_Vector<Vec3f> original_pathLUT;
		LongMarch_Vector<Vec3f> smooth_pathLUT;
		LongMarch_Vector<float> smooth_path_arc_lengthLUT;

		EntityDecorator path_agent;
		EntityDecorator debug_path_root_node;
		EntityDecorator debug_smooth_path_root_node;

		float current_time{ 0.f }; //!< current time after moving along the path, updated by incramenting by delta time
		float current_distance{ 0.f }; //!< current distance along the path
		float current_speed{ 0.f }; //! current speed along the path

		float target_speed{ 2.5f };
		float speed_ramp_up_time{ 1.f };
		float speed_ramp_down_time{ 1.f };
		float total_time{ -1.f }; //!< total time determined by intergrating the trapezoidal made by speed ramping up/down and the total arc length
		bool pause{ false };
	};

	class NPCPathFindingControllerComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(NPCPathFindingControllerComSys);
		COMSYS_DEFAULT_COPY(NPCPathFindingControllerComSys);

		NPCPathFindingControllerComSys() = default;
		virtual void Init() override;
		virtual void Update(double dt) override;

	private:
		void DebugDrawOriginalPath(PathMoverData& data);
		void DebugDrawSmoothPath(PathMoverData& data);

		void _ON_GEN_RND_PATH(EventQueue<CS560EventType>::EventPtr e);
		void _ON_SET_PATHER_CONTROL(EventQueue<CS560EventType>::EventPtr e);
		void _ON_SET_PATHER_MISC(EventQueue<CS560EventType>::EventPtr e);
		void _ON_SET_PATHER_SMOOTH_SETTING(EventQueue<CS560EventType>::EventPtr e);

	private:
		LongMarch_UnorderedMap_flat<Entity, PathMoverData> m_PatherLUT;

	private:
		inline static Vec3f Points[] = { Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10),
							Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10),
							Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10),
							Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10),
							Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), Vec3f(RAND_I(-30,30), RAND_I(-30,30), -10), };
		inline constexpr static int NumPoints = sizeof(Points) / sizeof(Vec3f);
	};
}