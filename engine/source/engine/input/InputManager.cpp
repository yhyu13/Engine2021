#include "engine-precompiled-header.h"
#include "engine/input/InputManager.h"
#include "engine/Engine.h"

namespace longmarch
{
    InputManager* InputManager::GetInstance()
    {
        static InputManager input;
        return &input;
    }

    InputManager::InputManager()
    {
        {
            memset(m_previousKeyBoardState, false, sizeof(m_previousKeyBoardState));
            memset(m_currentKeyBoardState, false, sizeof(m_currentKeyBoardState));

            memset(m_previousMouseButtonState, false, sizeof(m_previousMouseButtonState));
            memset(m_currentMouseButtonState, false, sizeof(m_currentMouseButtonState));
        }
        {
            m_deltaTime = std::numeric_limits<double>::lowest();
            m_updateTimeStamp = std::chrono::steady_clock::now();
            for (int i = 0; i < KEY_LAST + 1; ++i)
            {
                m_releaseKeyBoardTimeStamp[i] = m_pressKeyBoardTimeStamp[i] = m_updateTimeStamp;
            }
            for (int i = 0; i < MOUSE_BUTTON_LAST + 1; ++i)
            {
                m_releaseMouseButtonTimeStamp[i] = m_pressMouseButtonTimeStamp[i] = m_updateTimeStamp;
            }
        }
        {
            // Register input manager update to late update so that the input state is all available in any layer
            Engine::GetInstance()->EventQueueUpdate().Connect(std::bind(&InputManager::Update, this, std::placeholders::_1));
        }
    }

    void InputManager::Update(double deltaTime)
    {
        // Gamepad
        {
            LOCK_GUARD_NC();
            m_gamepad.Update();
        }
        // Keyboard & Cursor
        {
            LOCK_GUARD_NC();
            memcpy(m_previousKeyBoardState, m_currentKeyBoardState, sizeof(m_currentKeyBoardState));
            // copy current state to previous state
            memcpy(m_previousMouseButtonState, m_currentMouseButtonState, sizeof(m_currentMouseButtonState));
            // copy current state to previous state

            m_cursorPrevPosition = m_cursorPosition;
            m_mosueScrollXOffset = m_mosueScrollYOffset = 0.0f;

            auto _now = std::chrono::steady_clock::now();
            m_deltaTime = std::chrono::duration<double>(_now - m_updateTimeStamp).count();
            m_deltaTime = MIN(m_deltaTime, 1.0 / 30.0);
            m_updateTimeStamp = _now;
        }
    }

    void InputManager::UpdateKeyboardState(int key, bool state, bool repeat)
    {
        LOCK_GUARD_NC();
        m_currentKeyBoardState[key] = state;
        if (state)
        {
            m_pressKeyBoardTimeStamp[key] = std::chrono::steady_clock::now();
        }
        else
        {
            m_releaseKeyBoardTimeStamp[key] = std::chrono::steady_clock::now();
        }
    }

    void InputManager::UpdateMouseButtonState(int button, bool state)
    {
        LOCK_GUARD_NC();
        m_currentMouseButtonState[button] = state;
        if (state)
        {
            m_pressMouseButtonTimeStamp[button] = std::chrono::steady_clock::now();
        }
        else
        {
            m_releaseMouseButtonTimeStamp[button] = std::chrono::steady_clock::now();
        }
    }

    bool InputManager::IsKeyPressed(int keyCode) const
    {
        ASSERT((unsigned)keyCode <= KEY_LAST, "Invalid key-code.");
        LOCK_GUARD_NC();
        return m_currentKeyBoardState[keyCode];
    }

