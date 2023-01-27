#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <SDL_test_common.h>
#include <SDL_main.h>


#define MAX_SPHERES 100

class Vector3
{
public:
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)    {}
  Vector3() : x(0), y(0), z(0){}

    Vector3 operator-(const Vector3& v) const
    {
        return { x - v.x, y - v.y, z - v.z };
    }

    Vector3& operator-=(const Vector3& v)
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

    float Dot(const Vector3 &v) const
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    float x, y, z;
};

class Color
{
  public:
    Color() : r(0), g(0), b(0), a(0) {}
    Color(int red, int green, int blue) : Color(red, green, blue, 255) {}
    Color(int red, int green, int blue, int alpha) : r(red), g(green), b(blue), a(alpha) {}

    int r, g, b, a;
};

class Sphere
{
public:
    Sphere() : radius(0), center(0,0,0), color(0,0,0,0){}
    Sphere(Vector3 ctr, float rad, Color clr) : radius(rad), center(ctr), color(clr) {}

    float radius;
    Vector3 center;
    Color color;
};

static SDLTest_CommonState *gState;
static SDL_Renderer *gRenderer = nullptr;
SDL_Surface *screenSurface = nullptr;

static int CANVAS_WIDTH = 1080;
static int CANVAS_HEIGHT = 1080;
static int VIEWPORT_WIDTH = 1;
static int VIEWPORT_HEIGHT = 1;
static float VIEWPORT_DIST = 1;
static Color BACKGROUND = Color(255, 255, 255, 255);

std::vector<Sphere> spheres;

void PutPixel(int x, int y, int r, int g, int b)
{
    int sx = CANVAS_WIDTH / 2 + x;
    int sy = CANVAS_HEIGHT / 2 - y;

    SDL_SetRenderDrawColor(gRenderer, r, g, b, 0xff);
    SDL_RenderDrawPoint(gRenderer, sx, sy);
}

void PutPixel(int x, int y, Color color)
{
    PutPixel(x, y, color.r, color.g, color.b);
}

SDL_bool IntersectRaySphere(Vector3 &O, Vector3 &D, const Sphere &sphere, float &t1, float &t2)
{
    float r = sphere.radius;
    Vector3 CO = O - sphere.center;

    float a = D.Dot(D);
    float b = 2 * CO.Dot(D);
    float c = CO.Dot(CO) - r * r;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        t1 = t2 = -1;
        return SDL_FALSE;
    }

    t1 = (-b + (float)sqrt(discriminant)) / (2 * a);
    t2 = (-b - (float)sqrt(discriminant)) / (2 * a);

    return SDL_TRUE;
}

void TraceRay(Vector3 O, Vector3 D, float t_min, float t_max, Color& oColor)
{
    float closest_t = 100000000.f;
    int closestSphereIndex = -1;

    int sphereIndex = 0;
    for (auto s : spheres) {
        float t1, t2;
        if (IntersectRaySphere(O, D, s, t1, t2)) {
            if (t1 >= t_min && t1 <= t_max && t1 < closest_t) {
                closest_t = t1;
                closestSphereIndex = sphereIndex;
            }
            if (t2 >= t_min && t2 <= t_max && t2 < closest_t) {
                closest_t = t2;
                closestSphereIndex = sphereIndex;
            }
        }
        sphereIndex++;
    }

    if (closestSphereIndex == -1)
        oColor = BACKGROUND;
    else
        oColor = spheres[closestSphereIndex].color;
}

Vector3 CanvasToViewport(float canvasX, float canvasY)
{
    Vector3 v;
    v.x = canvasX * (float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH;
    v.y = canvasY * (float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT;
    v.z = VIEWPORT_DIST;
    return v;
}

void DoSpheres()
{
    spheres.clear();
    spheres.emplace_back(Vector3(0.f, -1.f, 3.f), 1.f, Color(255, 0, 0));
    spheres.emplace_back(Vector3(2.f, 0.f, 4.f), 1.f, Color(0, 0, 255));
    spheres.emplace_back(Vector3(-2.f, 0.f, 4.f), 1.f, Color(0, 255, 0));

    Vector3 O(0, 0, 0);
    Color color;
    for (int x = -CANVAS_WIDTH / 2; x <= CANVAS_WIDTH / 2; x++) {
        for (int y=-CANVAS_HEIGHT/2; y<=CANVAS_HEIGHT/2; y++) {
            Vector3 D = CanvasToViewport((float)x, (float)y);
            TraceRay(O, D, 1, 1000000.f, color);
            PutPixel(x, y, color);
        }
    }
    SDL_RenderPresent(gRenderer);
}


void DoSpiral()
{
    float angle = 0;
    float radius = 1;
    float cy = radius * (float)sin(angle);
    float cx = radius * (float)cos(angle);

    int r = rand() & 1 ? 0xff : 0;
    int g = 0;
    int b = 0;

    //        SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, r, g, b));
    PutPixel((int)cx, (int)cy, r, g, b);
    angle += 3.14159f * .005f;
    radius += .1f;
    SDL_RenderPresent(gRenderer);
}

int main(int argc, char *argv[])
{
    SDL_Event e;
    bool quit = false;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetRenderDrawColor(gRenderer, 0x0A, 0x0A, 0x0A, 0xFF);
    SDL_RenderClear(gRenderer);

    screenSurface = SDL_GetWindowSurface(window); //Fill the surface white

    while (quit == false) {
        if (argc > 1 && !_stricmp(argv[1], "spiral"))
            DoSpiral();
        else
            DoSpheres();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

