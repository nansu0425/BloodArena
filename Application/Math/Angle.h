#pragma once

#include <numbers>

namespace BA
{

inline constexpr float kPi = std::numbers::pi_v<float>;

constexpr float DegToRad(float degrees)
{
    return degrees * (kPi / 180.0f);
}

constexpr float RadToDeg(float radians)
{
    return radians * (180.0f / kPi);
}

} // namespace BA