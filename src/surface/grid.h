#pragma once

#include "structs/vec3.h"
#include <vector>
#include "structs/resources.h"
#include "groundCube.h"

struct Grid
{
public:
	Grid(vec3::Vector3 gridSize, float cellSize, float cellPadding, const Resources& res);

	void Draw();
	void SetOutline(bool enabled);

	// Every solid piece of level geometry: floor tiles and obstacles.
	std::vector<GameObject*> GetColliders();

	vec3::Vector3 GetGridSize() const { return gridSize; }
	float GetCellSize() const { return cellSize; }

private:
	vec3::Vector3 gridSize;
	float cellSize;
	std::vector<GroundCube> floor;
	std::vector<GroundCube> obstacles;

	// Invisible walls that keep the player on the floor. They take part in
	// collision but are not drawn and are not surface-raycast targets.
	std::vector<GameObject> boundaries;
};