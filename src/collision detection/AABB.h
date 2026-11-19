#pragma once

// Axis-aligned bounding box in 3D. Non-rotated box.
//
// Stored as a minimum corner plus full extents:
//     x, y, z  = minimum corner
//     w, h, d  = full width, height and depth
// Two boxes that only touch along a face, edge or corner are not considered overlapping.
struct AABB {
    float x = 0.f, y = 0.f, z = 0.f;
    float w = 0.f, h = 0.f, d = 0.f;

    float minX() const { return x; }
    float minY() const { return y; }
    float minZ() const { return z; }
    float maxX() const { return x + w; }
    float maxY() const { return y + h; }
    float maxZ() const { return z + d; }
    float centerX() const { return x + w * 0.5f; }
    float centerY() const { return y + h * 0.5f; }
    float centerZ() const { return z + d * 0.5f; }

    bool contains(float px, float py, float pz) const {
        return px >= x && px < x + w &&
               py >= y && py < y + h &&
               pz >= z && pz < z + d;
    }

    bool contains(const AABB& other) const {
        return other.x >= x && other.maxX() <= maxX() &&
               other.y >= y && other.maxY() <= maxY() &&
               other.z >= z && other.maxZ() <= maxZ();
    }

    bool intersects(const AABB& other) const {
        return x < other.maxX() && maxX() > other.x &&
               y < other.maxY() && maxY() > other.y &&
               z < other.maxZ() && maxZ() > other.z;
    }

    bool isEmpty() const { return w <= 0.f || h <= 0.f || d <= 0.f; }

    // Builds AABB box with center (cx, cy, cz)
    static AABB fromCenter(float cx, float cy, float cz,
                           float sizeX, float sizeY, float sizeZ) {
        return {
            cx - sizeX * 0.5f,
            cy - sizeY * 0.5f,
            cz - sizeZ * 0.5f,
            sizeX,
            sizeY,
            sizeZ
        };
    }
};
