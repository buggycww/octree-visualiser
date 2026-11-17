#include <raylib.h>

#include "game/game.h"

int main()
{
    constexpr int screenWidth = 1920;
    constexpr int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Recreating DOS2 Surface System");
    SetTargetFPS(60);

    RunGame();

    CloseWindow();
}
