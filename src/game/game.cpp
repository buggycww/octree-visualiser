#include "game.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "constants/colors.h"

Game::Game() : resources{}, grid(vec3::Vector3(10, 1, 10), 1.f, 0.05f, resources), player(vec3::Vector3(0.f, 1.3f, 0.f), resources)
{
    assert(IsWindowReady() && "Game must be constructed after InitWindow()");

    // SETUP CAMERA ==================================================
    camera.up = Vector3(0, 1.f, 0);
    camera.fovy = 45.f;
    camera.projection = CAMERA_PERSPECTIVE;

    yaw = 0.f;
    pitch = -45.f * DEG2RAD;
    cameraDistance = 14.f;

    // Static level geometry (floor and obstacles) never moves, so it is
    // indexed once here. The player queries this tree for collision.
    collision.build(grid.GetColliders());
    player.SetCollisionWorld(&collision);
    player.SetCamera(&camera);

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
        EndDrawing();
    }
}

void Game::Update(float dt)
{
    Input(dt);

    player.Update(dt);

    // Follow after the player has moved, so the camera never lags a frame.
    UpdateCamera();
}

void Game::Input(float dt)
{
    // ROTATION =======================================================
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
    const float forwardInput = float(IsKeyDown(KEY_W)) - float(IsKeyDown(KEY_S));
    const float strafeInput = float(IsKeyDown(KEY_D)) - float(IsKeyDown(KEY_A));

    vec3::Vector3 move = { 0.f, 0.f, 0.f };

    if (forwardInput != 0.f || strafeInput != 0.f)
    {
        const Vector3 forward = HorizontalForward();
        const Vector3 right = Right();

        move.x = forward.x * forwardInput + right.x * strafeInput;
        move.z = forward.z * forwardInput + right.z * strafeInput;

        // Normalise, so W+D is not ~1.41x faster than W alone.
        const float length = std::sqrt(move.x * move.x + move.z * move.z);
        move.x /= length;
        move.z /= length;
    }

    player.direction = move;
    player.speed = playerMoveSpeed;

    // ZOOM ===========================================================
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.f)
    {
        cameraDistance = std::clamp(cameraDistance - wheel * cameraZoomSpeed, 5.f, 25.f);
    }
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
    player.Draw();
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
