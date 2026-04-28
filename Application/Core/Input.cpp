#include "Core/PCH.h"
#include "Core/Input.h"
#include "Core/Window.h"

namespace BA
{

namespace
{

struct WindowCenterPoint
{
    int clientX;
    int clientY;
    int screenX;
    int screenY;
};

WindowCenterPoint GetWindowCenterPoint(HWND hwnd)
{
    RECT clientRect = {};
    GetClientRect(hwnd, &clientRect);
    POINT point = {
        (clientRect.right - clientRect.left) / 2,
        (clientRect.bottom - clientRect.top) / 2
    };

    const int clientX = point.x;
    const int clientY = point.y;
    ClientToScreen(hwnd, &point);

    return WindowCenterPoint{ clientX, clientY, point.x, point.y };
}

} // namespace

void Input::Initialize()
{
    m_keyDown.fill(false);
    m_keyDownPrev.fill(false);
    m_mouseDelta = {};
    m_rightMouseDown = false;
    m_isKeyboardCaptured = false;
    m_isMouseCaptured = false;
    m_isCursorLocked = false;
    m_consumeFirstLockedDelta = false;

    BA_LOG_INFO("Input initialized.");
}

void Input::Shutdown()
{
    BA_LOG_INFO("Input shutdown.");
}

void Input::BeginFrame()
{
    m_keyDownPrev = m_keyDown;
    m_mouseDelta = {};

    if (m_isCursorLocked && g_window)
    {
        WindowCenterPoint center = GetWindowCenterPoint(g_window->GetHandle());
        SetCursorPos(center.screenX, center.screenY);
        m_lastMouseX = center.clientX;
        m_lastMouseY = center.clientY;
    }
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
    if (!m_rightMouseDown && !m_isCursorLocked)
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
    if (!m_isCursorLocked && m_isKeyboardCaptured)
    {
        return false;
    }

    return m_keyDown[vkCode];
}

bool Input::IsKeyJustPressed(uint32_t vkCode) const
{
    BA_ASSERT(vkCode < kKeyCount);
    if (!m_isCursorLocked && m_isKeyboardCaptured)
    {
        return false;
    }

    return m_keyDown[vkCode] && !m_keyDownPrev[vkCode];
}

bool Input::IsSystemKeyDown(uint32_t vkCode) const
{
    BA_ASSERT(vkCode < kKeyCount);
    return m_keyDown[vkCode];
}

bool Input::IsSystemKeyJustPressed(uint32_t vkCode) const
{
    BA_ASSERT(vkCode < kKeyCount);
    return m_keyDown[vkCode] && !m_keyDownPrev[vkCode];
}

Vector2 Input::GetMouseDelta() const
{
    if (!m_isCursorLocked && m_isMouseCaptured)
    {
        return {0.0f, 0.0f};
    }

    return m_mouseDelta;
}

bool Input::IsRightMouseDown() const
{
    if (m_isMouseCaptured)
    {
        return false;
    }
    return m_rightMouseDown;
}

void Input::SetKeyboardCaptured(bool isCaptured)
{
    m_isKeyboardCaptured = isCaptured;
}

void Input::SetMouseCaptured(bool isCaptured)
{
    m_isMouseCaptured = isCaptured;
}

void Input::SetCursorLocked(bool isLocked)
{
    if (m_isCursorLocked == isLocked)
    {
        return;
    }

    m_isCursorLocked = isLocked;
    m_mouseDelta = {};

    if (isLocked)
    {
        m_consumeFirstLockedDelta = true;
        ShowCursor(FALSE);
    }
    else
    {
        ShowCursor(TRUE);
    }
}

void Input::ConsumeFirstLockedDelta()
{
    if (!m_consumeFirstLockedDelta)
    {
        return;
    }

    m_mouseDelta = {};
    m_consumeFirstLockedDelta = false;
}

bool Input::IsCursorLocked() const
{
    return m_isCursorLocked;
}

std::unique_ptr<Input> g_input;

} // namespace BA
