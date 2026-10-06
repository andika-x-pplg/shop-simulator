#include "Shop.hpp"
#include "ShopCustomization.hpp"
#include <algorithm>

Shop::Shop()
    : shopSizeLevel(1),
      expansionTier(0),
      shopWidth(20.0f), shopLength(24.0f), shopHeight(5.0f),
      cashier({ 5.5f, 0.6f, 8.5f }, { 3.5f, 1.2f, 1.6f })
{
}

void Shop::Init() {
    shopSizeLevel = 1;
    expansionTier = 0;
    shopWidth = 20.0f;
    shopLength = 24.0f;
    BuildStructure();
    BuildRacks();
    cashier.Init();
    storage.Init();
}

void Shop::SetShopSizeLevel(int level) {
    shopSizeLevel = level;
    // Calculate combined effective dimensions
    SetExpansionDimensions(expansionTier);
}

void Shop::SetExpansionDimensions(int tier) {
    expansionTier = std::max(0, std::min(tier, 3));

    // Base dimensions depending on Shop Size Level and Expansion Tier
    // Default Level 1: 20x24m
    // Tier 1 (Right Wing): +5m Width (25x24m)
    // Tier 2 (Back Wing): +5m Width, +4m Length (25x28m)
    // Tier 3 (Grand Hall): +10m Width, +8m Length (30x32m)
    if (expansionTier == 0) {
        if (shopSizeLevel == 1) {
            shopWidth = 20.0f;
            shopLength = 24.0f;
        } else if (shopSizeLevel == 2) {
            shopWidth = 25.0f;
            shopLength = 28.0f;
        } else {
            shopWidth = 30.0f;
            shopLength = 32.0f;
        }
    } else if (expansionTier == 1) {
        shopWidth = (shopSizeLevel >= 3) ? 30.0f : 25.0f;
        shopLength = (shopSizeLevel >= 2) ? 28.0f : 24.0f;
    } else if (expansionTier == 2) {
        shopWidth = (shopSizeLevel >= 3) ? 30.0f : 25.0f;
        shopLength = 28.0f;
    } else if (expansionTier >= 3) {
        shopWidth = 30.0f;
        shopLength = 32.0f;
    }

    BuildStructure();
    BuildRacks();
}

void Shop::Update(float deltaTime) {
    cashier.Update(deltaTime);
}

