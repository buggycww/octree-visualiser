#include "Octree.h"
#include <algorithm>

Octree::OctNode::OctNode(const AABB& _bounds, int _depth)
    :bounds(_bounds), depth(_depth)
{
}

Octree::Octree(const AABB& bounds, int cap, int maxD,
               float _minW, float _minH, float _minD)
    : capacity(std::max(cap, 1)), maxDepth(std::max(maxD, 0)),
      minW(_minW), minH(_minH), minD(_minD)
{
    root = std::make_unique<OctNode>(bounds, 0);
}

bool Octree::insert(const AABB& bounds, std::uint32_t id) {
    if (bounds.isEmpty()) return false;

    // Entries outside the root can never be found by a query.
    if (!root->bounds.contains(bounds)) return false;

    return insertHelper(root.get(), bounds, id);
}

bool Octree::remove(std::uint32_t id) {
    return removeHelper(root.get(), id);
}

void Octree::clear() {
    AABB bounds = root->bounds;
    root = std::make_unique<OctNode>(bounds, 0);
}

int Octree::countNodes() const {
    return countNodesHelper(root.get());
}

int Octree::countEntries() const {
    return countEntriesHelper(root.get());
}

bool Octree::insertHelper(OctNode* node, const AABB& bounds, std::uint32_t id) {
    if (!node->isLeaf()) {
        int o = getOctant(node->bounds, bounds);
        if (o >= 0) return insertHelper(node->children[o].get(), bounds, id);

        // The entry spans a split; keep it here so it is found from both sides.
        node->entries.push_back({ bounds, id });
        return true;
    }

    node->entries.push_back({ bounds, id });
    if ((int)node->entries.size() > capacity) subdivide(node);
    return true;
}

bool Octree::removeHelper(OctNode* node, std::uint32_t id) {
    if (node == nullptr) return false;

    auto it = std::find_if(node->entries.begin(), node->entries.end(),
        [id](const Entry& entry) { return entry.id == id; });

    if (it != node->entries.end()) {
        node->entries.erase(it);
        return true;
    }

    if (!node->isLeaf()) {
        for (const auto& child : node->children) {
            if (removeHelper(child.get(), id)) return true;
        }
    }

    return false;
}

void Octree::subdivide(OctNode* node) {
    if (node->depth >= maxDepth) return;

    const float halfW = node->bounds.w / 2.f;
    const float halfH = node->bounds.h / 2.f;
    const float halfD = node->bounds.d / 2.f;
    if (halfW < minW || halfH < minH || halfD < minD) return;

    const float middleX = node->bounds.x + halfW;
    const float middleY = node->bounds.y + halfH;
    const float middleZ = node->bounds.z + halfD;

    // Children are indexed by which side of each split they occupy:
    // bit 0 = +X, bit 1 = +Y, bit 2 = +Z. This is the 3D version of the
    // old NW/NE/SW/SE quadrant table.
    for (int i = 0; i < 8; i++) {
        const AABB childBounds{
            (i & 1) ? middleX : node->bounds.x,
            (i & 2) ? middleY : node->bounds.y,
            (i & 4) ? middleZ : node->bounds.z,
            halfW,
            halfH,
            halfD
        };

        node->children[i] = std::make_unique<OctNode>(childBounds, node->depth + 1);
    }

    // Move entries that fit entirely inside one child down. If entry straddles, stay in parent node.
    std::vector<Entry> entries;
    entries.swap(node->entries);

    for (const Entry& entry : entries) {
        int o = getOctant(node->bounds, entry.bounds);
        if (o >= 0) {
            insertHelper(node->children[o].get(), entry.bounds, entry.id);
        }
        else {
            node->entries.push_back(entry);
        }
    }
}

int Octree::countNodesHelper(const OctNode* node) const {
    if (node == nullptr) return 0;

    int count = 1;
    if (!node->isLeaf()) {
        for (const auto& child : node->children) {
            count += countNodesHelper(child.get());
        }
    }
    return count;
}

int Octree::countEntriesHelper(const OctNode* node) const {
    if (node == nullptr) return 0;

    int count = static_cast<int>(node->entries.size());
    if (!node->isLeaf()) {
        for (const auto& child : node->children) {
            count += countEntriesHelper(child.get());
        }
    }
    return count;
}

int Octree::getOctant(const AABB& bounds, const AABB& item) const {
    const float middleX = bounds.x + bounds.w / 2.f;
    const float middleY = bounds.y + bounds.h / 2.f;
    const float middleZ = bounds.z + bounds.d / 2.f;

    // A box that crosses any split belongs to the parent.
    if (item.minX() < middleX && item.maxX() > middleX) return -1;
    if (item.minY() < middleY && item.maxY() > middleY) return -1;
    if (item.minZ() < middleZ && item.maxZ() > middleZ) return -1;

    const bool negativeX = item.maxX() <= middleX;
    const bool negativeY = item.maxY() <= middleY;
    const bool negativeZ = item.maxZ() <= middleZ;

    int octant = 0;
    if (!negativeX) octant |= 1;
    if (!negativeY) octant |= 2;
    if (!negativeZ) octant |= 4;
    return octant;
}
