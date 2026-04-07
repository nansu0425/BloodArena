#pragma once

namespace BA
{

struct Transform
{
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation    = 0.0f; // Left-handed Z-Axis
    float scale[3]    = {1.0f, 1.0f, 1.0f};
};

} // namespace BA
