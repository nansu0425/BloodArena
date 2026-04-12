#include "Core/PCH.h"
#include "Math/MathUtils.h"
#include "Math/Transform.h"

namespace BA
{

Matrix4x4 BuildWorldMatrix(const Transform& transform)
{
    float cos = cosf(transform.rotation);
    float sin = sinf(transform.rotation);

    float sx = transform.scale[0];
    float sy = transform.scale[1];
    float sz = transform.scale[2];

    float tx = transform.position[0];
    float ty = transform.position[1];
    float tz = transform.position[2];

    Matrix4x4 result = {};

    result.m[0][0] = sx * cos;
    result.m[0][1] = sx * sin;

    result.m[1][0] = -sy * sin;
    result.m[1][1] = sy * cos;

    result.m[2][2] = sz;

    result.m[3][0] = tx;
    result.m[3][1] = ty;
    result.m[3][2] = tz;
    result.m[3][3] = 1.0f;

    return result;
}

Float3 TransformPoint(const float point[3], const Matrix4x4& matrix)
{
    Float3 result;
    result.x = point[0] * matrix.m[0][0] + point[1] * matrix.m[1][0] + point[2] * matrix.m[2][0] + matrix.m[3][0];
    result.y = point[0] * matrix.m[0][1] + point[1] * matrix.m[1][1] + point[2] * matrix.m[2][1] + matrix.m[3][1];
    result.z = point[0] * matrix.m[0][2] + point[1] * matrix.m[1][2] + point[2] * matrix.m[2][2] + matrix.m[3][2];
    return result;
}

bool IsPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float d1 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    float d2 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
    float d3 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);

    bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
    bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);

    return !(hasNeg && hasPos);
}

} // namespace BA
