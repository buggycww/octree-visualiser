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

	// vertical movement is swept in tiny steps to revent imprecise falling logic
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

	// apply velocity
	verticalVelocity = std::max(verticalVelocity - gravity * deltaTime, -terminalVelocity);
	const bool verticalMoveFree = MoveWithCollision(
		vec3::Vector3{ 0.f, verticalVelocity * deltaTime, 0.f }, *collision, verticalStep);

	// downward move that is blocked means the player is standing on something
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

void Player::Input(const Vector3 forward, const Vector3 right)
{
	// PLAYER MOVEMENT ================================================
	const float forwardInput = float(IsKeyDown(KEY_W)) - float(IsKeyDown(KEY_S));
	const float strafeInput = float(IsKeyDown(KEY_D)) - float(IsKeyDown(KEY_A));

	vec3::Vector3 move = { 0.f, 0.f, 0.f };

	if (forwardInput != 0.f || strafeInput != 0.f)
	{
		move.x = forward.x * forwardInput + right.x * strafeInput;
		move.z = forward.z * forwardInput + right.z * strafeInput;

		// Normalise, so W+D is not ~1.41x faster than W alone.
		const float length = std::sqrt(move.x * move.x + move.z * move.z);
		move.x /= length;
		move.z /= length;
	}

	direction = move;

	// JUMP ===========================================================
	if (IsKeyPressed(KEY_SPACE))
	{
		Jump();
	}
}

void Player::Jump()
{
	bJumpRequested = true;
}

bool Player::MoveWithCollision(const vec3::Vector3& delta, const CollisionWorld& world, float maxStep)
{
	const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	if (length <= 0.f) return true;

	// Sub-step long frames so a fast player cannot tunnel through an obstacle.
	const int steps = std::max(1, static_cast<int>(std::ceil(length / maxStep)));
	const vec3::Vector3 step = delta * (1.f / static_cast<float>(steps));

	bool blocked = false;

	const auto moveAxis = [&](int axis, float amount)
	{
		if (MoveAxis(axis, amount, world)) return;

		blocked = true;

		// The full step was blocked. Binary-search the furthest legal fraction
		// so the player comes to rest exactly against the surface instead of
		// stopping up to a whole step away from it.
		float lo = 0.f;
		float hi = amount;
		for (int i = 0; i < 12; ++i)
		{
			const float mid = (lo + hi) * 0.5f;
			if (CanMoveAxis(axis, mid, world)) lo = mid;
			else hi = mid;
		}

		if (lo != 0.f) MoveAxis(axis, lo, world);
	};

	for (int i = 0; i < steps; ++i)
	{
		// Resolve one axis at a time so hitting a wall still allows sliding along it (doesn't block other axes).
		moveAxis(0, step.x);
		moveAxis(2, step.z);
		moveAxis(1, step.y);
	}

	return !blocked;
}

bool Player::CanMoveAxis(int axis, float amount, const CollisionWorld& world) const
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
	return !world.overlaps(bounds);
}

bool Player::MoveAxis(int axis, float amount, const CollisionWorld& world)
{
	if (amount == 0.f) return true;
	if (!CanMoveAxis(axis, amount, world)) return false;

	switch (axis)
	{
	case 0: transform.position.x += amount; break;
	case 1: transform.position.y += amount; break;
	default: transform.position.z += amount; break;
	}

	return true;
}
