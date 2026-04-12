#include "Core/PCH.h"
#include "Math/Float4x4.h"

namespace BA
{

bool Float4x4::operator==(const Float4x4& rhs) const
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (m[i][j] != rhs.m[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

bool Float4x4::operator!=(const Float4x4& rhs) const
{
    return !(*this == rhs);
}

Float4x4 operator*(const Float4x4& a, const Float4x4& b)
{
    Float4x4 result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.m[i][j] =
                a.m[i][0] * b.m[0][j] +
                a.m[i][1] * b.m[1][j] +
                a.m[i][2] * b.m[2][j] +
                a.m[i][3] * b.m[3][j];
        }
    }
    return result;
}

Float4x4 BuildIdentity()
{
    Float4x4 result;
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

Float4x4 Transpose(const Float4x4& matrix)
{
    Float4x4 result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.m[i][j] = matrix.m[j][i];
        }
    }
    return result;
}

Float4x4 BuildTranslation(const Float3& translation)
{
    Float4x4 result = BuildIdentity();
    result.m[3][0] = translation.x;
    result.m[3][1] = translation.y;
    result.m[3][2] = translation.z;
    return result;
}

Float4x4 BuildScale(const Float3& scale)
{
    Float4x4 result;
    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;
    result.m[3][3] = 1.0f;
    return result;
}

Float4x4 BuildRotationX(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    Float4x4 result = BuildIdentity();
    result.m[1][1] =  c;
    result.m[1][2] =  s;
    result.m[2][1] = -s;
    result.m[2][2] =  c;
    return result;
}

Float4x4 BuildRotationY(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    Float4x4 result = BuildIdentity();
    result.m[0][0] =  c;
    result.m[0][2] = -s;
    result.m[2][0] =  s;
    result.m[2][2] =  c;
    return result;
}

Float4x4 BuildRotationZ(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    Float4x4 result = BuildIdentity();
    result.m[0][0] =  c;
    result.m[0][1] =  s;
    result.m[1][0] = -s;
    result.m[1][1] =  c;
    return result;
}

Float4x4 BuildLookAt(const Float3& eye, const Float3& target, const Float3& up)
{
    Float3 zAxis = Normalize(target - eye);
    Float3 xAxis = Normalize(Cross(up, zAxis));
    Float3 yAxis = Cross(zAxis, xAxis);

    Float4x4 result;
    result.m[0][0] = xAxis.x;
    result.m[0][1] = yAxis.x;
    result.m[0][2] = zAxis.x;
    result.m[0][3] = 0.0f;

    result.m[1][0] = xAxis.y;
    result.m[1][1] = yAxis.y;
    result.m[1][2] = zAxis.y;
    result.m[1][3] = 0.0f;

    result.m[2][0] = xAxis.z;
    result.m[2][1] = yAxis.z;
    result.m[2][2] = zAxis.z;
    result.m[2][3] = 0.0f;

    result.m[3][0] = -Dot(xAxis, eye);
    result.m[3][1] = -Dot(yAxis, eye);
    result.m[3][2] = -Dot(zAxis, eye);
    result.m[3][3] = 1.0f;
    return result;
}

Float4x4 BuildPerspective(float fovY, float aspect, float nearZ, float farZ)
{
    BA_ASSERT(aspect > 0.0f);
    BA_ASSERT(farZ > nearZ);

    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;
    float zRange = farZ - nearZ;

    Float4x4 result;
    result.m[0][0] = xScale;
    result.m[1][1] = yScale;
    result.m[2][2] = farZ / zRange;
    result.m[2][3] = 1.0f;
    result.m[3][2] = -nearZ * farZ / zRange;
    return result;
}

Float4x4 BuildOrthographic(float width, float height, float nearZ, float farZ)
{
    BA_ASSERT(width > 0.0f);
    BA_ASSERT(height > 0.0f);
    BA_ASSERT(farZ > nearZ);

    float zRange = farZ - nearZ;

    Float4x4 result;
    result.m[0][0] = 2.0f / width;
    result.m[1][1] = 2.0f / height;
    result.m[2][2] = 1.0f / zRange;
    result.m[3][2] = -nearZ / zRange;
    result.m[3][3] = 1.0f;
    return result;
}

Float3 TransformPoint(const Float3& point, const Float4x4& matrix)
{
    return {
        point.x * matrix.m[0][0] + point.y * matrix.m[1][0] + point.z * matrix.m[2][0] + matrix.m[3][0],
        point.x * matrix.m[0][1] + point.y * matrix.m[1][1] + point.z * matrix.m[2][1] + matrix.m[3][1],
        point.x * matrix.m[0][2] + point.y * matrix.m[1][2] + point.z * matrix.m[2][2] + matrix.m[3][2],
    };
}

Float3 TransformVector(const Float3& vector, const Float4x4& matrix)
{
    return {
        vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0],
        vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1],
        vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2],
    };
}

} // namespace BA
