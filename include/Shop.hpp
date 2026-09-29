#pragma once
#include "Common.hpp"
#include "Rack.hpp"
#include <vector>
#include <memory>

class Customer; // Forward declaration

struct Wall {
    Vector3 position;
    Vector3 size;
    Color color;
};

struct CounterTable {
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
    
    // Colliders for physics
    std::vector<AABB> GetColliders() const;

    // Rack access and interaction
    std::vector<Rack>& GetRacks() { return racks; }
    const std::vector<Rack>& GetRacks() const { return racks; }

    // Find the rack player is aiming at within interaction range
    Rack* GetTargetedRack(Vector3 playerEyePos, Vector3 playerLookDir, float maxDistance = 3.5f);

    // Get front standing/browsing position for a given rack index
    Vector3 GetRackFrontPosition(size_t rackIndex) const;
    size_t GetRackCount() const { return racks.size(); }

    // Customer navigation: Find a rack that has stock > 0
    // Returns index of rack in racks vector, or -1 if no racks have stock
    int FindAvailableRackIndex(int preferredStartIndex = 0) const;

private:
    float shopWidth;
    float shopLength;
    float shopHeight;

    std::vector<Wall> walls;
    std::vector<CounterTable> counterTables;
    std::vector<Rack> racks;

    void BuildStructure();
    void BuildRacks();
};
