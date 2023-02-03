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


    int r, g, b, a;
};

class Sphere
{
public:
    Sphere() : radius(0), center(0,0,0), color(0,0,0,0), specular(-1) {}
    Sphere(Vector3 ctr, float rad, Color clr, float s=-1) : radius(rad), center(ctr), color(clr), specular(s) {}

    float radius;
    Vector3 center;
    Color color;
    float specular;
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

float ComputeLighting(Vector3 P, Vector3 N, Vector3 V, float s=-1)
{
    float i = 0;
    Vector3 L;

    for (Light light : lights) {
        if (light.type == Light::Type::ambient)
            i += light.intensity;
        else
        {
            switch (light.type) {
            case Light::Type::point:
                L = light.position - P;
                break;
            case Light::Type::directional:
                L = light.direction;
                break;
            }

            float n_dot_l = N.Dot(L);
            if (n_dot_l > 0) {
                i += light.intensity * n_dot_l / (N.Length() * L.Length());
            }

            if (s != -1) {
                Vector3 R = N * 2 * N.Dot(L) - L;
                float r_dot_v = R.Dot(V);
                if (r_dot_v > 0) {
                    i += light.intensity * pow(r_dot_v / (R.Length() * V.Length()), s);
                }
            }
        }
    }

    return i;
}


SDL_bool IntersectRaySphere(Vector3 &O, Vector3 &D, const Sphere* sphere, float &t1, float &t2)
{
    float r = sphere->radius;
    Vector3 CO = O - sphere->center;

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
    for (int i = 0; i < spheres.size(); i++) {
        Sphere *s = &spheres[i];
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
    else {
        Sphere &s = spheres[closestSphereIndex];
        Vector3 P = O + D * closest_t;
        Vector3 N = P - s.center;
        N = N * (1.f/N.Length());
        float l = ComputeLighting(P, N, D * -1, s.specular);
        l = SDL_clamp(l, 0, 1);
        oColor = s.color * l;
    }
}

Vector3 CanvasToViewport(float canvasX, float canvasY)
{
    Vector3 v;
    v.x = canvasX * (float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH;
    v.y = canvasY * (float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT;
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
    spheres.emplace_back(Vector3(0.f, -1.f, 3.f), 1.f, Color(255, 0, 0), 500);
    spheres.emplace_back(Vector3(2.f, 0.f, 4.f), 1.f, Color(0, 0, 255), 500);
    spheres.emplace_back(Vector3(-2.f, 0.f, 4.f), 1.f, Color(0, 255, 0), 10);
    spheres.emplace_back(Vector3(0, -5001, 0), 5000, Color(255, 255, 0), 1000);

    lights.emplace_back(Light(Light::ambient, 0.2f, Vector3(0, 0, 0), Vector3(0, 0, 0)));
    lights.emplace_back(Light(Light::point, 0.6f, Vector3(2, 1, 0), Vector3(0, 0, 0)));
    lights.emplace_back(Light(Light::directional, 0.2f, Vector3(0, 0, 0), Vector3(1, 4, 4)));


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

int main(int argc, char *argv[])
{
    bool quit = false;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetRenderDrawColor(gRenderer, 0x0A, 0x0A, 0x0A, 0xFF);
    SDL_RenderClear(gRenderer);

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
            case SDLK_2:
                DoSpiral();
                break;
            default:
                break;
            }
        }
    }

    SDL_Quit();

    return 0;
}

