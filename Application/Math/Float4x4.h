#pragma once

#include "Math/Float3.h"

// Conventions:
//   Coordinate system : Left-handed
//   Matrix storage    : row-major
//   Vector-matrix mul : row-vector (v * M)

namespace BA
{

struct Float4x4
{
    float m[4][4] = {};

    bool operator==(const Float4x4& rhs) const;
    bool operator!=(const Float4x4& rhs) const;
};

Float4x4 operator*(const Float4x4& a, const Float4x4& b);

Float4x4 BuildIdentity();
Float4x4 Transpose(const Float4x4& matrix);
Float4x4 Inverse(const Float4x4& matrix);

Float4x4 BuildTranslation(const Float3& translation);
Float4x4 BuildScale(const Float3& scale);
Float4x4 BuildRotationX(float radians);
Float4x4 BuildRotationY(float radians);
Float4x4 BuildRotationZ(float radians);

Float4x4 BuildLookAt(const Float3& eye, const Float3& target, const Float3& up);
Float4x4 BuildPerspective(float fovY, float aspect, float nearZ, float farZ);
Float4x4 BuildOrthographic(float width, float height, float nearZ, float farZ);

Float3 TransformPoint(const Float3& point, const Float4x4& matrix);
Float3 TransformVector(const Float3& vector, const Float4x4& matrix);
Float4 TransformHomogeneous(const Float4& v, const Float4x4& matrix);

} // namespace BA
