#pragma once

#include "Math/MathTypes.h"

namespace BA
{

class Input
{
public:
    void Initialize();
    void Shutdown();

    void BeginFrame();

    void OnKeyDown(uint32_t vkCode);
    void OnKeyUp(uint32_t vkCode);
    void OnMouseMove(int32_t x, int32_t y);
    void OnRightMouseDown(int32_t x, int32_t y);
    void OnRightMouseUp();

    bool IsKeyDown(uint32_t vkCode) const;
    bool IsKeyJustPressed(uint32_t vkCode) const;
    bool IsSystemKeyDown(uint32_t vkCode) const;
    bool IsSystemKeyJustPressed(uint32_t vkCode) const;
    Vector2 GetMouseDelta() const;
    bool IsRightMouseDown() const;

    void SetKeyboardCaptured(bool isCaptured);
    void SetMouseCaptured(bool isCaptured);

    void SetCursorLocked(bool isLocked);
    bool IsCursorLocked() const;

private:
    static constexpr uint32_t kKeyCount = 256;

    std::array<bool, kKeyCount> m_keyDown = {};
    std::array<bool, kKeyCount> m_keyDownPrev = {};
    Vector2 m_mouseDelta = {};
    int32_t m_lastMouseX = 0;
    int32_t m_lastMouseY = 0;
    bool m_rightMouseDown = false;
    bool m_isKeyboardCaptured = false;
    bool m_isMouseCaptured = false;
    bool m_isCursorLocked = false;
};

extern std::unique_ptr<Input> g_input;

} // namespace BA
