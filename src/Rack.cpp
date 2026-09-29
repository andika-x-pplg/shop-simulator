#include "Rack.hpp"
#include <algorithm>
#include <cmath>

Rack::Rack()
    : id(0),
      position{ 0.0f, 1.1f, 0.0f },
      size{ 2.0f, 2.2f, 5.0f },
      productType(ProductType::NONE),
      currentStock(0),
      maxStock(20),
      baseColor{ 130, 90, 60, 255 },
      trimColor{ 160, 120, 80, 255 }
{
}

Rack::Rack(int id, Vector3 position, Vector3 size, ProductType productType, int initialStock, int maxStock, Color baseColor, Color trimColor)
    : id(id),
      position(position),
      size(size),
      productType(productType),
      currentStock(initialStock),
      maxStock(maxStock),
      baseColor(baseColor),
      trimColor(trimColor)
{
}

std::string Rack::GetProductName() const {
    return GetProductInfo(productType).name;
}

AABB Rack::GetCollider() const {
    AABB box;
    box.min = { position.x - size.x / 2.0f, 0.0f, position.z - size.z / 2.0f };
    box.max = { position.x + size.x / 2.0f, position.y + size.y / 2.0f, position.z + size.z / 2.0f };
    return box;
}

bool Rack::TakeProduct() {
    if (currentStock > 0) {
        currentStock--;
        return true;
    }
    return false;
}

bool Rack::PlaceProduct(ProductType incomingType) {
    if (incomingType == productType && currentStock < maxStock) {
        currentStock++;
        return true;
    }
    return false;
}

bool Rack::IsPlayerLookingAt(Vector3 playerEyePos, Vector3 playerLookDir, float maxDistance) const {
    // Vector from player eye to rack center
    Vector3 toRack = Vector3Subtract(position, playerEyePos);
    float dist = Vector3Length(toRack);
    
    if (dist > maxDistance) {
        return false;
    }

    // Direction vector to rack
    Vector3 toRackDir = Vector3Normalize(toRack);
    Vector3 lookNorm = Vector3Normalize(playerLookDir);

    // Dot product to check angle (< 45 degrees cone)
    float dot = Vector3DotProduct(lookNorm, toRackDir);
    return dot > 0.65f;
}

void Rack::DrawProductItem(Vector3 itemPos, ProductType type) {
    ProductInfo info = GetProductInfo(type);
    
    if (type == ProductType::BEVERAGE) {
        // Beverage Can / Bottle: Blue body + silver top
        DrawCube(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, { 20, 80, 160, 255 });
        
        Vector3 capPos = { itemPos.x, itemPos.y + info.modelDimensions.y / 2.0f + 0.04f, itemPos.z };
        DrawCube(capPos, info.modelDimensions.x * 0.7f, 0.08f, info.modelDimensions.z * 0.7f, info.secondaryColor);
    }
    else if (type == ProductType::BREAD) {
        // Bread loaf: Golden crust body + lighter bread slit on top
        DrawCube(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, { 140, 90, 40, 255 });

        Vector3 topSlit = { itemPos.x, itemPos.y + info.modelDimensions.y / 2.0f + 0.02f, itemPos.z };
        DrawCube(topSlit, info.modelDimensions.x * 0.8f, 0.04f, info.modelDimensions.z * 0.4f, info.secondaryColor);
    }
    else if (type == ProductType::CANNED_FOOD) {
        // Canned food: Red can + silver rim top & bottom
        DrawCube(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x, info.modelDimensions.y, info.modelDimensions.z, { 150, 30, 30, 255 });

        Vector3 rimTop = { itemPos.x, itemPos.y + info.modelDimensions.y / 2.0f + 0.02f, itemPos.z };
        DrawCube(rimTop, info.modelDimensions.x * 0.9f, 0.04f, info.modelDimensions.z * 0.9f, info.secondaryColor);
    }
}

void Rack::RenderVisualProducts() {
    if (currentStock <= 0 || productType == ProductType::NONE) {
        return;
    }

    ProductInfo info = GetProductInfo(productType);

    // Shelf tier 1 (Middle shelf): Y height = position.y + 0.05f + info.modelDimensions.y / 2.0f
    // Shelf tier 2 (Top shelf): Y height = position.y + size.y / 2.0f + 0.05f + info.modelDimensions.y / 2.0f
    
    // We display items in a grid along Z-axis (length) and X-axis (width)
    // Max visual items on display proportional to stock
    int itemsToShow = std::min(currentStock, 8);

    float shelf1Y = position.y + 0.05f + info.modelDimensions.y / 2.0f;
    float shelf2Y = position.y + size.y / 2.0f + 0.05f + info.modelDimensions.y / 2.0f;

    // Arrange items on top shelf and middle shelf
    float zSpacing = (size.z * 0.7f) / 4.0f;
    float startZ = position.z - (size.z * 0.35f);

    for (int i = 0; i < itemsToShow; ++i) {
        float yPos = (i < 4) ? shelf2Y : shelf1Y;
        int indexOnShelf = i % 4;
        float zPos = startZ + indexOnShelf * zSpacing;
        
        // Slight offset for left/right item rows
        float xPosLeft = position.x - size.x * 0.22f;
        float xPosRight = position.x + size.x * 0.22f;

        Vector3 itemPos = (i % 2 == 0) ? Vector3{ xPosLeft, yPos, zPos } : Vector3{ xPosRight, yPos, zPos };
        DrawProductItem(itemPos, productType);
    }
}

void Rack::RenderShelves() {
    // Base rack body
    DrawCube(position, size.x, size.y, size.z, baseColor);
    DrawCubeWires(position, size.x, size.y, size.z, { 40, 40, 45, 255 });

    // Top shelf trim
    Vector3 topPos = { position.x, position.y + size.y / 2.0f - 0.05f, position.z };
    DrawCube(topPos, size.x + 0.1f, 0.1f, size.z + 0.1f, trimColor);

    // Middle shelf tier
    if (size.y > 1.5f) {
        Vector3 midPos = { position.x, position.y, position.z };
        DrawCube(midPos, size.x + 0.06f, 0.08f, size.z + 0.06f, trimColor);
    }

    // Label banner on top edge indicating product category
    Vector3 labelPos = { position.x, position.y + size.y / 2.0f + 0.35f, position.z };
    Color bannerColor = (productType == ProductType::BEVERAGE) ? Color{ 0, 102, 204, 255 } :
                        (productType == ProductType::BREAD) ? Color{ 180, 110, 40, 255 } :
                        (productType == ProductType::CANNED_FOOD) ? Color{ 180, 40, 40, 255 } : Color{ 80, 85, 90, 255 };
    
    DrawCube(labelPos, size.x * 0.85f, 0.35f, 0.15f, bannerColor);
    DrawCubeWires(labelPos, size.x * 0.85f, 0.35f, 0.15f, RAYWHITE);
}

void Rack::Render() {
    RenderShelves();
    RenderVisualProducts();
}
