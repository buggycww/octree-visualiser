#pragma once

#include <raylib.h>

#include "structs/resources.h"
#include "surface/grid.h"
#include "collision detection/CollisionWorld.h"
#include "player/player.h"
#include "structs/vec3.h"

class Game
{
public:
    Game();

    void Run();

private:
    void Update(float dt);
    void Input(float dt);
    void Draw();
    void UpdateCamera();

    // Camera basis, derived from yaw/pitch.
    Vector3 Forward() const;            // where the camera looks (includes pitch)
    Vector3 HorizontalForward() const;  // Forward() flattened onto the XZ plane
    Vector3 Right() const;              // strafe direction

    Camera3D camera;
    float yaw = 0.f;                    // radians, 0 = looking toward -Z
    float pitch = 0.f;                  // radians, + = looking up
    float cameraDistance = 14.f;        // orbit distance from the player
    float cameraTurnSpeed = 100.f;      // degrees per second
    float cameraZoomSpeed = 2.f;        // world units per wheel notch
    float playerMoveSpeed = 5.f;        // units per second

    Resources resources;
    Grid grid;
    CollisionWorld collision;
    Player player;
};

// Runs a single game session: builds the Game, plays until the window closes,
// and destroys it before returning. The Game object lives inside this function,
// so its resources are unloaded while the OpenGL context is still alive.
void RunGame();
