#pragma once
class Color
{
  public:
    Color() : r(0), g(0), b(0), a(0) {}
    Color(int red, int green, int blue) : Color(red, green, blue, 255) {}
    Color(int red, int green, int blue, int alpha) : r(red), g(green), b(blue), a(alpha) {}

    Color operator*(float f) const
    {
        return { static_cast<int>(f * static_cast<float>(r)), static_cast<int>(f * static_cast<float>(g)), static_cast<int>(f * static_cast<float>(b)), static_cast<int>(f * static_cast<float>(a)) };
    }

    Color operator+(Color color) const
    {
        return { r + color.r, g + color.g, b + color.b, a + color.a };
    }

    int r, g, b, a;
};
