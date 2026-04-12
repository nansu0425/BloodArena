#include "Core/PCH.h"
#include "Math/MathUtils.h"
#include "Math/Transform.h"

namespace BA
{

Float4x4 BuildWorld(const Transform& transform)
{
    return BuildScale(transform.scale)
         * BuildRotationZ(transform.rotation.z)
         * BuildRotationX(transform.rotation.x)
         * BuildRotationY(transform.rotation.y)
         * BuildTranslation(transform.position);
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
