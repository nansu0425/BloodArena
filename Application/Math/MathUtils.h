#pragma once

namespace BA
{

struct Transform;

void BuildWorldMatrix(const Transform& transform, float outMatrix[4][4]);

} // namespace BA
