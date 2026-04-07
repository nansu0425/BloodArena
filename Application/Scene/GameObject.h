#pragma once

namespace BA
{

struct GameObject
{
    uint32_t m_id = 0;
    float m_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_position[2] = {0.0f, 0.0f};
};

} // namespace BA
