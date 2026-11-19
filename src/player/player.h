#pragma once

#include "structs/gameObjects.h"

class CollisionWorld;
struct Grid;

class Player : public GameObject
{
public:
	Player(vec3::Vector3 pos, const Resources& res)
	{
		transform.position = pos;
		speed = 5.f;

		colliderSize = { 0.6f, 1.3f, 0.6f };
		colliderOffset = { 0.f, 0.35f, 0.f };

		Transform3D bodyLocal;
		bodyLocal.position = { 0.f, -0.25f, 0.f };
		bodyLocal.scale = { .3f, .6f, .3f };

		ModelAttachment& body = Attach(res.cylinderModel.Get(), bodyLocal, DARKGRAY);
		body.outline = true;
		body.outlineColor = rum;

		Transform3D headLocal;
		headLocal.position = { 0.f, 0.6f, 0.f };
		headLocal.scale = { 0.2f, 0.2f, 0.2f };

		ModelAttachment& head = Attach(res.sphereModel.Get(), headLocal, DARKGRAY);
		head.outline = true;
		head.outlineColor = rum;
	}

	virtual void Update(float deltaTime) override;
	virtual void Draw() const override;

	void Input(const Vector3 forward, const Vector3 right);

	// Requests a jump. The request is consumed on the next update if player is grounded
	void Jump();

	// World-space point at the bottom of the collider
	vec3::Vector3 GetFootPosition() const
	{
		return {
			transform.position.x,
			transform.position.y + colliderOffset.y - colliderSize.y * 0.5f,
			transform.position.z
		};
	}

	void SetCamera(Camera3D* camera) { cam = camera; }
	void SetCollisionWorld(const CollisionWorld* world) { collision = world; }

private:
	bool MoveWithCollision(const vec3::Vector3& delta, const CollisionWorld& world, float maxStep);
	bool MoveAxis(int axis, float amount, const CollisionWorld& world);
	bool CanMoveAxis(int axis, float amount, const CollisionWorld& world) const;

	const Camera3D* cam = nullptr;
	const CollisionWorld* collision = nullptr;
	float verticalVelocity = 0.f;
	bool bIsGrounded = false;
	bool bJumpRequested = false;
};
