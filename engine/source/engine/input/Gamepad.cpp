#include "engine-precompiled-header.h"

#include <GLFW/glfw3.h>

#include "Gamepad.h"
#include "engine/core/EngineCore.h"

namespace AAAAgames
{
	std::function<float(float, float)> Gamepad::s_remap_stick = [](float v, float dead_zone)->float
	{
		if (fabs(v) <= dead_zone)
		{
			return 0.0f;
		}
		else
		{
			if (v >= 0.0f)
			{
				return glm::clamp((v - dead_zone) / (1.0f - dead_zone), 0.0f, 1.0f);
			}
			else
			{
				return glm::clamp((v + dead_zone) / (1.0f - dead_zone), -1.0f, 0.0f);
			}
		}
	};

	std::function<float(float, float)> Gamepad::s_remap_trigger = [](float v, float dead_zone)->float
	{
		if (v < (-1.0f + dead_zone))
		{
			return -1.0f;
		}
		else
		{
			float a = 1.0f / (1.0f + dead_zone * 0.5f);
			float b = a * dead_zone * 0.5f;
			return glm::clamp(a * v + b, -1.0f, 1.0f);
		}
	};

	Gamepad::Gamepad()
	{
		m_active = glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
		memset(m_previousButtonState, false, sizeof(m_previousButtonState));
		memset(m_currentButtonState, false, sizeof(m_currentButtonState));
		memset(m_axes, 0, sizeof(m_axes));
		ResetAllDeadZone();
	}

	void Gamepad::Update()
	{
		if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
		{
			m_active = true;
			GLFWgamepadstate gamepadState;
			if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepadState))
			{
				memcpy(m_previousButtonState, m_currentButtonState, sizeof(m_currentButtonState)); // copy current state to previous state
				for (int button : GAMEPAD_BUTTONS) // update current state
				{
					if (gamepadState.buttons[button])
					{
						m_currentButtonState[button] = true;
					}
					else
					{
						m_currentButtonState[button] = false;
					}
				}
				for (int axis : GAMEPAD_AXES)
				{
					m_axes[axis] = gamepadState.axes[axis];
				}
			}
			else
			{
				goto RESET;
			}
		}
		else
		{
			m_active = false;
		RESET:
			// Reset button states to false
			memset(m_previousButtonState, false, sizeof(m_previousButtonState));
			memset(m_currentButtonState, false, sizeof(m_currentButtonState));
			memset(m_axes, 0.0, sizeof(m_axes));
		}
	}

	bool Gamepad::IsActive() const
	{
		return m_active;
	}

	bool Gamepad::IsButtonPressed(int button) const
	{
		ASSERT((unsigned)button <= GAMEPAD_BUTTON_LAST, "Invalid button.");
		return m_currentButtonState[button];
	}

	bool Gamepad::IsButtonTriggered(int button) const
	{
		ASSERT((unsigned)button <= GAMEPAD_BUTTON_LAST, "Invalid button.");
		if (m_currentButtonState[button] && !m_previousButtonState[button]) {
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Gamepad::IsButtonReleased(int button) const
	{
		ASSERT((unsigned)button <= GAMEPAD_BUTTON_LAST, "Invalid button.");
		if (!m_currentButtonState[button] && m_previousButtonState[button]) {
			return true;
		}
		else
		{
			return false;
		}
	}

	void Gamepad::SetDeadZone(int axis, float v)
	{
		ASSERT((unsigned)axis <= GAMEPAD_AXIS_LAST, "Invalid button.");
		ASSERT(fabs(v) < 1.0f, "Invalid dead zone.");
		m_axes_dead_zone[axis] = fabs(v);
	}

	void Gamepad::ResetDeadZone(int axis)
	{
		ASSERT((unsigned)axis <= GAMEPAD_AXIS_LAST, "Invalid button.");
		m_axes_dead_zone[axis] = DEADZONE_CONSTNAT;
	}

	void Gamepad::ResetAllDeadZone()
	{
		for (int axis : GAMEPAD_AXES)
		{
			m_axes_dead_zone[axis] = DEADZONE_CONSTNAT;
		}
	}

	const XY_Pair Gamepad::GetRightStickXY() const
	{
		return XY_Pair{ GetRightStickX(), GetRightStickY() };
	}

	const XY_Pair Gamepad::GetLeftStickXY() const
	{
		return XY_Pair{ GetLeftStickX(), GetLeftStickY() };
	}

	const float Gamepad::GetRightStickX() const
	{
		float v = m_axes[GAMEPAD_AXIS_RIGHT_X];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_RIGHT_X];
		return s_remap_stick(v, v_dead);
	}

	const float Gamepad::GetRightStickY() const
	{
		float v = m_axes[GAMEPAD_AXIS_RIGHT_Y];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_RIGHT_Y];
		return s_remap_stick(v, v_dead);
	}

	const float Gamepad::GetLeftStickX() const
	{
		float v = m_axes[GAMEPAD_AXIS_LEFT_X];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_LEFT_X];
		return s_remap_stick(v, v_dead);
	}

	const float Gamepad::GetLeftStickY() const
	{
		float v = m_axes[GAMEPAD_AXIS_LEFT_Y];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_LEFT_Y];
		return s_remap_stick(v, v_dead);
	}

	const float Gamepad::GetLeftTrigger() const
	{
		float v = m_axes[GAMEPAD_AXIS_LEFT_TRIGGER];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_LEFT_TRIGGER];
		return s_remap_trigger(v, v_dead);
	}

	const float Gamepad::GetRightTrigger() const
	{
		float v = m_axes[GAMEPAD_AXIS_RIGHT_TRIGGER];
		float v_dead = m_axes_dead_zone[GAMEPAD_AXIS_RIGHT_TRIGGER];
		return s_remap_trigger(v, v_dead);
	}
}