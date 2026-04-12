#include "Core/PCH.h"
#include "Math/Float3.h"

namespace BA
{

Float2::Float2(float s) : x(s), y(s) {}
Float2::Float2(float x, float y) : x(x), y(y) {}

Float2& Float2::operator+=(const Float2& rhs) { x += rhs.x; y += rhs.y; return *this; }
Float2& Float2::operator-=(const Float2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
Float2& Float2::operator*=(const Float2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
Float2& Float2::operator/=(const Float2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
Float2& Float2::operator+=(float s) { x += s; y += s; return *this; }
Float2& Float2::operator-=(float s) { x -= s; y -= s; return *this; }
Float2& Float2::operator*=(float s) { x *= s; y *= s; return *this; }
Float2& Float2::operator/=(float s) { x /= s; y /= s; return *this; }

bool Float2::operator==(const Float2& rhs) const { return x == rhs.x && y == rhs.y; }
bool Float2::operator!=(const Float2& rhs) const { return !(*this == rhs); }

Float3::Float3(float s) : x(s), y(s), z(s) {}
Float3::Float3(float x, float y, float z) : x(x), y(y), z(z) {}
Float3::Float3(const Float2& xy, float z) : x(xy.x), y(xy.y), z(z) {}

Float3& Float3::operator+=(const Float3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
Float3& Float3::operator-=(const Float3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
Float3& Float3::operator*=(const Float3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
Float3& Float3::operator/=(const Float3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
Float3& Float3::operator+=(float s) { x += s; y += s; z += s; return *this; }
Float3& Float3::operator-=(float s) { x -= s; y -= s; z -= s; return *this; }
Float3& Float3::operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
Float3& Float3::operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

bool Float3::operator==(const Float3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool Float3::operator!=(const Float3& rhs) const { return !(*this == rhs); }

Float4::Float4(float s) : x(s), y(s), z(s), w(s) {}
Float4::Float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
Float4::Float4(const Float3& xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

Float4& Float4::operator+=(const Float4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
Float4& Float4::operator-=(const Float4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
Float4& Float4::operator*=(const Float4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
Float4& Float4::operator/=(const Float4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
Float4& Float4::operator+=(float s) { x += s; y += s; z += s; w += s; return *this; }
Float4& Float4::operator-=(float s) { x -= s; y -= s; z -= s; w -= s; return *this; }
Float4& Float4::operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
Float4& Float4::operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

bool Float4::operator==(const Float4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
bool Float4::operator!=(const Float4& rhs) const { return !(*this == rhs); }

Float2 operator+(const Float2& a, const Float2& b) { return {a.x + b.x, a.y + b.y}; }
Float2 operator-(const Float2& a, const Float2& b) { return {a.x - b.x, a.y - b.y}; }
Float2 operator*(const Float2& a, const Float2& b) { return {a.x * b.x, a.y * b.y}; }
Float2 operator/(const Float2& a, const Float2& b) { return {a.x / b.x, a.y / b.y}; }
Float2 operator+(const Float2& v, float s) { return {v.x + s, v.y + s}; }
Float2 operator-(const Float2& v, float s) { return {v.x - s, v.y - s}; }
Float2 operator*(const Float2& v, float s) { return {v.x * s, v.y * s}; }
Float2 operator/(const Float2& v, float s) { return {v.x / s, v.y / s}; }
Float2 operator+(float s, const Float2& v) { return {s + v.x, s + v.y}; }
Float2 operator-(float s, const Float2& v) { return {s - v.x, s - v.y}; }
Float2 operator*(float s, const Float2& v) { return {s * v.x, s * v.y}; }
Float2 operator/(float s, const Float2& v) { return {s / v.x, s / v.y}; }
Float2 operator-(const Float2& v) { return {-v.x, -v.y}; }

Float3 operator+(const Float3& a, const Float3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Float3 operator-(const Float3& a, const Float3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Float3 operator*(const Float3& a, const Float3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
Float3 operator/(const Float3& a, const Float3& b) { return {a.x / b.x, a.y / b.y, a.z / b.z}; }
Float3 operator+(const Float3& v, float s) { return {v.x + s, v.y + s, v.z + s}; }
Float3 operator-(const Float3& v, float s) { return {v.x - s, v.y - s, v.z - s}; }
Float3 operator*(const Float3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
Float3 operator/(const Float3& v, float s) { return {v.x / s, v.y / s, v.z / s}; }
Float3 operator+(float s, const Float3& v) { return {s + v.x, s + v.y, s + v.z}; }
Float3 operator-(float s, const Float3& v) { return {s - v.x, s - v.y, s - v.z}; }
Float3 operator*(float s, const Float3& v) { return {s * v.x, s * v.y, s * v.z}; }
Float3 operator/(float s, const Float3& v) { return {s / v.x, s / v.y, s / v.z}; }
Float3 operator-(const Float3& v) { return {-v.x, -v.y, -v.z}; }

Float4 operator+(const Float4& a, const Float4& b) { return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }
Float4 operator-(const Float4& a, const Float4& b) { return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }
Float4 operator*(const Float4& a, const Float4& b) { return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w}; }
Float4 operator/(const Float4& a, const Float4& b) { return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w}; }
Float4 operator+(const Float4& v, float s) { return {v.x + s, v.y + s, v.z + s, v.w + s}; }
Float4 operator-(const Float4& v, float s) { return {v.x - s, v.y - s, v.z - s, v.w - s}; }
Float4 operator*(const Float4& v, float s) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }
Float4 operator/(const Float4& v, float s) { return {v.x / s, v.y / s, v.z / s, v.w / s}; }
Float4 operator+(float s, const Float4& v) { return {s + v.x, s + v.y, s + v.z, s + v.w}; }
Float4 operator-(float s, const Float4& v) { return {s - v.x, s - v.y, s - v.z, s - v.w}; }
Float4 operator*(float s, const Float4& v) { return {s * v.x, s * v.y, s * v.z, s * v.w}; }
Float4 operator/(float s, const Float4& v) { return {s / v.x, s / v.y, s / v.z, s / v.w}; }
Float4 operator-(const Float4& v) { return {-v.x, -v.y, -v.z, -v.w}; }

float Dot(const Float2& a, const Float2& b) { return a.x * b.x + a.y * b.y; }
float Dot(const Float3& a, const Float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float Dot(const Float4& a, const Float4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

Float3 Cross(const Float3& a, const Float3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float LengthSquared(const Float2& v) { return Dot(v, v); }
float LengthSquared(const Float3& v) { return Dot(v, v); }
float LengthSquared(const Float4& v) { return Dot(v, v); }

float Length(const Float2& v) { return sqrtf(LengthSquared(v)); }
float Length(const Float3& v) { return sqrtf(LengthSquared(v)); }
float Length(const Float4& v) { return sqrtf(LengthSquared(v)); }

Float2 Normalize(const Float2& v)
{
    float len = Length(v);
    BA_ASSERT(len > 0.0f);
    return v / len;
}

Float3 Normalize(const Float3& v)
{
    float len = Length(v);
    BA_ASSERT(len > 0.0f);
    return v / len;
}

Float4 Normalize(const Float4& v)
{
    float len = Length(v);
    BA_ASSERT(len > 0.0f);
    return v / len;
}

} // namespace BA
