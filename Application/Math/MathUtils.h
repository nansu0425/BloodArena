#pragma once

namespace BA
{

struct Float3
{
    float x, y, z;
};

struct Matrix4x4
{
    float m[4][4];
};

struct Transform;

Matrix4x4 BuildWorldMatrix(const Transform& transform);
Float3 TransformPoint(const float point[3], const Matrix4x4& matrix);
bool IsPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy);

} // namespace BA