    bool InputManager::IsKeyTriggered(int keyCode) const
    {
        ASSERT((unsigned)keyCode <= KEY_LAST, "Invalid key-code.");
        LOCK_GUARD_NC();
        if (m_currentKeyBoardState[keyCode] && !m_previousKeyBoardState[keyCode])
        {
            return true;
        }
        else if (std::chrono::duration<double>(m_updateTimeStamp - m_pressKeyBoardTimeStamp[keyCode]).count() <=
            m_deltaTime
            && std::chrono::duration<double>(m_updateTimeStamp - m_releaseKeyBoardTimeStamp[keyCode]).count() <=
            m_deltaTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool InputManager::IsKeyReleased(int keyCode) const
    {
        ASSERT((unsigned)keyCode <= KEY_LAST, "Invalid key-code.");
        LOCK_GUARD_NC();
        if (!m_currentKeyBoardState[keyCode] && m_previousKeyBoardState[keyCode])
        {
            return true;
        }
        else if (std::chrono::duration<double>(m_releaseKeyBoardTimeStamp[keyCode] - m_pressKeyBoardTimeStamp[keyCode]).
            count() <= 0.0
            && std::chrono::duration<double>(m_updateTimeStamp - m_pressKeyBoardTimeStamp[keyCode]).count() <=
            m_deltaTime
            && std::chrono::duration<double>(m_updateTimeStamp - m_releaseKeyBoardTimeStamp[keyCode]).count() <=
            m_deltaTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool InputManager::IsMouseButtonPressed(int mouseButton) const
    {
        ASSERT((unsigned)mouseButton <= MOUSE_BUTTON_LAST, "Invalid mouse-button.");
        LOCK_GUARD_NC();
        return m_currentMouseButtonState[mouseButton];
    }

    bool InputManager::IsMouseButtonTriggered(int mouseButton) const
    {
        ASSERT((unsigned)mouseButton <= MOUSE_BUTTON_LAST, "Invalid mouse-button.");
        LOCK_GUARD_NC();
        if (m_currentMouseButtonState[mouseButton] && !m_previousMouseButtonState[mouseButton])
        {
            return true;
        }
        else if (std::chrono::duration<double>(m_updateTimeStamp - m_pressMouseButtonTimeStamp[mouseButton]).count() <=
            m_deltaTime
            && std::chrono::duration<double>(m_updateTimeStamp - m_releaseMouseButtonTimeStamp[mouseButton]).count() <=
            m_deltaTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool InputManager::IsMouseButtonReleased(int mouseButton) const
    {
        ASSERT((unsigned)mouseButton <= MOUSE_BUTTON_LAST, "Invalid mouse-button.");
        LOCK_GUARD_NC();
        if (!m_currentMouseButtonState[mouseButton] && m_previousMouseButtonState[mouseButton])
        {
            return true;
        }
        else if (std::chrono::duration<double>(
                m_releaseMouseButtonTimeStamp[mouseButton] - m_pressMouseButtonTimeStamp[mouseButton]).count() <= 0.0
            && std::chrono::duration<double>(m_updateTimeStamp - m_pressMouseButtonTimeStamp[mouseButton]).count() <=
            m_deltaTime
            && std::chrono::duration<double>(m_updateTimeStamp - m_releaseMouseButtonTimeStamp[mouseButton]).count() <=
            m_deltaTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void InputManager::UpdateCursorPosition(float positionX, float positionY)
    {
        LOCK_GUARD_NC();
        m_cursorPrevPosition.x = m_cursorPosition.x;
        m_cursorPrevPosition.y = m_cursorPosition.y;
        m_cursorPosition.x = positionX;
        m_cursorPosition.y = positionY;
    }

    const Vec2f InputManager::GetCursorPositionXY() const
    {
        LOCK_GUARD_NC();
        return m_cursorPosition;
    }

    const Vec2f InputManager::GetCursorPositionDeltaXY() const
    {
        LOCK_GUARD_NC();
        return (m_cursorPosition - m_cursorPrevPosition);
    }

    const float InputManager::GetCursorPositionX() const
    {
        LOCK_GUARD_NC();
        return m_cursorPosition.x;
    }

    const float InputManager::GetCursorPositionY() const
    {
        LOCK_GUARD_NC();
        return m_cursorPosition.y;
    }

    void InputManager::SetCursorMaxPositions(float x, float y)
    {
        LOCK_GUARD_NC();
        m_cursorMaxPosition.x = x;
        m_cursorMaxPosition.y = y;
    }

    const Vec2f InputManager::GetCursorMaxPositionXY() const
    {
        LOCK_GUARD_NC();
        return m_cursorMaxPosition;
    }

    const float InputManager::GetCursorMaxPositionX() const
    {
        LOCK_GUARD_NC();
        return m_cursorMaxPosition.x;
    }

    const float InputManager::GetCursorMaxPositionY() const
    {
        LOCK_GUARD_NC();
        return m_cursorMaxPosition.y;
    }

    const Vec2f InputManager::GetCursorScreenSpaceUnormalizedPosition() const
    {
        LOCK_GUARD_NC();
        float a = -m_cursorMaxPosition.x / m_cursorMaxPosition.y;
        float b = -a;

        float x = LongMarch_Lerp(a, b, m_cursorPosition.x / m_cursorMaxPosition.x);
        float y = LongMarch_Lerp(1.0f, -1.0f, m_cursorPosition.y / m_cursorMaxPosition.y);
        return Vec2f(x, y);
    }

    const Vec2f InputManager::GetCursorScreenSpaceNormalizedPosition() const
    {
        LOCK_GUARD_NC();
        float x = LongMarch_Lerp(-1.0f, 1.0f, m_cursorPosition.x / m_cursorMaxPosition.x);
        float y = LongMarch_Lerp(1.0f, -1.0f, m_cursorPosition.y / m_cursorMaxPosition.y);
        return Vec2f(x, y);
    }

    void InputManager::UpdateMouseScroll(float xoffset, float yoffset)
    {
        LOCK_GUARD_NC();
        m_mosueScrollXOffset = xoffset;
        m_mosueScrollYOffset = yoffset;
    }

    const Vec2f InputManager::GetMouseScrollOffsets() const
    {
        LOCK_GUARD_NC();
        return Vec2f(m_mosueScrollXOffset, m_mosueScrollYOffset);
    }

    bool InputManager::IsGamepadActive() const
    {
        LOCK_GUARD_NC();
        return m_gamepad.IsActive();
    }

    bool InputManager::IsGamepadButtonTriggered(int Button) const
    {
        LOCK_GUARD_NC();
        return m_gamepad.IsButtonTriggered(Button);
    }

    bool InputManager::IsGamepadButtonPressed(int Button) const
    {
        LOCK_GUARD_NC();
        return m_gamepad.IsButtonPressed(Button);
    }

    bool InputManager::IsGamepadButtonReleased(int Button) const
    {
        LOCK_GUARD_NC();
        return m_gamepad.IsButtonReleased(Button);
    }

    const Vec2f InputManager::GetGamepadLeftStickXY() const
    {
        LOCK_GUARD_NC();
        auto v = m_gamepad.GetLeftStickXY();
        return Vec2f{v.x, v.y};
    }

    const Vec2f InputManager::GetGamepadRightStickXY() const
    {
        LOCK_GUARD_NC();
        auto v = m_gamepad.GetRightStickXY();
        return Vec2f{v.x, v.y};
    }

    const float InputManager::GetGamepadLeftTrigger() const
    {
        LOCK_GUARD_NC();
        return m_gamepad.GetLeftTrigger();
    }

    const float InputManager::GetGamepadRightTrigger() const
    {
        LOCK_GUARD_NC();
        return m_gamepad.GetRightTrigger();
    }
}
