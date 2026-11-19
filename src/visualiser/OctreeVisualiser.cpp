#include "OctreeVisualiser.h"

#include <rlgl.h>

#include "collision detection/CollisionWorld.h"
#include "constants/colors.h"
#include "structs/gameObjects.h"

namespace
{
	// parent node draw settings
	constexpr float parentFillAlpha = 0.18f;
	constexpr float parentEdgeAlpha = 0.5f;
	constexpr float cellFillAlpha = 0.04f;

	constexpr float highlightPadding = 0.035f;

	// for floor tiles
	constexpr float surfaceAlpha = 0.55f;
	constexpr float surfacePadding = 0.012f;

	struct CellChain
	{
		const Octree::OctNode* cell = nullptr;
		const Octree::OctNode* parent = nullptr;
	};

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

	parentCell = (chain.parent != tree->getRoot()) ? chain.parent : nullptr;

	// Only entries stored in this node are highlighted
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

	// Draw parent bounds first for context, but don't draw their entries
	if (parentCell != nullptr)
	{
		const AABB& parentBounds = parentCell->bounds;
		const Vector3 parentCenter{
			parentBounds.centerX(),
			parentBounds.centerY(),
			parentBounds.centerZ()
		};
		const Vector3 parentSize{ parentBounds.w, parentBounds.h, parentBounds.d };

		rlDisableDepthMask();
		DrawCubeV(parentCenter, parentSize, Fade(octreeCell, parentFillAlpha));
		rlDrawRenderBatchActive();
		rlEnableDepthMask();

		DrawCubeWiresV(parentCenter, parentSize, Fade(octreeCellEdge, parentEdgeAlpha));
	}

	const AABB& bounds = cell->bounds;
	const Vector3 center{ bounds.centerX(), bounds.centerY(), bounds.centerZ() };
	const Vector3 size{ bounds.w, bounds.h, bounds.d };

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
