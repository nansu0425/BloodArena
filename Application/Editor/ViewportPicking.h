#pragma once

#include <cstdint>
#include "Math/MathTypes.h"

namespace BA
{

uint32_t PickGameObject(float ndcX, float ndcY,
                        const Matrix& view, const Matrix& projection);

} // namespace BA
