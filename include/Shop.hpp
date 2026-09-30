#pragma once
#include "Common.hpp"
#include "Rack.hpp"
#include "Cashier.hpp"
#include "Storage.hpp"
#include <vector>
#include <memory>

class Customer;

struct Wall {
    Vector3 position;
    Vector3 size;
    Color color;
};

class Shop {
public:
    Shop();
    ~Shop() = default;

    void Init();
    void Update(float deltaTime);
    void Render();
    
    // Colliders for physics
    std::vector<AABB> GetColliders() const;

    // Rack access and interaction
    std::vector<Rack>& GetRacks() { return racks; }
    const std::vector<Rack>& GetRacks() const { return racks; }

    // Cashier access
    Cashier& GetCashier() { return cashier; }
    const Cashier& GetCashier() const { return cashier; }

    // Storage access
    Storage& GetStorage() { return storage; }
    const Storage& GetStorage() const { return storage; }

    // Find the rack player is aiming at within interaction range
    Rack* GetTargetedRack(Vector3 playerEyePos, Vector3 playerLookDir, float maxDistance = 3.5f);

    // Get front standing/browsing position for a given rack index
    Vector3 GetRackFrontPosition(size_t rackIndex) const;
    size_t GetRackCount() const { return racks.size(); }

    // Customer navigation: Find a rack that has stock > 0
    int FindAvailableRackIndex(int preferredStartIndex = 0) const;
    int FindRackWithProduct(ProductType type) const;
    int GetProductStockOnShelves(ProductType type) const;

    // Shop Upgrade: Size expansion
    int GetShopSizeLevel() const { return shopSizeLevel; }
    void SetShopSizeLevel(int level);
    float GetShopWidth() const { return shopWidth; }
    float GetShopLength() const { return shopLength; }

private:
    int shopSizeLevel;
    float shopWidth;
    float shopLength;
    float shopHeight;

    std::vector<Wall> walls;
    Cashier cashier;
    Storage storage;
    std::vector<Rack> racks;

    void BuildStructure();
    void BuildRacks();
};
