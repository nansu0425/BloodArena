#pragma once

namespace BA
{

struct Float2
{
    float x = 0.0f;
    float y = 0.0f;

    Float2() = default;
    explicit Float2(float s);
    Float2(float x, float y);

    Float2& operator+=(const Float2& rhs);
    Float2& operator-=(const Float2& rhs);
    Float2& operator*=(const Float2& rhs);
    Float2& operator/=(const Float2& rhs);
    Float2& operator+=(float s);
    Float2& operator-=(float s);
    Float2& operator*=(float s);
    Float2& operator/=(float s);

    bool operator==(const Float2& rhs) const;
    bool operator!=(const Float2& rhs) const;
};

struct Float3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Float3() = default;
    explicit Float3(float s);
    Float3(float x, float y, float z);
    Float3(const Float2& xy, float z);

    Float3& operator+=(const Float3& rhs);
    Float3& operator-=(const Float3& rhs);
    Float3& operator*=(const Float3& rhs);
    Float3& operator/=(const Float3& rhs);
    Float3& operator+=(float s);
    Float3& operator-=(float s);
    Float3& operator*=(float s);
    Float3& operator/=(float s);

    bool operator==(const Float3& rhs) const;
    bool operator!=(const Float3& rhs) const;
};

struct Float4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Float4() = default;
    explicit Float4(float s);
    Float4(float x, float y, float z, float w);
    Float4(const Float3& xyz, float w);

    Float4& operator+=(const Float4& rhs);
    Float4& operator-=(const Float4& rhs);
    Float4& operator*=(const Float4& rhs);
    Float4& operator/=(const Float4& rhs);
    Float4& operator+=(float s);
    Float4& operator-=(float s);
    Float4& operator*=(float s);
    Float4& operator/=(float s);

    bool operator==(const Float4& rhs) const;
    bool operator!=(const Float4& rhs) const;
};

Float2 operator+(const Float2& a, const Float2& b);
Float2 operator-(const Float2& a, const Float2& b);
Float2 operator*(const Float2& a, const Float2& b);
Float2 operator/(const Float2& a, const Float2& b);
Float2 operator+(const Float2& v, float s);
Float2 operator-(const Float2& v, float s);
Float2 operator*(const Float2& v, float s);
Float2 operator/(const Float2& v, float s);
Float2 operator+(float s, const Float2& v);
Float2 operator-(float s, const Float2& v);
Float2 operator*(float s, const Float2& v);
Float2 operator/(float s, const Float2& v);
Float2 operator-(const Float2& v);

Float3 operator+(const Float3& a, const Float3& b);
Float3 operator-(const Float3& a, const Float3& b);
Float3 operator*(const Float3& a, const Float3& b);
Float3 operator/(const Float3& a, const Float3& b);
Float3 operator+(const Float3& v, float s);
Float3 operator-(const Float3& v, float s);
Float3 operator*(const Float3& v, float s);
Float3 operator/(const Float3& v, float s);
Float3 operator+(float s, const Float3& v);
Float3 operator-(float s, const Float3& v);
Float3 operator*(float s, const Float3& v);
Float3 operator/(float s, const Float3& v);
Float3 operator-(const Float3& v);

Float4 operator+(const Float4& a, const Float4& b);
Float4 operator-(const Float4& a, const Float4& b);
Float4 operator*(const Float4& a, const Float4& b);
Float4 operator/(const Float4& a, const Float4& b);
Float4 operator+(const Float4& v, float s);
Float4 operator-(const Float4& v, float s);
Float4 operator*(const Float4& v, float s);
Float4 operator/(const Float4& v, float s);
Float4 operator+(float s, const Float4& v);
Float4 operator-(float s, const Float4& v);
Float4 operator*(float s, const Float4& v);
Float4 operator/(float s, const Float4& v);
Float4 operator-(const Float4& v);

float Dot(const Float2& a, const Float2& b);
float Dot(const Float3& a, const Float3& b);
float Dot(const Float4& a, const Float4& b);

Float3 Cross(const Float3& a, const Float3& b);

float LengthSquared(const Float2& v);
float LengthSquared(const Float3& v);
float LengthSquared(const Float4& v);

float Length(const Float2& v);
float Length(const Float3& v);
float Length(const Float4& v);

Float2 Normalize(const Float2& v);
Float3 Normalize(const Float3& v);
Float4 Normalize(const Float4& v);

} // namespace BA