void Shop::BuildStructure() {
    walls.clear();

    float halfW = shopWidth / 2.0f;
    float halfL = shopLength / 2.0f;
    float wallThickness = 0.5f;

    Color wallColor = ShopCustomization::Instance().GetWallPrimaryColor();
    Color backWallColor = ShopCustomization::Instance().GetWallSecondaryColor();

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
    // Preserve existing stocks when rebuilding/expanding racks
    std::vector<int> previousStocks;
    for (const auto& r : racks) {
        previousStocks.push_back(r.GetStock());
    }

    racks.clear();

    // Default Main Area Racks (1 to 6)
    // Rack 1: Left-Back Aisle -> Minuman (Air Mineral / DRK-001)
    racks.emplace_back(1, Vector3{ -5.5f, 1.1f, -4.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                       ProductType::BEVERAGE, 4, 10,
                       Color{ 100, 130, 160, 255 }, Color{ 130, 170, 210, 255 });

    // Rack 2: Left-Front Aisle -> Roti Tawar (FOD-001)
    racks.emplace_back(2, Vector3{ -5.5f, 1.1f, 3.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                       ProductType::BREAD, 5, 10,
                       Color{ 140, 110, 80, 255 }, Color{ 180, 140, 100, 255 });

    // Rack 3: Right-Back Aisle -> Makanan Kaleng (FOD-003)
    racks.emplace_back(3, Vector3{ 5.5f, 1.1f, -4.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                       ProductType::CANNED_FOOD, 3, 10,
                       Color{ 140, 80, 80, 255 }, Color{ 180, 110, 110, 255 });

    // Rack 4: Right-Front Aisle -> Sabun Mandi (HOU-001)
    racks.emplace_back(4, Vector3{ 5.5f, 1.1f, 3.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                       ProductType::SOAP_BAR, 4, 10,
                       Color{ 60, 140, 120, 255 }, Color{ 90, 180, 150, 255 });

    // Rack 5: Center-Front Display -> Camilan / Biskuit Cokelat (SNK-001)
    racks.emplace_back(5, Vector3{ 0.0f, 0.8f, -2.5f }, Vector3{ 2.8f, 1.6f, 3.2f },
                       ProductType::SNACK_BISCUIT, 4, 10,
                       Color{ 150, 100, 40, 255 }, Color{ 200, 150, 70, 255 });

    // Rack 6: Center-Back Display -> Mie Instan (FOD-002)
    racks.emplace_back(6, Vector3{ 0.0f, 0.8f, -7.0f }, Vector3{ 2.8f, 1.6f, 3.2f },
                       ProductType::INSTANT_NOODLE, 4, 10,
                       Color{ 160, 130, 40, 255 }, Color{ 210, 170, 60, 255 });

    // Stage 19: Additional Racks in Expansion Zones
    // Tier 1 (Right Wing Unlocked): Rack 7 in Right Wing Expansion -> Teh Botol (DRK-002)
    if (expansionTier >= 1 || shopWidth >= 25.0f) {
        racks.emplace_back(7, Vector3{ 9.5f, 1.1f, -4.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                           ProductType::TEA_BOTTLE, 4, 10,
                           Color{ 180, 90, 30, 255 }, Color{ 220, 140, 60, 255 });
    }

    // Tier 2 (Back Wing Unlocked): Rack 8 in Back Wing Expansion -> Tisu Wajah (HOU-002)
    if (expansionTier >= 2 || shopLength >= 28.0f) {
        racks.emplace_back(8, Vector3{ -5.5f, 1.1f, -10.5f }, Vector3{ 2.0f, 2.2f, 4.0f },
                           ProductType::TISSUE_PACK, 4, 10,
                           Color{ 120, 140, 180, 255 }, Color{ 170, 190, 220, 255 });
    }

    // Tier 3 (Grand Hall Unlocked): Rack 9 & 10 in Grand Hall Aisles -> Extra Beverage & Snack
    if (expansionTier >= 3 || (shopWidth >= 30.0f && shopLength >= 32.0f)) {
        racks.emplace_back(9, Vector3{ 9.5f, 1.1f, 3.5f }, Vector3{ 2.0f, 2.2f, 4.5f },
                           ProductType::BEVERAGE, 5, 10,
                           Color{ 70, 130, 180, 255 }, Color{ 100, 170, 230, 255 });
        racks.emplace_back(10, Vector3{ 0.0f, 0.8f, -11.5f }, Vector3{ 2.8f, 1.6f, 3.2f },
                           ProductType::SNACK_BISCUIT, 5, 10,
                           Color{ 160, 110, 50, 255 }, Color{ 210, 160, 80, 255 });
    }

    // Restore stocks if they existed
    for (size_t i = 0; i < racks.size() && i < previousStocks.size(); ++i) {
        racks[i].SetStock(previousStocks[i]);
    }
}

Vector3 Shop::GetRackFrontPosition(size_t rackIndex) const {
    if (rackIndex >= racks.size()) return { 0.0f, 0.0f, 0.0f };

    Vector3 rackPos = racks[rackIndex].GetPosition();
    
    // Standing spot in the aisle facing the rack
    if (rackPos.x < -2.0f) {
        // Left aisle racks: standing spot is inside the aisle (x = -3.2f)
        return { rackPos.x + 2.3f, 0.0f, rackPos.z };
    } else if (rackPos.x > 2.0f) {
        // Right aisle racks: standing spot is inside the aisle
        return { rackPos.x - 2.3f, 0.0f, rackPos.z };
    } else {
        // Center island tables: standing spot on front side (+Z)
        return { 0.0f, 0.0f, rackPos.z + 2.2f };
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

int Shop::GetProductStockOnShelves(ProductType type) const {
    int total = 0;
    for (const auto& r : racks) {
        if (r.GetProductType() == type) {
            total += r.GetStock();
        }
    }
    return total;
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
    Color floorCol = ShopCustomization::Instance().GetFloorPrimaryColor();
    Color gridCol = ShopCustomization::Instance().GetFloorGridColor();
    Color lightCol = ShopCustomization::Instance().GetCeilingLightColor();

    DrawCube({ 0.0f, -0.05f, 0.0f }, shopWidth, 0.1f, shopLength, floorCol); // Customizable floor
    DrawGrid((int)(shopLength / 2), 2.0f); // Tile grid lines

    // Highlight Expansion Zones on floor when unlocked
    if (expansionTier >= 1 || shopWidth >= 25.0f) {
        // Right wing zone subtle aesthetic border
        DrawCube({ (shopWidth / 2.0f) - 2.5f, 0.005f, 0.0f }, 5.0f, 0.01f, shopLength - 1.0f, { 230, 245, 255, 60 });
    }
    if (expansionTier >= 2 || shopLength >= 28.0f) {
        // Back wing zone subtle aesthetic border
        DrawCube({ 0.0f, 0.005f, (-shopLength / 2.0f) + 2.0f }, shopWidth - 1.0f, 0.01f, 4.0f, { 240, 255, 240, 60 });
    }

    // Subtle Ground Shadows beneath major fixtures
    DrawCube({ 5.5f, 0.01f, 8.5f }, 3.7f, 0.02f, 1.8f, { 0, 0, 0, 45 }); // Cashier counter shadow
    DrawCube({ 3.2f, 0.01f, -9.5f }, 6.8f, 0.02f, 2.2f, { 0, 0, 0, 45 }); // Storage pallet shadow

    // Ceiling & Ceiling Panels
    DrawCube({ 0.0f, shopHeight + 0.05f, 0.0f }, shopWidth, 0.1f, shopLength, { 210, 215, 220, 255 });

    // Ceiling Light Panels (Illuminated 3D light strips across aisles)
    float lightSpacing = 6.0f;
    for (float z = -shopLength / 2.0f + 4.0f; z < shopLength / 2.0f - 2.0f; z += lightSpacing) {
        // Left light panel
        DrawCube({ -4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, lightCol);
        DrawCubeWires({ -4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 200, 200, 180, 255 });

        // Right light panel
        DrawCube({ 4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, lightCol);
        DrawCubeWires({ 4.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 200, 200, 180, 255 });

        // Extra Right Wing Light Panels if expanded
        if (shopWidth >= 25.0f) {
            DrawCube({ 9.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, lightCol);
            DrawCubeWires({ 9.5f, shopHeight - 0.08f, z }, 1.2f, 0.12f, 2.8f, { 200, 200, 180, 255 });
        }
    }

    // Render Walls
    for (const auto& w : walls) {
        DrawCube(w.position, w.size.x, w.size.y, w.size.z, w.color);
        DrawCubeWires(w.position, w.size.x, w.size.y, w.size.z, { 80, 85, 95, 255 });
    }

    // Modern Glass Door & Frame at South Wall
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

    // Render 3D Storefront Signboard
    ShopCustomization::Instance().RenderSignboard(shopWidth, shopLength, shopHeight);
}
