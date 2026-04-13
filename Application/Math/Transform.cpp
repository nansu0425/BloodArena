#include "Core/PCH.h"
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

} // namespace BA
