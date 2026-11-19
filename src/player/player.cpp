#include "player.h"

#include "collision detection/AABB.h"
#include "collision detection/CollisionWorld.h"
#include "surface/grid.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float gravity = 22.f;          // units per second squared
	constexpr float jumpSpeed = 8.f;         // initial upward speed
	constexpr float terminalVelocity = 25.f; // keeps falls tunnelling-free

	// Horizontal movement is swept in fairly large steps; vertical movement is
	// swept in fine ones. A coarse vertical step can stop a fall up to a full
	// step above a surface and then keep failing to close the remaining gap,
	// which leaves the player hovering and unable to jump.
	constexpr float horizontalStep = 0.2f;
	constexpr float verticalStep = 0.01f;

	AABB BoundsAt(const vec3::Vector3& position,
				  const vec3::Vector3& size,
				  const vec3::Vector3& offset)
	{
		return {
			position.x + offset.x - size.x * 0.5f,
			position.y + offset.y - size.y * 0.5f,
			position.z + offset.z - size.z * 0.5f,
			size.x,
			size.y,
			size.z
		};
	}
}

void Player::Jump()
{
	bJumpRequested = true;
}

void Player::Update(float deltaTime)
{
	if (collision == nullptr)
	{
		GameObject::Update(deltaTime);
		return;
	}

	const bool hasInput = direction.x != 0.f || direction.y != 0.f || direction.z != 0.f;
	if (hasInput)
	{
		MoveWithCollision(direction * speed * deltaTime, *collision, horizontalStep);
	}

	// Gravity always runs, so walking off a ledge drops the player onto whatever
	// is below and a jump is just a temporary upward velocity.
	verticalVelocity = std::max(verticalVelocity - gravity * deltaTime, -terminalVelocity);
	const bool verticalMoveFree = MoveWithCollision(
		vec3::Vector3{ 0.f, verticalVelocity * deltaTime, 0.f }, *collision, verticalStep);

	// A downward move that is blocked means the player is standing on something.
	// This is more reliable than probing below the feet: the probe could miss a
	// narrow ledge or graze a wall the player is next to.
	bIsGrounded = verticalVelocity <= 0.f && !verticalMoveFree;
	if (bIsGrounded) verticalVelocity = 0.f;

	// The request is consumed after the ground check, so pressing jump on the
	// frame the player lands still works.
	if (bJumpRequested)
	{
		bJumpRequested = false;
		if (bIsGrounded)
		{
			verticalVelocity = jumpSpeed;
			bIsGrounded = false;
		}
	}
}

void Player::Draw() const
{
	GameObject::Draw();
}

bool Player::MoveWithCollision(const vec3::Vector3& delta, const CollisionWorld& world, float maxStep)
{
	const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	if (length <= 0.f) return true;

	// Sub-step long frames so a fast player cannot tunnel through an obstacle.
	const int steps = std::max(1, static_cast<int>(std::ceil(length / maxStep)));
	const vec3::Vector3 step = delta * (1.f / static_cast<float>(steps));

	bool blocked = false;

	for (int i = 0; i < steps; ++i)
	{
		// Resolve one axis at a time so hitting a wall still allows sliding
		// along it.
		if (!MoveAxis(0, step.x, world)) blocked = true;
		if (!MoveAxis(2, step.z, world)) blocked = true;
		if (!MoveAxis(1, step.y, world)) blocked = true;
	}

	return !blocked;
}

bool Player::MoveAxis(int axis, float amount, const CollisionWorld& world)
{
	if (amount == 0.f) return true;

	vec3::Vector3 next = transform.position;

	switch (axis)
	{
	case 0: next.x += amount; break;
	case 1: next.y += amount; break;
	default: next.z += amount; break;
	}

	const AABB bounds = BoundsAt(next, colliderSize, colliderOffset);
	if (world.overlaps(bounds)) return false;

	transform.position = next;
	return true;
}
