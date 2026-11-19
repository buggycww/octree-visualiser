#include "game.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>

#include "constants/colors.h"

static void DrawDebugBackdrop(const char* line1, int size1,
    const char* line2, int size2,
    int x, int y, int lineGap)
{
    constexpr int pad = 8;

    const int w1 = MeasureText(line1, size1);
    const int w2 = MeasureText(line2, size2);

    const int line1Bottom = y + size1;
    const int line2Top = line1Bottom + lineGap;
    const int line2Bottom = line2Top + size2;

    const Rectangle panel{
        static_cast<float>(x - pad),
        static_cast<float>(y - pad),
        static_cast<float>(std::max(w1, w2) + pad * 2),
        static_cast<float>(line2Bottom - y + pad * 2)
    };

    DrawRectangleRounded(panel, 0.15f, 8, Fade(BLACK, 0.40f));
    DrawRectangleRoundedLinesEx(panel, 0.15f, 8, 1.f, Fade(RAYWHITE, 0.20f));
}

Game::Game() : resources{}, grid(vec3::Vector3(14, 1, 14), 1.f, 0.05f, resources), player(vec3::Vector3(1.5f, 1.275f, 1.5f), resources)
{
    assert(IsWindowReady() && "Game must be constructed after InitWindow()");

    // SETUP CAMERA ==================================================
    camera.up = Vector3(0, 1.f, 0);
    camera.fovy = 45.f;
    camera.projection = CAMERA_PERSPECTIVE;

    yaw = 0.f;
    pitch = -45.f * DEG2RAD;
    cameraDistance = 17.f;

    // Static level geometry (floor and obstacles) never moves, so it is
    // indexed once here. The player queries this tree for collision.
    collision.build(grid.GetColliders());
    player.SetCollisionWorld(&collision);
    player.SetCamera(&camera);
    visualiser.SetWorld(&collision);

    UpdateCamera();
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Update(dt);

        BeginDrawing();
        ClearBackground(santasGrey);
        BeginMode3D(camera);
        Draw();
        EndMode3D();
        DrawVisualiserHud();
        EndDrawing();
    }
}

void Game::Update(float dt)
{
    Input(dt);

    player.Update(dt);
    UpdateCamera();

    visualiser.Update(player.GetFootPosition());
}

void Game::Input(float dt)
{
    // CAMERA MOVEMENT ================================================
    // The camera orbits the player. Turning first means movement below uses
    // the direction the camera faces *now*.
    const float turnInput = float(IsKeyDown(KEY_PAGE_DOWN)) - float(IsKeyDown(KEY_PAGE_UP));
    yaw += turnInput * cameraTurnSpeed * DEG2RAD * dt;

    // Q/E tilt the orbit up/down, clamped so the camera never flips over or
    // sinks through the floor.
    const float pitchInput = float(IsKeyDown(KEY_E)) - float(IsKeyDown(KEY_Q));
    pitch = std::clamp(pitch + pitchInput * cameraTurnSpeed * DEG2RAD * dt,
                       -85.f * DEG2RAD, 10.f * DEG2RAD);

    // CAMERA ZOOM ====================================================
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.f)
    {
        cameraDistance = std::clamp(cameraDistance - wheel * cameraZoomSpeed, 5.f, 25.f);
    }

    // PLAYER MOVEMENT =================================================
    player.Input(Forward(), Right());
}

void Game::UpdateCamera()
{
    const Vector3 target = {
        player.transform.position.x,
        player.transform.position.y + 0.7f,
        player.transform.position.z
    };

    const Vector3 forward = Forward();

    camera.position = Vector3{
        target.x - forward.x * cameraDistance,
        target.y - forward.y * cameraDistance,
        target.z - forward.z * cameraDistance
    };

    // Keep the orbit camera above the floor even at steep pitch angles.
    camera.position.y = std::max(camera.position.y, 0.6f);
    camera.target = target;
}

void Game::Draw()
{
    DrawGrid(15, 1);
    grid.Draw();
    visualiser.Draw();
    player.Draw();
}

void Game::DrawVisualiserHud()
{
    if (!visualiser.HasCell()) return;

    const AABB bounds = visualiser.GetCellBounds();
    const int surfaces = visualiser.GetSurfaceCount();
    const int obstacles = visualiser.GetObstacleCount();
    const int entries = visualiser.GetEntryCount();
    const int ancestorCount = visualiser.GetAncestorCount();

    char parentText[96] = "root";
    if (ancestorCount > 0)
    {
        int written = std::snprintf(parentText, sizeof(parentText), "parents d%d",
                                    visualiser.GetAncestorDepth(ancestorCount - 1));

        for (int i = ancestorCount - 2; i >= 0; --i)
        {
            if (written < 0 || written >= static_cast<int>(sizeof(parentText))) break;
            written += std::snprintf(parentText + written,
                                     sizeof(parentText) - static_cast<std::size_t>(written),
                                     ", d%d", visualiser.GetAncestorDepth(i));
        }
    }

    char text[256];
    std::snprintf(text, sizeof(text),
        "octree cell  |  depth %d (%s)  |  %.2f x %.2f x %.2f  |  %d stored  |  %d %s  |  %d %s",
        visualiser.GetDepth(), parentText,
        bounds.w, bounds.h, bounds.d,
        entries,
        surfaces, surfaces == 1 ? "surface" : "surfaces",
        obstacles, obstacles == 1 ? "obstacle" : "obstacles");

    constexpr const char* kControls =
        "WASD move   |   SPACE jump   |   wheel zoom   |   Q/E tilt   |   PgUp/PgDn orbit";
    constexpr int kX = 24, kY = 24, kGap = 8;

    DrawDebugBackdrop(text, 20, kControls, 18, kX, kY, kGap);

    DrawText(text, kX, kY, 20, RAYWHITE);   // was BLACK
    DrawText(kControls, kX, kY + 20 + kGap, 18, LIGHTGRAY);
}

Vector3 Game::Forward() const
{
    const float cp = std::cos(pitch);
    return Vector3{ std::sin(yaw) * cp, std::sin(pitch), -std::cos(yaw) * cp };
}

Vector3 Game::HorizontalForward() const
{
    return Vector3{ std::sin(yaw), 0.f, -std::cos(yaw) };
}

Vector3 Game::Right() const
{
    return Vector3{ std::cos(yaw), 0.f, std::sin(yaw) };
}

void RunGame()
{
    Game game;
    game.Run();
}