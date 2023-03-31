#pragma once

#include <math.h>

class Vector3
{
  public:
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector3() : x(0), y(0), z(0) {}

    Vector3 operator-(const Vector3 &v) const
    {
        return { x - v.x, y - v.y, z - v.z };
    }

    Vector3 &operator-=(const Vector3 &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3 operator+(const Vector3 &v) const
    {
        return { x + v.x, y + v.y, z + v.z };
    }

    Vector3 &operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3 operator*(float f) const
    {
        return { f * x, f * y, f * z };
    }

    Vector3 &operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    float Dot(const Vector3 &v) const
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    float Length() const
    {
        return sqrtf(x * x + y * y + z * z);
    }

    float x, y, z;
};

class Vector2
{
  public:
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    Vector2() : x(0), y(0) {}

    Vector2 operator-(const Vector2 &v) const
    {
        return { x - v.x, y - v.y };
    }

    Vector2 &operator-=(const Vector2 &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2 operator+(const Vector2 &v) const
    {
        return { x + v.x, y + v.y};
    }

    Vector2 &operator+=(const Vector2 &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2 operator*(float f) const
    {
        return { f * x, f * y};
    }

    Vector2 &operator*=(float f)
    {
        x *= f;
        y *= f;
        return *this;
    }

    float Dot(const Vector2 &v) const
    {
        return (x * v.x) + (y * v.y);
    }

    float Length() const
    {
        return sqrtf(x * x + y * y);
    }

    float x, y;
};
