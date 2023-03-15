#pragma once

#include "Color.h"
#include "Vector.h"

class Renderer
{
  public:
    Renderer(int w, int h) {}

    virtual void DrawPixel(int x, int y, Color color) = 0;
    virtual void DrawLine(const Vector3& p0, const Vector3& p1, const Color& color) = 0
};
