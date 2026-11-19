#include "OctreeVisualiser.h"

#include <algorithm>
#include <vector>

#include <rlgl.h>

#include "collision detection/CollisionWorld.h"
#include "constants/colors.h"
#include "structs/gameObjects.h"

namespace
{
	// ancestor node draw settings
	constexpr float parentFillAlpha = 0.18f;
	constexpr float parentEdgeAlpha = 1.f;
	constexpr float cellFillAlpha = 0.04f;

	constexpr float highlightPadding = 0.035f;

	// for floor tiles
	constexpr float surfaceAlpha = 0.32f;
	constexpr float surfacePadding = 0.012f;

	struct CellChain
	{
		const Octree::OctNode* cell = nullptr;
		std::vector<const Octree::OctNode*> ancestors;
	};

	CellChain FindCell(const Octree::OctNode* node,
					   std::vector<const Octree::OctNode*>& path,
					   float x, float y, float z)
	{
		if (node == nullptr || !node->bounds.contains(x, y, z)) return {};

		CellChain result;
		if (!node->entries.empty())
		{
			result.cell = node;
			result.ancestors = path;
		}

		path.push_back(node);

		if (!node->isLeaf())
		{
			for (const auto& child : node->children)
			{
				if (child != nullptr && child->bounds.contains(x, y, z))
				{
					const CellChain deeper = FindCell(child.get(), path, x, y, z);
					if (deeper.cell != nullptr) result = deeper;
					break;
				}
			}
		}

		path.pop_back();
		return result;
	}
}

void OctreeVisualiser::Update(const vec3::Vector3& focus)
{
	cell = nullptr;
	ancestors.clear();
	surfaces.clear();
	obstacles.clear();

	if (world == nullptr) return;

	const Octree* tree = world->getTree();
	if (tree == nullptr) return;

	std::vector<const Octree::OctNode*> path;
	const CellChain chain = FindCell(tree->getRoot(), path, focus.x, focus.y, focus.z);
	cell = chain.cell;
	if (cell == nullptr) return;

	ancestors = chain.ancestors;
	if (!ancestors.empty() && ancestors.front() == tree->getRoot())
	{
		ancestors.erase(ancestors.begin());
	}

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

	// Draw every ancestor bound, root excluded, from the outermost inwards.
	// The immediate parent carries the translucent volume; higher ancestors are
	// fainter outlines so the nesting stays readable. Their entries are never
	// highlighted.
	for (std::size_t i = 0; i < ancestors.size(); ++i)
	{
		const Octree::OctNode* ancestor = ancestors[i];
		const AABB& ancestorBounds = ancestor->bounds;
		const Vector3 ancestorCenter{
			ancestorBounds.centerX(),
			ancestorBounds.centerY(),
			ancestorBounds.centerZ()
		};
		const Vector3 ancestorSize{ ancestorBounds.w, ancestorBounds.h, ancestorBounds.d };

		const int distance = static_cast<int>(ancestors.size() - 1 - i);

		rlDisableDepthMask();
		DrawCubeV(ancestorCenter, ancestorSize, Fade(octreeCell, parentFillAlpha));
		rlDrawRenderBatchActive();
		rlEnableDepthMask();

		DrawCubeWiresV(ancestorCenter, ancestorSize,
			Fade(octreeCellEdge, parentEdgeAlpha));
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
		// Keep the highlight shell inside the cell. Otherwise an object that
		// touches the node boundary (or one that fills the node) has its shell
		// drawn over the node's own outline.
		const float minX = std::max(obstacle.minX() - highlightPadding, bounds.minX());
		const float minY = std::max(obstacle.minY() - highlightPadding, bounds.minY());
		const float minZ = std::max(obstacle.minZ() - highlightPadding, bounds.minZ());
		const float maxX = std::min(obstacle.maxX() + highlightPadding, bounds.maxX());
		const float maxY = std::min(obstacle.maxY() + highlightPadding, bounds.maxY());
		const float maxZ = std::min(obstacle.maxZ() + highlightPadding, bounds.maxZ());

		const Vector3 obstacleCenter{
			(minX + maxX) * 0.5f,
			(minY + maxY) * 0.5f,
			(minZ + maxZ) * 0.5f
		};
		const Vector3 obstacleSize{ maxX - minX, maxY - minY, maxZ - minZ };

		DrawCubeV(obstacleCenter, obstacleSize, octreeHighlight);
		DrawCubeWiresV(obstacleCenter, obstacleSize,
					   ColorBrightness(octreeHighlight, -0.35f));
	}

	// draw edges
	rlDisableDepthTest();
	DrawCubeWiresV(center, size, octreeCellEdge);
	constexpr float edgeInflation = 0.014f;
	DrawCubeWiresV(center,
				   { size.x + edgeInflation, size.y + edgeInflation, size.z + edgeInflation },
				   octreeCellEdge);
	DrawCubeWiresV(center,
				   { size.x + edgeInflation * 2.f, size.y + edgeInflation * 2.f, size.z + edgeInflation * 2.f },
				   octreeCellEdge);
	rlEnableDepthTest();
}
