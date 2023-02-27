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

class Sphere
{
public:
    Sphere() : radius(0), center(0,0,0), color(0,0,0,0), specular(-1), reflective(0) {}
    Sphere(Vector3 ctr, float rad, Color clr, float s=-1, float r=0) : radius(rad), center(ctr), color(clr), specular(s), reflective(r) {}

    float radius;
    Vector3 center;
    Color color;
    float specular;
    float reflective;
};

class Light
{
public:
    enum Type {ambient, point, directional};

    Light(Type t, float i, Vector3 pos, Vector3 dir) : type(t), intensity(i), position(pos), direction(dir) {}
    Type type;
    float intensity;
    Vector3 position;
    Vector3 direction;
};

static SDLTest_CommonState *gState;
static SDL_Renderer *gRenderer = nullptr;
SDL_Surface *screenSurface = nullptr;
SDL_Window *gWindow = nullptr;


static int CANVAS_WIDTH = 1080;
static int CANVAS_HEIGHT = 1080;
static int VIEWPORT_WIDTH = 1;
static int VIEWPORT_HEIGHT = 1;
static float VIEWPORT_DIST = 1;
static Color BACKGROUND = Color(255, 255, 255, 255);

std::vector<Sphere> spheres;
std::vector<Light> lights;

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

SDL_bool IntersectRaySphere(Vector3 &O, Vector3 &D, const Sphere *sphere, float &t1, float &t2)
{
    float r = sphere->radius;
    Vector3 CO = O - sphere->center;

    float a = D.Dot(D);
    float b = 2 * CO.Dot(D);
    float c = CO.Dot(CO) - r * r;

    const float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        t1 = t2 = -1;
        return SDL_FALSE;
    }

    float d = sqrtf(discriminant);
    t1 = (-b + d) / (2 * a);
    t2 = (-b - d) / (2 * a);

    return SDL_TRUE;
}

bool ClosestIntersection(Vector3 O, Vector3 D, float t_min, float t_max, Sphere **oSphere, float &oT)
{
    float closest_t = FLT_MAX;
    Sphere *closest_sphere = nullptr;
    for (unsigned i = 0; i < spheres.size(); i++) {
        float t1, t2;
        IntersectRaySphere(O, D, &spheres[i], t1, t2);
        if (t1 >= t_min && t1 <= t_max && t1 < closest_t) {
            closest_t = t1;
            closest_sphere = &spheres[i];
        }
        if (t2 >= t_min && t2 <= t_max && t2 < closest_t) {
            closest_t = t2;
            closest_sphere = &spheres[i];
        }
    }

    oT = closest_t;
    *oSphere = closest_sphere;
    return closest_sphere != nullptr;
}

float ComputeLighting(Vector3 P, Vector3 N, Vector3 V, float s = -1)
{
    float i = 0;
    Vector3 L;

    for (Light light : lights) {
        if (light.type == Light::Type::ambient)
            i += light.intensity;
        else
        {
            float t_max = 0;

            switch (light.type) {
            case Light::Type::point:
                L = light.position - P;
                t_max = 1;
                break;
            case Light::Type::directional:
                L = light.direction;
                t_max = FLT_MAX;
                break;
            default:
                break;
            }

            Sphere *shadow_sphere = nullptr;
            float shadow_t;
            if (ClosestIntersection(P, L, 0.001f, t_max, &shadow_sphere, shadow_t))
                continue;

            // diffuse
            const float n_dot_l = N.Dot(L);
            if (n_dot_l > 0) {
                i += light.intensity * n_dot_l / (N.Length() * L.Length());
            }

            if (s != -1) {
                Vector3 R = N * 2 * N.Dot(L) - L;
                const float r_dot_v = R.Dot(V);
                if (r_dot_v > 0) {
                    i += light.intensity * static_cast<float>(pow((r_dot_v / (R.Length() * V.Length())), s));
                }
            }
        }
    }

    return i;
}

Vector3 ReflectRay(Vector3 R, Vector3 N)
{
    return (N * 2.f) * N.Dot(R) - R;
}

