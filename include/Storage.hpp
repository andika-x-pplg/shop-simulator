#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <map>
#include <string>
#include <vector>

struct StorageBox {
    Vector3 position;
    Vector3 size;
    ProductType productType;
    Color boxColor;
};

class Storage {
public:
    Storage();
    ~Storage() = default;

    void Init();
    void Render();

    // Collision
    std::vector<AABB> GetColliders() const;

    // Stock Management
    int GetStock(ProductType type) const;
    void AddStock(ProductType type, int amount);
    bool TakeStock(ProductType type);

    // Check interaction with storage zones
    ProductType GetTargetedProduct(Vector3 playerPos, Vector3 playerLookDir, float maxDistance = 3.5f);

    // Quick stock queries for UI/HUD
    int GetBeverageStock() const { return GetStock(ProductType::BEVERAGE); }
    int GetBreadStock() const { return GetStock(ProductType::BREAD); }
    int GetCannedFoodStock() const { return GetStock(ProductType::CANNED_FOOD); }
    int GetTotalStock() const { return GetBeverageStock() + GetBreadStock() + GetCannedFoodStock(); }
    int GetMaxCapacity() const { return maxStorageCapacity; }
    void SetMaxCapacity(int newMax) { if (newMax > 0) maxStorageCapacity = newMax; }
    bool IsFull(ProductType type) const { return GetStock(type) >= maxStorageCapacity; }

private:
    int maxStorageCapacity;
    std::map<ProductType, int> storageStocks;
    std::vector<StorageBox> storagePallets;
    std::vector<AABB> colliders;

    void BuildStorageLayout();
    void BuildColliders();
};
