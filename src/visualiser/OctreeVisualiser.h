#pragma once

#include <vector>

#include <raylib.h>

#include "collision detection/Octree.h"
#include "structs/vec3.h"

class CollisionWorld;

// Visualises the octree node that stores the player's surroundings.
//
// The active node is the deepest node on the player's branch that actually
// stores entries. Entries are shown from the node itself, not from a spatial
// query: an object is highlighted exactly when the octree stores it in this
// node. Straddling boxes stay in their parent by design, so the parent bounds
// are drawn too, but only the active node's entries are highlighted.
class OctreeVisualiser
{
public:
	void SetWorld(const CollisionWorld* collisionWorld) { world = collisionWorld; }

	// Resolves the active node and the entries it stores for this frame.
	void Update(const vec3::Vector3& focus);

	void Draw() const;

	bool HasCell() const { return cell != nullptr; }
	int GetDepth() const { return cell != nullptr ? cell->depth : -1; }
	int GetParentDepth() const { return parentCell != nullptr ? parentCell->depth : -1; }
	bool HasParent() const { return parentCell != nullptr; }
	int GetEntryCount() const { return cell != nullptr ? static_cast<int>(cell->entries.size()) : 0; }
	int GetSurfaceCount() const { return static_cast<int>(surfaces.size()); }
	int GetObstacleCount() const { return static_cast<int>(obstacles.size()); }
	AABB GetCellBounds() const { return cell != nullptr ? cell->bounds : AABB{}; }

private:
	const CollisionWorld* world = nullptr;
	const Octree::OctNode* cell = nullptr;       // node whose entries are shown
	const Octree::OctNode* parentCell = nullptr; // drawn for context
	std::vector<AABB> surfaces;
	std::vector<AABB> obstacles;
};
