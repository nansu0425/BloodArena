#pragma once

#include "Math/Float3.h"

// Project axis convention (left-handed, Y-up):
//   +X = right, +Y = up, +Z = forward
// These basis vectors apply to any coordinate frame in the project
// (world, local, view, etc.) since all frames share the same handedness
// and axis role assignment.

namespace BA
{

inline const Float3 kAxisRight   = {1.0f, 0.0f, 0.0f};
inline const Float3 kAxisUp      = {0.0f, 1.0f, 0.0f};
inline const Float3 kAxisForward = {0.0f, 0.0f, 1.0f};

} // namespace BA