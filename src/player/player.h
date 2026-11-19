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

		// The visible body is a small cylinder plus a head; the collider is a
		// single upright box that covers both.
		colliderSize = { 0.6f, 1.3f, 0.6f };
		colliderOffset = { 0.f, 0.35f, 0.f };

		Transform3D bodyLocal;
		bodyLocal.scale = { .3f, .6f, .3f };

		ModelAttachment& body = Attach(res.cylinderModel.Get(), bodyLocal, DARKGRAY);
		body.outline = true;
		body.outlineColor = rum;

		Transform3D headLocal;
		headLocal.position = { 0.f, .8f, 0.f };
		headLocal.scale = { 0.2f, 0.2f, 0.2f };

		ModelAttachment& head = Attach(res.sphereModel.Get(), headLocal, DARKGRAY);
		head.outline = true;
		head.outlineColor = rum;
	}

	virtual void Update(float deltaTime) override;
	virtual void Draw() const override;

	// Requests a jump. The request is consumed on the next update if the player
	// is standing on something solid.
	void Jump();

	void SetCamera(Camera3D* camera) { cam = camera; }
	void SetCollisionWorld(const CollisionWorld* world) { collision = world; }

private:
	bool MoveWithCollision(const vec3::Vector3& delta, const CollisionWorld& world, float maxStep);
	bool MoveAxis(int axis, float amount, const CollisionWorld& world);

	const Camera3D* cam = nullptr;
	const CollisionWorld* collision = nullptr;
	float verticalVelocity = 0.f;
	bool bIsGrounded = false;
	bool bJumpRequested = false;
};
