#pragma once

#include "AABB.h"
#include "Octree.h"

#include <cstdint>
#include <memory>
#include <vector>

class GameObject;

// Spatial index for the level's solid colliders.
//
// Static geometry never moves, so the tree is built once after the map is
// loaded. The caller passes the objects that should take part in collision;
// the world-space bounds are derived from each object's Transform3D.
class CollisionWorld {
public:
    void build(const std::vector<GameObject*>& objects);
    void clear();
    void query(const AABB& area, std::vector<GameObject*>& out) const;
    bool overlaps(const AABB& area) const;

    bool empty() const { return tree == nullptr; }
    int nodeCount() const { return tree ? tree->countNodes() : 0; }
    int entryCount() const { return tree ? tree->countEntries() : 0; }

    // Read-only view of the tree, used by the debug visualiser.
    const Octree* getTree() const { return tree.get(); }

    // Resolves an entry id back to the object it was built from.
    GameObject* getOwner(std::uint32_t id) const;

    // World-space collider bounds of an object: the same box the tree stores.
    static AABB boundsOf(const GameObject& object);

private:
    std::unique_ptr<Octree> tree;

    // The octree stores an index into owners, keeping the tree independent of
    // game types.
    std::vector<GameObject*> owners;
};
