#pragma once
#include "Common.hpp"
#include <vector>

struct Wall {
    Vector3 position;
    Vector3 size;
    Color color;
};

struct Shelf {
    Vector3 position;
    Vector3 size;
    Color color;
    Color topColor;
};

class Shop {
public:
    Shop();
    ~Shop() = default;

    void Init();
    void Render();
    const std::vector<AABB>& GetColliders() const { return colliders; }

private:
    float shopWidth;
    float shopLength;
    float shopHeight;

    std::vector<Wall> walls;
    std::vector<Shelf> shelves;
    std::vector<AABB> colliders;

    void BuildStructure();
    void BuildColliders();
};
