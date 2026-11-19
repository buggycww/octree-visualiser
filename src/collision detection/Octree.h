#pragma once

#include "AABB.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

// Octree broad phase for axis-aligned boxes in 3D.
//
//   - Changed from Quadtree: An entry stores its full AABB, not just a point
//   - A box is stored exactly once, in the deepest node that fully contains it.
//     Boxes that straddle a split stay in the parent node. A query tests the
//     entries of every visited node, so straddling boxes are still found.
//   - The id is only an identity/handle. It lets the caller map a hit back to
//     the owning game object without the tree knowing about game types.
//
// An octant is indexed by one bit per axis: bit 0 = positive X, bit 1 = positive Y,
// bit 2 = positive Z.
class Octree {
    public:
        struct Entry {
            AABB bounds;
            std::uint32_t id;
        };

        struct OctNode {
            AABB bounds;
            std::vector<Entry> entries;
            std::array<std::unique_ptr<OctNode>, 8> children;
            int depth;

            OctNode(const AABB& _bounds, int _depth);
            bool isLeaf() const { return children[0] == nullptr; }
        };

        Octree(const AABB& _bounds, int _capacity, int _maxDepth,
               float _minW = 1.f, float _minH = 1.f, float _minD = 1.f);
        ~Octree() = default;

        Octree(const Octree&) = delete;
        Octree& operator= (const Octree&) = delete;

        bool insert(const AABB& bounds, std::uint32_t id);
        bool remove(std::uint32_t id);
        void clear();

        // Calls visit(id) for every entry overlapping bounds. Return false
        // from the visitor to stop traversal early; query() then returns false.
        template <typename Visitor>
        bool query(const AABB& bounds, Visitor&& visitor) const {
            return queryHelper(root.get(), bounds, visitor);
        }

        const OctNode* getRoot() const { return root.get(); }
        int getCapacity() const { return capacity; }
        int getMaxDepth() const { return maxDepth; }

        // Diagnostics for the debug overlay.
        int countNodes() const;
        int countEntries() const;

    private:
        std::unique_ptr<OctNode> root;
        int capacity; // max entries a node can hold before it subdivides
        int maxDepth; // max depth of a node
        float minW;
        float minH;
        float minD; // minimums to guard against dividing an excessively small node

        bool insertHelper(OctNode* node, const AABB& bounds, std::uint32_t id);
        template <typename Visitor>
        bool queryHelper(const OctNode* node, const AABB& bounds, Visitor& visitor) const;
        bool removeHelper(OctNode* node, std::uint32_t id);
        void subdivide(OctNode* node);
        int countNodesHelper(const OctNode* node) const;
        int countEntriesHelper(const OctNode* node) const;

        // Returns the child index fully containing item, or -1 if the item
        // crosses one of the node's splits and must stay in this node.
        int getOctant(const AABB& bounds, const AABB& item) const;
};

template <typename Visitor>
bool Octree::queryHelper(const OctNode* node, const AABB& bounds, Visitor& visitor) const {
    if (node == nullptr) return true;
    if (!node->bounds.intersects(bounds)) return true;

    for (const Entry& entry : node->entries) {
        if (!entry.bounds.intersects(bounds)) continue;
        if (!visitor(entry.id)) return false;
    }

    if (!node->isLeaf()) {
        for (const auto& child : node->children) {
            if (!queryHelper(child.get(), bounds, visitor)) return false;
        }
    }

    return true;
}
