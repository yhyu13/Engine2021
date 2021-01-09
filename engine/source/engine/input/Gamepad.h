#pragma once

#include "GamepadButtonsAndAxes.h"
#include <functional>

namespace longmarch
{
	struct XY_Pair
	{
		float x, y;
	};

	class Gamepad
	{
	public:
		Gamepad(const Gamepad&) = delete; Gamepad(const Gamepad&&) = delete; \
			Gamepad& operator=(const Gamepad&) = delete; Gamepad& operator=(const Gamepad&&) = delete;

		Gamepad();
		void Update();
		bool IsActive() const;
		const XY_Pair GetRightStickXY() const;
		const XY_Pair GetLeftStickXY() const;
		const float GetRightStickX() const;
		const float GetRightStickY() const;
		const float GetLeftStickX() const;
		const float GetLeftStickY() const;
		const float GetLeftTrigger() const;
		const float GetRightTrigger() const;
		bool IsButtonTriggered(int button) const;
		bool IsButtonPressed(int button) const;
		bool IsButtonReleased(int button) const;
		void SetDeadZone(int axis, float v);
		void ResetDeadZone(int axis);
		void ResetAllDeadZone();
	private:
		float m_axes[GAMEPAD_AXIS_LAST + 1];
		float m_axes_dead_zone[GAMEPAD_AXIS_LAST + 1];
		bool m_previousButtonState[GAMEPAD_BUTTON_LAST + 1];
		bool m_currentButtonState[GAMEPAD_BUTTON_LAST + 1];
		bool m_active;

		static std::function<float(float, float)> s_remap_stick;
		static std::function<float(float, float)> s_remap_trigger;
		constexpr static float DEADZONE_CONSTNAT = { 0.5f };
		constexpr static int GAMEPAD_BUTTONS[] = { GAMEPAD_BUTTON_A, GAMEPAD_BUTTON_B,GAMEPAD_BUTTON_X, GAMEPAD_BUTTON_Y, GAMEPAD_BUTTON_LEFT_BUMPER, GAMEPAD_BUTTON_RIGHT_BUMPER, GAMEPAD_BUTTON_BACK, GAMEPAD_BUTTON_START, GAMEPAD_BUTTON_GUIDE, GAMEPAD_BUTTON_LEFT_THUMB, GAMEPAD_BUTTON_RIGHT_THUMB, GAMEPAD_BUTTON_DPAD_UP, GAMEPAD_BUTTON_DPAD_RIGHT, GAMEPAD_BUTTON_DPAD_DOWN, GAMEPAD_BUTTON_DPAD_LEFT, GAMEPAD_BUTTON_LAST };
		constexpr static int GAMEPAD_AXES[] = { GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_AXIS_RIGHT_Y, GAMEPAD_AXIS_LEFT_TRIGGER, GAMEPAD_AXIS_RIGHT_TRIGGER, GAMEPAD_AXIS_LAST };
	};
}
