#pragma once

#include "Math/Aabb.h"

namespace BA
{

class Scene;

Aabb ComputeSceneWorldAabb(const Scene& scene);

} // namespace BA
