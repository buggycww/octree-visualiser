#include "OctreeVisualiser.h"

#include <rlgl.h>

#include "collision detection/CollisionWorld.h"
#include "constants/colors.h"
#include "structs/gameObjects.h"

namespace
{
	// Per-face opacity of the active cell's volume. The cube shows both its near
	// and far faces, so the blended result ends up roughly twice this value.
	constexpr float cellFillAlpha = 0.26f;

	// The parent only provides context, so it is just a translucent outline.
	constexpr float parentEdgeAlpha = 0.6f;

	// Keeps the highlight shell just outside the stored box, so the two
	// surfaces do not z-fight.
	constexpr float highlightPadding = 0.035f;

	// Floor tiles have translucent highlight
	constexpr float surfaceAlpha = 0.55f;
	constexpr float surfacePadding = 0.012f;

	struct CellChain
	{
		const Octree::OctNode* cell = nullptr;
		const Octree::OctNode* parent = nullptr;
	};

	// Walks down the branch containing the point and keeps the deepest node
	// that stores entries itself. A node with no entries of its own is not
	// visualised: its parent is where the geometry actually lives.
	CellChain FindCell(const Octree::OctNode* node, const Octree::OctNode* parent,
					   float x, float y, float z)
	{
		if (node == nullptr || !node->bounds.contains(x, y, z)) return {};

		CellChain result;
		if (!node->entries.empty())
		{
			result.cell = node;
			result.parent = parent;
		}

		if (!node->isLeaf())
		{
			for (const auto& child : node->children)
			{
				if (child != nullptr && child->bounds.contains(x, y, z))
				{
					const CellChain deeper = FindCell(child.get(), node, x, y, z);
					if (deeper.cell != nullptr) result = deeper;
					break;
				}
			}
		}

		return result;
	}
}

void OctreeVisualiser::Update(const vec3::Vector3& focus)
{
	cell = nullptr;
	parentCell = nullptr;
	surfaces.clear();
	obstacles.clear();

	if (world == nullptr) return;

	const Octree* tree = world->getTree();
	if (tree == nullptr) return;

	const CellChain chain = FindCell(tree->getRoot(), nullptr, focus.x, focus.y, focus.z);
	cell = chain.cell;
	if (cell == nullptr) return;

	// The root has no meaningful parent, so a depth-1 cell gets no parent box.
	parentCell = (chain.parent != tree->getRoot()) ? chain.parent : nullptr;

	// Only the entries stored in this node are highlighted. This is the same
	// data the octree would return for a query that stops at this node, and it
	// is why straddling objects appear in an ancestor's highlight set.
	for (const Octree::Entry& entry : cell->entries)
	{
		GameObject* object = world->getOwner(entry.id);
		if (object == nullptr) continue;

		if (object->bIsObstacle) obstacles.push_back(entry.bounds);
		else if (object->bIsSurface) surfaces.push_back(entry.bounds);
	}
}

void OctreeVisualiser::Draw() const
{
	if (cell == nullptr) return;

	// Parent bounds first, as context. Its entries are deliberately not
	// highlighted: they belong to the parent, not to the active node.
	if (parentCell != nullptr)
	{
		const AABB& parentBounds = parentCell->bounds;
		const Vector3 parentCenter{
			parentBounds.centerX(),
			parentBounds.centerY(),
			parentBounds.centerZ()
		};
		const Vector3 parentSize{ parentBounds.w, parentBounds.h, parentBounds.d };

		DrawCubeWiresV(parentCenter, parentSize, Fade(octreeCellEdge, parentEdgeAlpha));
	}

	const AABB& bounds = cell->bounds;
	const Vector3 center{ bounds.centerX(), bounds.centerY(), bounds.centerZ() };
	const Vector3 size{ bounds.w, bounds.h, bounds.d };

	// raylib batches geometry and flushes it later, so the depth mask has to be
	// restored only after the batch holding the translucent cube has been
	// submitted. Without the explicit flush the cube would be replayed with
	// depth writes enabled, hiding the far side of the volume.
	rlDisableDepthMask();
	DrawCubeV(center, size, Fade(octreeCell, cellFillAlpha));
	rlDrawRenderBatchActive();
	rlEnableDepthMask();

	// draw floor tiles stored in the cell
	for (const AABB& surface : surfaces)
	{
		const Vector3 surfaceCenter{
			surface.centerX(),
			surface.centerY(),
			surface.centerZ()
		};
		const Vector3 surfaceSize{
			surface.w + surfacePadding * 2.f,
			surface.h + surfacePadding * 2.f,
			surface.d + surfacePadding * 2.f
		};

		DrawCubeV(surfaceCenter, surfaceSize, Fade(octreeHighlight, surfaceAlpha));
		DrawCubeWiresV(surfaceCenter, surfaceSize,
					   ColorBrightness(octreeHighlight, -0.3f));
	}

	// draw obstacles stored in the cell
	for (const AABB& obstacle : obstacles)
	{
		const Vector3 obstacleCenter{
			obstacle.centerX(),
			obstacle.centerY(),
			obstacle.centerZ()
		};
		const Vector3 obstacleSize{
			obstacle.w + highlightPadding * 2.f,
			obstacle.h + highlightPadding * 2.f,
			obstacle.d + highlightPadding * 2.f
		};

		DrawCubeV(obstacleCenter, obstacleSize, octreeHighlight);
		DrawCubeWiresV(obstacleCenter, obstacleSize,
					   ColorBrightness(octreeHighlight, -0.35f));
	}

	// draw edges
	DrawCubeWiresV(center, size, octreeCellEdge);
	constexpr float edgeInflation = 0.012f;
	DrawCubeWiresV(center,
				   { size.x + edgeInflation, size.y + edgeInflation, size.z + edgeInflation },
				   octreeCellEdge);
}
