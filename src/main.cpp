#include <raylib.h>

#include "game/game.h"

int main()
{
    constexpr int screenWidth = 1920;
    constexpr int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "octree-visualiser");
    SetTargetFPS(60);

    RunGame();

    CloseWindow();
}
