#include "game.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

#include "constants/colors.h"

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
        ClearBackground(periwinkleGray_02);
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

    // PLAYER MOVEMENT ================================================
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

    char text[256];

    if (visualiser.HasParent())
    {
        std::snprintf(text, sizeof(text),
                      "octree cell  |  depth %d (parent %d)  |  %.2f x %.2f x %.2f  |  %d stored  |  %d %s  |  %d %s",
                      visualiser.GetDepth(), visualiser.GetParentDepth(),
                      bounds.w, bounds.h, bounds.d,
                      entries,
                      surfaces, surfaces == 1 ? "surface" : "surfaces",
                      obstacles, obstacles == 1 ? "obstacle" : "obstacles");
    }
    else
    {
        std::snprintf(text, sizeof(text),
                      "octree cell  |  depth %d (root)  |  %.2f x %.2f x %.2f  |  %d stored  |  %d %s  |  %d %s",
                      visualiser.GetDepth(),
                      bounds.w, bounds.h, bounds.d,
                      entries,
                      surfaces, surfaces == 1 ? "surface" : "surfaces",
                      obstacles, obstacles == 1 ? "obstacle" : "obstacles");
    }

    DrawText(text, 24, 24, 20, rum);

    DrawText("WASD move   |   SPACE jump   |   wheel zoom   |   Q/E tilt   |   PgUp/PgDn orbit",
             24, 52, 18, Fade(rum, 0.75f));
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
