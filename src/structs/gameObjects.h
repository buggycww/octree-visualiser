#pragma once

#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "constants/colors.h"
#include "structs/resources.h"
#include "structs/vec3.h"

struct Transform3D
{
	vec3::Vector3 position{ 0.f };
	vec3::Vector3 rotation{ 0.f };  // Euler angles, RADIANS
	vec3::Vector3 scale{ 1.f };

	Matrix ToMatrix() const
	{
		const Matrix scaleMatrix = MatrixScale(scale.x, scale.y, scale.z);
		const Matrix rotationMatrix = MatrixRotateXYZ({ rotation.x, rotation.y, rotation.z });
		const Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);

		return MatrixMultiply(MatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);
	}
};

enum class OutlineStyle
{
	Silhouette,   // inverted hull around the mesh for round/organic shapes
	Edges         // 12 box edges for unit-cube tile model
};

struct ModelAttachment
{
	Model model{};
	Transform3D local;
	Color tint = WHITE;

	bool outline = false;
	Color outlineColor = BLACK;
	OutlineStyle outlineStyle = OutlineStyle::Silhouette;
	bool bIsVisible = true;
};

class GameObject
{
public:
	Transform3D transform;

	// Local-space collision box, centred on colliderOffset. 
	// Enables collision to differ from visible scale.
	// Keep zero to use transform's unit cube. 
	vec3::Vector3 colliderSize{ 0.f };
	vec3::Vector3 colliderOffset{ 0.f };

	GameObject* parent = nullptr;
	std::vector<ModelAttachment> models;

	vec3::Vector3 direction{ 0.f };
	float speed = 0.f;

	bool bIsVisible = true;
	bool bIsObstacle = false;
	bool bIsSurface = false;

	GameObject() = default;
	virtual ~GameObject() = default;

	Matrix WorldMatrix() const
	{
		const Matrix local = transform.ToMatrix();

		if (parent != nullptr)
			return MatrixMultiply(local, parent->WorldMatrix());

		return local;
	}

	ModelAttachment& Attach(const Model& model, const Transform3D& local = {}, Color tint = WHITE)
	{
		models.push_back({ model, local, tint });
		return models.back();
	}

	void SetOutline(bool enabled)
	{
		for (ModelAttachment& attachment : models)
			attachment.outline = enabled;
	}

	void Move(float deltaTime);

	virtual void Update(float deltaTime) { Move(deltaTime); }
	virtual void Draw() const;
};