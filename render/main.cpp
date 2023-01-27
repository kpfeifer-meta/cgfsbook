#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <SDL_test_common.h>
#include <SDL_main.h>


#define MAX_SPHERES 100

typedef struct
{
    float x, y, z;
} Vector3;

typedef struct
{
    float radius;
    Vector3 center;
} Sphere;


static SDLTest_CommonState *gState;
static SDL_Renderer *gRenderer = NULL;
SDL_Surface *screenSurface = NULL;

static int CANVAS_WIDTH = 1920;
static int CANVAS_HEIGHT = 1080;
static int VIEWPORT_WIDTH = 1;
static int VIEWPORT_HEIGHT = 1;
static float VIEWPORT_DIST = 1;

std::vector<Sphere *> spheres;
void PutPixel(int x, int y, int r, int g, int b)
{
    int sx = CANVAS_WIDTH / 2 + x;
    int sy = CANVAS_HEIGHT / 2 - y;

    SDL_SetRenderDrawColor(gRenderer, r, g, b, 0xff);
    SDL_RenderDrawPoint(gRenderer, sx, sy);
}

SDL_bool IntersectRaySphere(Vector3 &O, Vector3 &D, Sphere &sphere)
{
    return SDL_TRUE;
}

void TraceRay(Vector3 O, Vector3 D, float t_min, float t_max)
{
    float closest_t = -1;
    Sphere *closestSphere = NULL;

    for (int i = 0; i < spheres.size(); i++) {
        Sphere *s = spheres[i];
        if (s != NULL) {
        }
    }
}

Vector3 CanvasToViewport(float x, float y)
{
    Vector3 v;
    v.x = x * (float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH;
    v.y = y * (float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT;
    v.z = VIEWPORT_DIST;
    return v;
}

void DoSpiral()
{
    SDL_Event e;
    bool quit = false;

    float angle = 0;
    float radius = 1;
    while (quit == false) {
        float cy = radius * sin(angle);
        float cx = radius * cos(angle);

        int r = rand() & 1 ? 0xff : 0;
        int g = 0;
        int b = 0;

        //        SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, r, g, b));
        PutPixel(cx, cy, r, g, b);
        angle += 3.14159f * .005f;
        radius += .1f;
        SDL_RenderPresent(gRenderer);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
        }
    }
}

int main(int argc, char *argv[])
{

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetRenderDrawColor(gRenderer, 0x0A, 0x0A, 0x0A, 0xFF);
    SDL_RenderClear(gRenderer);

    screenSurface = SDL_GetWindowSurface(window); //Fill the surface white

    if (argc == 1 || !stricmp(argv[0] ,"spiral"))
        DoSpiral();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

