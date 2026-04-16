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

Float4x4 Inverse(const Float4x4& matrix)
{
    // The classic 4x4 cofactor expansion (GLU-style) is written assuming
    // column-major flat storage. We store row-major, which means feeding
    // our flat array in is equivalent to feeding the formula M^T. The
    // formula then produces (M^T)^-1 = (M^-1)^T in column-major, and
    // writing that output directly back into row-major slots transposes
    // it one more time, yielding M^-1. Two implicit transposes cancel,
    // so the formula can be used verbatim — no transpose buffers needed.
    const float* m = &matrix.m[0][0];

    Float4x4 result;
    float* inv = &result.m[0][0];

    inv[0]  =  m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8]  =  m[4] * m[9]  * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9]  * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5]  =  m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9]  = -m[0] * m[9]  * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] =  m[0] * m[9]  * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2]  =  m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] =  m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7]  =  m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9]  + m[4] * m[1] * m[11] - m[4] * m[3] * m[9]  - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] =  m[0] * m[5] * m[10] - m[0] * m[6] * m[9]  - m[4] * m[1] * m[10] + m[4] * m[2] * m[9]  + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    constexpr float kDeterminantEpsilon = 1e-8f;
    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    BA_ASSERT(fabsf(det) > kDeterminantEpsilon);

    float invDet = 1.0f / det;
    for (int i = 0; i < 16; ++i)
    {
        inv[i] *= invDet;
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

Float4 TransformHomogeneous(const Float4& v, const Float4x4& matrix)
{
    return {
        v.x * matrix.m[0][0] + v.y * matrix.m[1][0] + v.z * matrix.m[2][0] + v.w * matrix.m[3][0],
        v.x * matrix.m[0][1] + v.y * matrix.m[1][1] + v.z * matrix.m[2][1] + v.w * matrix.m[3][1],
        v.x * matrix.m[0][2] + v.y * matrix.m[1][2] + v.z * matrix.m[2][2] + v.w * matrix.m[3][2],
        v.x * matrix.m[0][3] + v.y * matrix.m[1][3] + v.z * matrix.m[2][3] + v.w * matrix.m[3][3],
    };
}

} // namespace BA
