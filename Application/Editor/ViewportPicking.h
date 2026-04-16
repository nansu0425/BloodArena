#pragma once

#include <cstdint>

namespace BA
{

class Camera;

uint32_t PickGameObject(float ndcX, float ndcY, const Camera& camera, float aspect);

} // namespace BA
