#include "grid.h"
#include "constants/colors.h"

namespace
{
	BoundingBox CubeBounds(const GroundCube& cube)
	{
		const vec3::Vector3& p = cube.transform.position;
		const vec3::Vector3& s = cube.transform.scale;

		return {
			{ p.x - s.x * 0.5f, p.y - s.y * 0.5f, p.z - s.z * 0.5f },
			{ p.x + s.x * 0.5f, p.y + s.y * 0.5f, p.z + s.z * 0.5f }
		};
	}
}

Grid::Grid(vec3::Vector3 gridSize, float cellSize, float cellPadding, const Resources& res) : gridSize(gridSize), cellSize(cellSize)
{
	vec3::Vector3 offset = { (gridSize.x - 1) * 0.5f * cellSize, -cellSize * 0.5f, (gridSize.z - 1) * 0.5f * cellSize };
	vec3::Vector3 scale = { cellSize - cellPadding };

	for (int rows = 0; rows < gridSize.z; ++rows)
	{
		for (int cols = 0; cols < gridSize.x; ++cols)
		{
			for (int height = 0; height < gridSize.y; ++height)
			{
				vec3::Vector3 pos = vec3::Vector3(cols * cellSize, height * cellSize, rows * cellSize) - offset;
				GroundCube cube = GroundCube(scale, pos, albescentWhite, bouquet, res);
				floor.push_back(cube);
			}
		}
	}

	// Obstacles sit on top of the topmost floor layer. Their transform scale
	// doubles as their collider, so the octree can index them without extra data.
	const float surfaceY = gridSize.y * cellSize - cellPadding * 0.5f;

	auto addObstacle = [&](vec3::Vector3 size, float x, float z, Color color, Color outlineColor)
	{
		const vec3::Vector3 pos = {
			x,
			surfaceY + size.y * 0.5f,
			z
		};
		obstacles.push_back(GroundCube(size, pos, color, outlineColor, res));
	};

	obstacles.reserve(4);
	addObstacle({ 1.5f, 2.0f, 1.5f }, -2.5f, -1.5f, cadetBlue, rum);
	addObstacle({ 1.0f, 1.0f, 2.5f }, 2.5f, 1.0f, thatch, bouquet);
	addObstacle({ 1.2f, 1.5f, 1.2f }, 0.5f, -3.0f, santasGrey, rum);
	addObstacle({ 2.0f, 0.8f, 1.0f }, 2.0f, -3.0f, bone, bouquet);

	// Keep the player on the level. The walls are just collision volumes, so
	// they never appear in the scene or block the mouse ray.
	const float halfX = (gridSize.x * cellSize - cellPadding) * 0.5f;
	const float halfZ = (gridSize.z * cellSize - cellPadding) * 0.5f;
	constexpr float wallThickness = 0.5f;
	constexpr float wallHeight = 3.f;
	const float wallY = surfaceY + wallHeight * 0.5f;

	auto addBoundary = [&](float x, float z, vec3::Vector3 size)
	{
		GameObject wall;
		wall.transform.position = { x, wallY, z };
		wall.transform.scale = size;
		boundaries.push_back(wall);
	};

	boundaries.reserve(4);
	addBoundary(-(halfX + wallThickness * 0.5f), 0.f,
				{ wallThickness, wallHeight, halfZ * 2.f + wallThickness * 2.f });
	addBoundary(halfX + wallThickness * 0.5f, 0.f,
				{ wallThickness, wallHeight, halfZ * 2.f + wallThickness * 2.f });
	addBoundary(0.f, -(halfZ + wallThickness * 0.5f),
				{ halfX * 2.f + wallThickness * 2.f, wallHeight, wallThickness });
	addBoundary(0.f, halfZ + wallThickness * 0.5f,
				{ halfX * 2.f + wallThickness * 2.f, wallHeight, wallThickness });
}

void Grid::Draw()
{
	for (const GroundCube& cube : floor)
	{
		cube.Draw();
	}

	for (const GroundCube& cube : obstacles)
	{
		cube.Draw();
	}
}

void Grid::SetOutline(bool enabled)
{
	for (GroundCube& cube : floor)
	{
		cube.SetOutline(enabled);
	}

	for (GroundCube& cube : obstacles)
	{
		cube.SetOutline(enabled);
	}
}

std::vector<GameObject*> Grid::GetColliders()
{
	std::vector<GameObject*> colliders;
	colliders.reserve(floor.size() + obstacles.size() + boundaries.size());

	for (GroundCube& cube : floor)
	{
		colliders.push_back(&cube);
	}

	for (GroundCube& cube : obstacles)
	{
		colliders.push_back(&cube);
	}

	for (GameObject& wall : boundaries)
	{
		colliders.push_back(&wall);
	}

	return colliders;
}
