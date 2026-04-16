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
    Vector2 GetMouseDelta() const;
    bool IsRightMouseDown() const;

private:
    static constexpr uint32_t kKeyCount = 256;

    std::array<bool, kKeyCount> m_keyDown = {};
    Vector2 m_mouseDelta = {};
    int32_t m_lastMouseX = 0;
    int32_t m_lastMouseY = 0;
    bool m_rightMouseDown = false;
};

extern std::unique_ptr<Input> g_input;

} // namespace BA
