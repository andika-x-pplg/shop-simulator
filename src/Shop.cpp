#include "Shop.hpp"

Shop::Shop()
    : shopSizeLevel(1),
      shopWidth(20.0f), shopLength(24.0f), shopHeight(5.0f),
      cashier({ 5.5f, 0.6f, 8.5f }, { 3.5f, 1.2f, 1.6f })
{
}

void Shop::Init() {
    shopSizeLevel = 1;
    shopWidth = 20.0f;
    shopLength = 24.0f;
    BuildStructure();
    BuildRacks();
    cashier.Init();
    storage.Init();
}

void Shop::SetShopSizeLevel(int level) {
    shopSizeLevel = level;
    if (level == 1) {
        shopWidth = 20.0f;
        shopLength = 24.0f;
    } else if (level == 2) {
        shopWidth = 25.0f;
        shopLength = 28.0f;
    } else if (level >= 3) {
        shopWidth = 30.0f;
        shopLength = 32.0f;
    }
    BuildStructure();
}

void Shop::Update(float deltaTime) {
    cashier.Update(deltaTime);
}

void Shop::BuildStructure() {
    walls.clear();

    float halfW = shopWidth / 2.0f;
    float halfL = shopLength / 2.0f;
    float wallThickness = 0.5f;

    Color wallColor = { 220, 225, 230, 255 };      // Soft light gray/white interior
    Color backWallColor = { 200, 210, 220, 255 };

    // North Wall (Back)
    walls.push_back({ { 0.0f, shopHeight / 2.0f, -halfL }, { shopWidth, shopHeight, wallThickness }, backWallColor });

    // West Wall (Left)
    walls.push_back({ { -halfW, shopHeight / 2.0f, 0.0f }, { wallThickness, shopHeight, shopLength }, wallColor });

    // East Wall (Right)
    walls.push_back({ { halfW, shopHeight / 2.0f, 0.0f }, { wallThickness, shopHeight, shopLength }, wallColor });

    // South Wall (Front) with Door opening in the center (4.0m wide from -2.0 to 2.0)
    float sideWallWidth = (shopWidth - 4.0f) / 2.0f;
    float leftWallX = -halfW + sideWallWidth / 2.0f;
    float rightWallX = halfW - sideWallWidth / 2.0f;

    // South-Left Wall
    walls.push_back({ { leftWallX, shopHeight / 2.0f, halfL }, { sideWallWidth, shopHeight, wallThickness }, wallColor });
    // South-Right Wall
    walls.push_back({ { rightWallX, shopHeight / 2.0f, halfL }, { sideWallWidth, shopHeight, wallThickness }, wallColor });
    // Top Door Frame
    walls.push_back({ { 0.0f, shopHeight - 0.6f, halfL }, { 4.0f, 1.2f, wallThickness }, { 100, 110, 120, 255 } });
}

void Shop::BuildRacks() {
    racks.clear();

    // Rack 1: Left-Back Aisle -> Minuman (Beverage), initial stock = 2, max stock = 10
    racks.emplace_back(1, Vector3{ -5.5f, 1.1f, -4.0f }, Vector3{ 2.0f, 2.2f, 5.5f },
                       ProductType::BEVERAGE, 2, 10,
                       Color{ 110, 130, 150, 255 }, Color{ 140, 170, 200, 255 });

    // Rack 2: Left-Front Aisle -> Roti (Bread), initial stock = 5, max stock = 10
    racks.emplace_back(2, Vector3{ -5.5f, 1.1f, 3.5f }, Vector3{ 2.0f, 2.2f, 5.5f },
                       ProductType::BREAD, 5, 10,
                       Color{ 140, 110, 80, 255 }, Color{ 180, 140, 100, 255 });

    // Rack 3: Right Aisle -> Makanan Kaleng (Canned Food), initial stock = 3, max stock = 10
    racks.emplace_back(3, Vector3{ 5.5f, 1.1f, -2.5f }, Vector3{ 2.0f, 2.2f, 7.0f },
                       ProductType::CANNED_FOOD, 3, 10,
                       Color{ 130, 90, 90, 255 }, Color{ 170, 120, 120, 255 });

    // Rack 4: Center Island Display Table -> Minuman promo (initial stock = 2, max stock = 10)
    racks.emplace_back(4, Vector3{ 0.0f, 0.7f, -6.5f }, Vector3{ 3.0f, 1.4f, 4.0f },
                       ProductType::BEVERAGE, 2, 10,
                       Color{ 60, 100, 140, 255 }, Color{ 90, 140, 190, 255 });
}

Vector3 Shop::GetRackFrontPosition(size_t rackIndex) const {
    if (rackIndex >= racks.size()) return { 0.0f, 0.0f, 0.0f };

    Vector3 rackPos = racks[rackIndex].GetPosition();
    
    // Standing spot in the aisle facing the rack
    if (rackPos.x < -2.0f) {
        // Left aisle racks: standing spot is inside the aisle (x = -3.2f)
        return { -3.2f, 0.0f, rackPos.z };
    } else if (rackPos.x > 2.0f) {
        // Right aisle racks: standing spot is inside the aisle (x = 3.2f)
        return { 3.2f, 0.0f, rackPos.z };
    } else {
        // Center island: standing spot clearly in front of the island table (z = -3.6f)
        return { 0.0f, 0.0f, -3.6f };
    }
}

