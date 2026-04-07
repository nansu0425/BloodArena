#include "Core/PCH.h"
#include "Math/MathUtils.h"
#include "Math/Transform.h"

namespace BA
{

void BuildWorldMatrix(const Transform& transform, float outMatrix[4][4])
{
    float cos = cosf(transform.rotation);
    float sin = sinf(transform.rotation);

    float sx = transform.scale[0];
    float sy = transform.scale[1];
    float sz = transform.scale[2];

    float tx = transform.position[0];
    float ty = transform.position[1];
    float tz = transform.position[2];

    outMatrix[0][0] = sx * cos;
    outMatrix[0][1] = sx * sin;
    outMatrix[0][2] = 0.0f;
    outMatrix[0][3] = 0.0f;

    outMatrix[1][0] = -sy * sin;
    outMatrix[1][1] = sy * cos;
    outMatrix[1][2] = 0.0f;
    outMatrix[1][3] = 0.0f;

    outMatrix[2][0] = 0.0f;
    outMatrix[2][1] = 0.0f;
    outMatrix[2][2] = sz;
    outMatrix[2][3] = 0.0f;

    outMatrix[3][0] = tx;
    outMatrix[3][1] = ty;
    outMatrix[3][2] = tz;
    outMatrix[3][3] = 1.0f;
}

} // namespace BA
