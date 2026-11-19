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
				cube.bIsSurface = true;
				floor.push_back(cube);
			}
		}
	}

	// Obstacles sit on top of the topmost floor layer. Their transform scale
	// doubles as their collider, so the octree can index them without extra data.
	const float surfaceY = gridSize.y * cellSize - cellPadding * 0.5f;

	auto addObstacleAt = [&](vec3::Vector3 size, float x, float z, float bottom,
							 Color color, Color outlineColor)
	{
		const vec3::Vector3 pos = {
			x,
			surfaceY + bottom + size.y * 0.5f,
			z
		};
		obstacles.push_back(GroundCube(size, pos, color, outlineColor, res));
		obstacles.back().bIsObstacle = true;
	};

	auto addObstacle = [&](vec3::Vector3 size, float x, float z, Color color, Color outlineColor)
	{
		addObstacleAt(size, x, z, 0.f, color, outlineColor);
	};

	obstacles.reserve(20);

	// Ground-level blocks and crates. Their varied heights are what gives the
	// octree something to subdivide in Y as well as in X/Z.
	addObstacle({ 1.4f, 2.0f, 1.4f }, -2.7f, -1.8f, cadetBlue, rum);
	addObstacle({ 1.0f, 1.0f, 2.2f }, 2.6f, 1.2f, thatch, bouquet);
	addObstacle({ 1.2f, 1.5f, 1.2f }, 0.4f, -3.2f, santasGrey, rum);
	addObstacle({ 1.8f, 0.8f, 1.0f }, 2.0f, -3.4f, bone, bouquet);
	addObstacle({ 1.0f, 3.0f, 1.0f }, -4.2f, -3.6f, santasGrey, rum);
	addObstacle({ 1.0f, 1.0f, 1.0f }, 4.3f, 1.2f, bone, bouquet);
	addObstacle({ 1.0f, 1.0f, 1.0f }, 4.3f, -0.2f, thatch, bouquet);
	addObstacle({ 0.8f, 0.6f, 0.8f }, -1.2f, -4.2f, cadetBlue, rum);
	addObstacle({ 1.0f, 1.0f, 1.0f }, 2.8f, 4.4f, thatch, bouquet);
	addObstacle({ 0.8f, 1.0f, 0.8f }, 1.0f, -0.6f, bone, bouquet);

	// Three-tier terrace in the front-left corner. Each tier is one unit above
	// the previous one, so the player can jump from the floor to the first and
	// climb the rest.
	addObstacleAt({ 4.0f, 1.0f, 2.6f }, -2.6f, 3.6f, 0.0f, bone, bouquet);
	addObstacleAt({ 2.4f, 1.0f, 1.4f }, -3.2f, 4.0f, 1.0f, cadetBlue, rum);
	addObstacleAt({ 1.2f, 1.0f, 0.8f }, -3.6f, 4.2f, 2.0f, santasGrey, rum);
	addObstacleAt({ 1.0f, 1.0f, 1.0f }, -1.2f, 4.2f, 1.0f, thatch, bouquet);
	addObstacleAt({ 1.0f, 1.0f, 1.0f }, -1.4f, 3.0f, 1.0f, santasGrey, rum);

	// Raised platform and a tall corner tower on the back-right side. Crates on
	// the platform bridge the last jump to the tower top.
	addObstacleAt({ 2.2f, 1.0f, 2.2f }, 3.6f, -2.2f, 0.0f, bone, bouquet);
	addObstacleAt({ 1.0f, 1.0f, 1.0f }, 3.2f, -2.6f, 1.0f, thatch, bouquet);
	addObstacleAt({ 1.0f, 1.0f, 1.0f }, 4.2f, -2.8f, 1.0f, santasGrey, rum);
	addObstacleAt({ 1.0f, 3.0f, 1.0f }, 4.4f, -4.2f, 0.0f, cadetBlue, rum);

	// A block stacked on the second tier, level with the top tier. It gives the
	// upper octree cells on the east side something to hold, so climbing does
	// not always flatten the visualised cell back to the whole quadrant.
	addObstacleAt({ 0.7f, 1.0f, 0.7f }, -2.3f, 3.8f, 2.0f, bone, bouquet);

	// Keep the player on the level. The walls are just collision volumes, so
	// they never appear in the scene or block the mouse ray. They are deliberately
	// taller than the highest platform plus a jump, so a leaping player cannot
	// leave the map.
	const float halfX = (gridSize.x * cellSize - cellPadding) * 0.5f;
	const float halfZ = (gridSize.z * cellSize - cellPadding) * 0.5f;
	constexpr float wallThickness = 0.5f;
	constexpr float wallHeight = 12.f;
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
