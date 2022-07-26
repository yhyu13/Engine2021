#pragma once
#include "engine/math/Geommath.h"
#include "engine/input/MouseButton.h"
#include "engine/input/KeyboardKeys.h"
#include "engine/input/Gamepad.h"
#include "engine/core/thread/Lock.h"

namespace longmarch
{
    // mouse cursor coordinate : origin at upper left conrer, +x = right, +y = down. Range from [0,width],[0,height]
    // screen space(aka. view) un-normalized coordinate : origin at center, +x= right, +y = up. Range from [-aspectWByH,aspectWByH],[-1,1]
    // screen space(aka. view) normalized coordinate : origin at center, +x= right, +y = up. Range from [-1,1],[-1,1]
    class ENGINE_API InputManager final : private BaseAtomicClassNC
    {
    private:
        NONCOPYABLE(InputManager);
        InputManager();

    public:
        static InputManager* GetInstance();

        //! Input states of current frame will not be available after Update()
        void Update(double deltaTime);

        void UpdateKeyboardState(int key, bool state, bool repeat);
        bool IsKeyPressed(int keyCode) const;
        bool IsKeyTriggered(int keyCode) const;
        bool IsKeyReleased(int keyCode) const;

        void UpdateMouseButtonState(int button, bool state);
        bool IsMouseButtonTriggered(int mouseButton) const;
        bool IsMouseButtonPressed(int mouseButton) const;
        bool IsMouseButtonReleased(int mouseButton) const;

        void UpdateMouseScroll(float xoffset, float yoffset);
        const Vec2f GetMouseScrollOffsets() const;

        void UpdateCursorPosition(float positionX, float positionY);
        const Vec2f GetCursorPositionXY() const;
        const Vec2f GetCursorPositionDeltaXY() const;
        const float GetCursorPositionX() const;
        const float GetCursorPositionY() const;
        void SetCursorMaxPositions(float x, float y);
        const Vec2f GetCursorMaxPositionXY() const;
        const float GetCursorMaxPositionX() const;
        const float GetCursorMaxPositionY() const;

        const Vec2f GetCursorScreenSpaceUnormalizedPosition() const;
        const Vec2f GetCursorScreenSpaceNormalizedPosition() const;

        bool IsGamepadActive() const;
        bool IsGamepadButtonTriggered(int Button) const;
        bool IsGamepadButtonPressed(int Button) const;
        bool IsGamepadButtonReleased(int Button) const;
        const Vec2f GetGamepadLeftStickXY() const;
        const Vec2f GetGamepadRightStickXY() const;
        const float GetGamepadLeftTrigger() const;
        const float GetGamepadRightTrigger() const;

    private:
        bool m_previousKeyBoardState[KEY_LAST + 1];
        bool m_currentKeyBoardState[KEY_LAST + 1];

        bool m_previousMouseButtonState[MOUSE_BUTTON_LAST + 1];
        bool m_currentMouseButtonState[MOUSE_BUTTON_LAST + 1];

        std::chrono::steady_clock::time_point m_releaseKeyBoardTimeStamp[KEY_LAST + 1];
        std::chrono::steady_clock::time_point m_pressKeyBoardTimeStamp[KEY_LAST + 1];

        std::chrono::steady_clock::time_point m_releaseMouseButtonTimeStamp[MOUSE_BUTTON_LAST + 1];
        std::chrono::steady_clock::time_point m_pressMouseButtonTimeStamp[MOUSE_BUTTON_LAST + 1];

        // Gamepad
        longmarch::Gamepad m_gamepad;

        Vec2f m_cursorPrevPosition;
        Vec2f m_cursorPosition;
        Vec2f m_cursorMaxPosition;

        std::chrono::steady_clock::time_point m_updateTimeStamp;
        double m_deltaTime;
        float m_mosueScrollXOffset;
        float m_mosueScrollYOffset;
    };
}
