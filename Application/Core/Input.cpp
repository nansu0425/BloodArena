#include "Core/PCH.h"
#include "Core/Input.h"

namespace BA
{

void Input::Initialize()
{
    m_keyDown.fill(false);
    m_mouseDelta = {};
    m_rightMouseDown = false;

    BA_LOG_INFO("Input initialized.");
}

void Input::Shutdown()
{
    BA_LOG_INFO("Input shutdown.");
}

void Input::BeginFrame()
{
    m_mouseDelta = {};
}

void Input::OnKeyDown(uint32_t vkCode)
{
    BA_ASSERT(vkCode < kKeyCount);
    m_keyDown[vkCode] = true;
}

void Input::OnKeyUp(uint32_t vkCode)
{
    BA_ASSERT(vkCode < kKeyCount);
    m_keyDown[vkCode] = false;
}

void Input::OnMouseMove(int32_t x, int32_t y)
{
    if (!m_rightMouseDown)
    {
        return;
    }

    m_mouseDelta.x += static_cast<float>(x - m_lastMouseX);
    m_mouseDelta.y += static_cast<float>(y - m_lastMouseY);
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void Input::OnRightMouseDown(int32_t x, int32_t y)
{
    m_rightMouseDown = true;
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void Input::OnRightMouseUp()
{
    m_rightMouseDown = false;
}

bool Input::IsKeyDown(uint32_t vkCode) const
{
    BA_ASSERT(vkCode < kKeyCount);
    return m_keyDown[vkCode];
}

Vector2 Input::GetMouseDelta() const
{
    return m_mouseDelta;
}

bool Input::IsRightMouseDown() const
{
    return m_rightMouseDown;
}

std::unique_ptr<Input> g_input;

} // namespace BA