Color TraceRay(Vector3 O, Vector3 D, float t_min, float t_max, int recursion_depth)
{
    float closest_t = 100000000.f;
    int closestSphereIndex = -1;
    Sphere *closestSphere = nullptr;

    const bool found = ClosestIntersection(O, D, t_min, t_max, &closestSphere, closest_t);
    if (!found)
        return BACKGROUND;
    else {
        Vector3 P = O + D * closest_t;
        Vector3 N = P - closestSphere->center;
        N = N * (1.f/N.Length());
        float l = ComputeLighting(P, N, D * -1, closestSphere->specular);
        l = SDL_clamp(l, 0, 1);
        Color color = closestSphere->color * l;
        float r = closestSphere->reflective;
        if (recursion_depth <= 0 || r <= 0)
            return color;

        Vector3 R = ReflectRay(D * -1.f, N);
        Color reflectedColor = TraceRay(P, R, 0.001f, FLT_MAX, recursion_depth - 1);
        return color * (1.f - r) + reflectedColor * r;
    }
}

Vector3 CanvasToViewport(float canvasX, float canvasY)
{
    Vector3 v;
    v.x = canvasX * static_cast<float>(VIEWPORT_WIDTH) / static_cast<float>(CANVAS_WIDTH);
    v.y = canvasY * static_cast<float>(VIEWPORT_HEIGHT) / static_cast<float>(CANVAS_HEIGHT);
    v.z = VIEWPORT_DIST;
    return v;
}

void CreateWindow()
{
    gWindow = SDL_CreateWindow("SDL demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_SHOWN);
    screenSurface = SDL_GetWindowSurface(gWindow); //Fill the surface white
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void DestroyWindow()
{
    SDL_DestroyWindow(gWindow);
}

bool CheckForEscape()
{
    SDL_Event e;
    SDL_PollEvent(&e);
    if (e.type == SDL_KEYUP) {
        if (e.key.keysym.sym == SDLK_ESCAPE)
            return true;
    }

    return false;
}


void WaitForEscape()
{
    while (!CheckForEscape()) {
    }
}

void DoSpheres()
{
    CreateWindow();

    spheres.clear();
    spheres.emplace_back(Sphere(Vector3(0.f, -1.f, 3.f), 1.f, Color(255, 0, 0), 500, 0.2f));
    spheres.emplace_back(Sphere(Vector3(2.f, 0.f, 4.f), 1.f, Color(0, 0, 255), 500, 0.3f));
    spheres.emplace_back(Sphere(Vector3(-2.f, 0.f, 4.f), 1.f, Color(0, 255, 0), 10, 0.4f));
    spheres.emplace_back(Sphere(Vector3(0, -5001, 0), 5000, Color(255, 255, 0), 1000, 0.5f));

    lights.emplace_back(Light(Light::ambient, 0.2f, Vector3(0, 0, 0), Vector3(0, 0, 0)));
    lights.emplace_back(Light(Light::point, 0.6f, Vector3(2, 1, 0), Vector3(0, 0, 0)));
    lights.emplace_back(Light(Light::directional, 0.2f, Vector3(0, 0, 0), Vector3(1, 4, 4)));


    Vector3 O(0, 0, 0);
    for (int x = -CANVAS_WIDTH / 2; x <= CANVAS_WIDTH / 2; x++) {
        for (int y=-CANVAS_HEIGHT/2; y<=CANVAS_HEIGHT/2; y++) {
            Vector3 D = CanvasToViewport(static_cast<float>(x), static_cast<float>(y));
            const Color color = TraceRay(O, D, 1, 1000000.f, 1);
            PutPixel(x, y, color);
        }
    }
    SDL_RenderPresent(gRenderer);

    WaitForEscape();

    DestroyWindow();
}


void DoSpiral()
{
    CreateWindow();
    float angle = 0;
    float radius = 1;

    while (radius < CANVAS_WIDTH/2) {
        float cy = radius * (float)sin(angle);
        float cx = radius * (float)cos(angle);
        PutPixel((int)cx, (int)cy, 0xff, 0, 0);
        angle += 3.14159f * .005f;
        radius += .1f;
        SDL_RenderPresent(gRenderer);
        if (CheckForEscape())
            break;
    }

    DestroyWindow();
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[])
{
    bool quit = false;
    bool doMenu = false;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetRenderDrawColor(gRenderer, 0x0A, 0x0A, 0x0A, 0xFF);
    SDL_RenderClear(gRenderer);

    if (doMenu)
    {
        while (quit == false) {
            printf("\n\n");
            printf("1 - Spheres\n");
            printf("2 - Spiral\n");
            printf("q - Quit\n");

            int ch = getc(stdin);
            if (ch == 'q') {
                quit = true;
            } else {
                switch (ch) {
                case '1':
                    DoSpheres();
                    break;
                case '2':
                    DoSpiral();
                    break;
                default:
                    break;
                }
            }
        }
    }
    else
        DoSpheres();

    SDL_Quit();

    return 0;
}

