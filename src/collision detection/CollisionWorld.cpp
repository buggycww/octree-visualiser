#include "CollisionWorld.h"

#include "structs/gameObjects.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
bool hasExplicitCollider(const GameObject& obj) {
    const vec3::Vector3& s = obj.colliderSize;
    return s.x > 0.f && s.y > 0.f && s.z > 0.f;
}

// World-space AABB of a GameObject.
//
// A GameObject's transform describes a unit cube: scale gives its full size,
// rotation rotates it and position places its centre. When an explicit local
// collider is set, that box is used instead, still in the object's local space.
// The eight corners are transformed and the smallest enclosing axis-aligned
// box is returned, so rotated and parented objects get correct bounds.
AABB colliderBounds(const GameObject& obj) {
    const Matrix world = obj.WorldMatrix();

    const bool explicitCollider = hasExplicitCollider(obj);
    const vec3::Vector3 size = explicitCollider
        ? obj.colliderSize
        : vec3::Vector3{ 1.f, 1.f, 1.f };
    const vec3::Vector3 offset = explicitCollider
        ? obj.colliderOffset
        : vec3::Vector3{ 0.f, 0.f, 0.f };

    const float halfX = size.x * 0.5f;
    const float halfY = size.y * 0.5f;
    const float halfZ = size.z * 0.5f;

    float minX = 0.f, minY = 0.f, minZ = 0.f;
    float maxX = 0.f, maxY = 0.f, maxZ = 0.f;

    for (int i = 0; i < 8; i++) {
        const Vector3 corner{
            offset.x + ((i & 1) ? halfX : -halfX),
            offset.y + ((i & 2) ? halfY : -halfY),
            offset.z + ((i & 4) ? halfZ : -halfZ)
        };
        const Vector3 p = Vector3Transform(corner, world);

        if (i == 0) {
            minX = maxX = p.x;
            minY = maxY = p.y;
            minZ = maxZ = p.z;
        }
        else {
            minX = std::min(minX, p.x);
            minY = std::min(minY, p.y);
            minZ = std::min(minZ, p.z);
            maxX = std::max(maxX, p.x);
            maxY = std::max(maxY, p.y);
            maxZ = std::max(maxZ, p.z);
        }
    }

    return { minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ };
}

bool hasVolume(const GameObject& obj) {
    if (hasExplicitCollider(obj)) return true;

    const vec3::Vector3& s = obj.transform.scale;
    return std::abs(s.x) > 0.f && std::abs(s.y) > 0.f && std::abs(s.z) > 0.f;
}
}

void CollisionWorld::clear() {
    tree.reset();
    owners.clear();
}

void CollisionWorld::build(const std::vector<GameObject*>& objects) {
    clear();

    // Fit the root to the level geometry. Padding keeps boxes on the map's
    // maximum edges inside the root despite floating-point rounding.
    float minX = 0.f, minY = 0.f, minZ = 0.f;
    float maxX = 0.f, maxY = 0.f, maxZ = 0.f;
    bool found = false;

    for (const GameObject* obj : objects) {
        if (obj == nullptr || !hasVolume(*obj)) continue;

        const AABB bounds = colliderBounds(*obj);
        if (bounds.isEmpty()) continue;

        if (!found) {
            minX = bounds.minX();
            minY = bounds.minY();
            minZ = bounds.minZ();
            maxX = bounds.maxX();
            maxY = bounds.maxY();
            maxZ = bounds.maxZ();
            found = true;
        }
        else {
            minX = std::min(minX, bounds.minX());
            minY = std::min(minY, bounds.minY());
            minZ = std::min(minZ, bounds.minZ());
            maxX = std::max(maxX, bounds.maxX());
            maxY = std::max(maxY, bounds.maxY());
            maxZ = std::max(maxZ, bounds.maxZ());
        }
    }

    if (!found) return;

    constexpr float padding = 1.f;
    const AABB world{
        minX - padding,
        minY - padding,
        minZ - padding,
        (maxX - minX) + padding * 2.f,
        (maxY - minY) + padding * 2.f,
        (maxZ - minZ) + padding * 2.f
    };

    constexpr int capacity = 8;
    constexpr int maxDepth = 10;
    tree = std::make_unique<Octree>(world, capacity, maxDepth);

    for (GameObject* obj : objects) {
        if (obj == nullptr || !hasVolume(*obj)) continue;

        const AABB bounds = colliderBounds(*obj);
        const auto id = static_cast<std::uint32_t>(owners.size());

        if (!tree->insert(bounds, id)) continue; // outside the root

        owners.push_back(obj);
    }
}

void CollisionWorld::query(const AABB& area, std::vector<GameObject*>& out) const {
    if (!tree) return;

    tree->query(area, [&out, this](std::uint32_t id) {
        if (static_cast<std::size_t>(id) < owners.size()) {
            out.push_back(owners[id]);
        }
        return true;
    });
}

bool CollisionWorld::overlaps(const AABB& area) const {
    if (!tree) return false;

    bool found = false;
    tree->query(area, [&found](std::uint32_t) {
        found = true;
        return false; // one hit is enough
    });
    return found;
}