int Shop::FindAvailableRackIndex(int preferredStartIndex) const {
    if (racks.empty()) return -1;

    size_t count = racks.size();
    size_t start = (preferredStartIndex >= 0) ? ((size_t)preferredStartIndex % count) : 0;

    // Check starting from preferred index in loop
    for (size_t i = 0; i < count; ++i) {
        size_t idx = (start + i) % count;
        if (racks[idx].HasStock()) {
            return (int)idx;
        }
    }
    return -1;
}

int Shop::FindRackWithProduct(ProductType type) const {
    for (size_t i = 0; i < racks.size(); ++i) {
        if (racks[i].GetProductType() == type && racks[i].HasStock()) {
            return (int)i;
        }
    }
    return -1;
}

std::vector<AABB> Shop::GetColliders() const {
    std::vector<AABB> colliders;

    // Wall colliders
    for (const auto& w : walls) {
        AABB box;
        box.min = { w.position.x - w.size.x / 2.0f, 0.0f, w.position.z - w.size.z / 2.0f };
        box.max = { w.position.x + w.size.x / 2.0f, w.position.y + w.size.y / 2.0f, w.position.z + w.size.z / 2.0f };
        colliders.push_back(box);
    }

    // Cashier counter collider
    colliders.push_back(cashier.GetCollider());

    // Storage pallets collider
    auto storageColliders = storage.GetColliders();
    colliders.insert(colliders.end(), storageColliders.begin(), storageColliders.end());

    // Rack colliders
    for (const auto& r : racks) {
        colliders.push_back(r.GetCollider());
    }

    return colliders;
}

Rack* Shop::GetTargetedRack(Vector3 playerEyePos, Vector3 playerLookDir, float maxDistance) {
    Rack* bestRack = nullptr;
    float closestDist = maxDistance + 1.0f;

    for (auto& r : racks) {
        if (r.IsPlayerLookingAt(playerEyePos, playerLookDir, maxDistance)) {
            float d = Vector3Distance(playerEyePos, r.GetPosition());
            if (d < closestDist) {
                closestDist = d;
                bestRack = &r;
            }
        }
    }

    return bestRack;
}

void Shop::Render() {
    // Floor
    DrawPlane({ 0.0f, 0.0f, 0.0f }, { shopWidth + 8.0f, shopLength + 8.0f }, { 190, 195, 190, 255 }); // Outside grass/ground
    DrawCube({ 0.0f, -0.05f, 0.0f }, shopWidth, 0.1f, shopLength, { 245, 245, 250, 255 }); // Shop clean tile floor
    DrawGrid((int)(shopLength / 2), 2.0f); // Tile grid lines

    // Subtle Ground Shadows beneath major fixtures
    DrawCube({ 5.5f, 0.01f, 8.5f }, 3.7f, 0.02f, 1.8f, { 0, 0, 0, 45 }); // Cashier counter shadow
    DrawCube({ 3.2f, 0.01f, -9.5f }, 6.8f, 0.02f, 2.2f, { 0, 0, 0, 45 }); // Storage pallet shadow

    // Ceiling & Ceiling Panels
    DrawCube({ 0.0f, shopHeight + 0.05f, 0.0f }, shopWidth, 0.1f, shopLength, { 210, 215, 220, 255 });

    // Ceiling Light Panels (Illuminated 3D light strips)
    float lightSpacing = 6.0f;
    for (float z = -shopLength / 2.0f + 4.0f; z < shopLength / 2.0f - 2.0f; z += lightSpacing) {
        // Left light panel
        DrawCube({ -4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 255, 255, 240, 255 });
        DrawCubeWires({ -4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 200, 200, 180, 255 });

        // Right light panel
        DrawCube({ 4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 255, 255, 240, 255 });
        DrawCubeWires({ 4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 200, 200, 180, 255 });
    }

    // Render Walls
    for (const auto& w : walls) {
        DrawCube(w.position, w.size.x, w.size.y, w.size.z, w.color);
        DrawCubeWires(w.position, w.size.x, w.size.y, w.size.z, { 80, 85, 95, 255 });
    }

    // Modern Glass Door & Frame
    DrawCube({ -2.0f, 1.8f, shopLength / 2.0f }, 0.2f, 3.6f, 0.6f, { 60, 65, 70, 255 });
    DrawCube({ 2.0f, 1.8f, shopLength / 2.0f }, 0.2f, 3.6f, 0.6f, { 60, 65, 70, 255 });
    DrawCube({ 0.0f, 3.5f, shopLength / 2.0f }, 4.0f, 0.2f, 0.6f, { 60, 65, 70, 255 });
    // Translucent Glass Panels
    DrawCube({ -1.0f, 1.7f, shopLength / 2.0f }, 1.8f, 3.2f, 0.05f, { 180, 230, 255, 120 });
    DrawCube({ 1.0f, 1.7f, shopLength / 2.0f }, 1.8f, 3.2f, 0.05f, { 180, 230, 255, 120 });

    // Store Entrance Welcome Mat
    DrawCube({ 0.0f, 0.01f, shopLength / 2.0f - 1.2f }, 3.2f, 0.02f, 1.6f, { 192, 57, 43, 255 }); // Red welcome carpet
    DrawCubeWires({ 0.0f, 0.01f, shopLength / 2.0f - 1.2f }, 3.2f, 0.02f, 1.6f, { 241, 196, 15, 255 });

    // Render Cashier Counter & POS System
    cashier.Render();

    // Render Storage Warehouse Area
    storage.Render();

    // Render all Racks and their visual products
    for (auto& r : racks) {
        r.Render();
    }
}
