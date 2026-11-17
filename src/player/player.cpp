#include "player.h"

#include "collision detection/AABB.h"
#include "collision detection/CollisionWorld.h"
#include "surface/grid.h"

#include <algorithm>
#include <cmath>

namespace
{
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
	if (collision != nullptr)
	{
		const bool hasInput = direction.x != 0.f || direction.y != 0.f || direction.z != 0.f;
		if (hasInput)
		{
			MoveWithCollision(direction * speed * deltaTime, *collision);
		}
	}
	else
	{
		GameObject::Update(deltaTime);
	}
}

void Player::Draw() const
{
	GameObject::Draw();
}

void Player::MoveWithCollision(const vec3::Vector3& delta, const CollisionWorld& world)
{
	const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	if (length <= 0.f) return;

	// Sub-step long frames so a fast player cannot tunnel through an obstacle.
	constexpr float maxStep = 0.25f;
	const int steps = std::max(1, static_cast<int>(std::ceil(length / maxStep)));
	const vec3::Vector3 step = delta * (1.f / static_cast<float>(steps));

	for (int i = 0; i < steps; ++i)
	{
		// Resolve one axis at a time so hitting a wall still allows sliding
		// along it.
		MoveAxis(0, step.x, world);
		MoveAxis(2, step.z, world);
		MoveAxis(1, step.y, world);
	}
}

void Player::MoveAxis(int axis, float amount, const CollisionWorld& world)
{
	if (amount == 0.f) return;

	vec3::Vector3 next = transform.position;

	switch (axis)
	{
	case 0: next.x += amount; break;
	case 1: next.y += amount; break;
	default: next.z += amount; break;
	}

	const AABB bounds = BoundsAt(next, colliderSize, colliderOffset);
	if (world.overlaps(bounds)) return;

	transform.position = next;
}
