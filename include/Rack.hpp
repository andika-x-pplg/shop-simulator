#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>

class Rack {
public:
    Rack();
    Rack(int id, Vector3 position, Vector3 size, ProductType productType, int initialStock, int maxStock, Color baseColor, Color trimColor);
    ~Rack() = default;

    void Render();
    
    // Collision & Interaction
    AABB GetCollider() const;
    Vector3 GetPosition() const { return position; }
    Vector3 GetSize() const { return size; }
    int GetId() const { return id; }

    // Product & Stock management
    ProductType GetProductType() const { return productType; }
    std::string GetProductName() const;
    int GetStock() const { return currentStock; }
    int GetMaxStock() const { return maxStock; }
    void SetMaxStock(int newMax) { if (newMax > 0) maxStock = newMax; }
    bool HasStock() const { return currentStock > 0; }
    bool IsFull() const { return currentStock >= maxStock; }

    bool TakeProduct();
    bool PlaceProduct(ProductType incomingType);

    // Interaction test: checks distance and angle from player camera
    bool IsPlayerLookingAt(Vector3 playerEyePos, Vector3 playerLookDir, float maxDistance = 3.5f) const;

private:
    int id;
    Vector3 position;
    Vector3 size;
    ProductType productType;
    int currentStock;
    int maxStock;
    Color baseColor;
    Color trimColor;

    void RenderShelves();
    void RenderVisualProducts();
    void DrawProductItem(Vector3 itemPos, ProductType type);
};
