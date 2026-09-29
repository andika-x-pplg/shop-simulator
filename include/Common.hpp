#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// Axis-Aligned Bounding Box for basic collision
struct AABB {
    Vector3 min;
    Vector3 max;

    bool CheckCollision(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
};
