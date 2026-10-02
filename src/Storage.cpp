#include "Storage.hpp"
#include <algorithm>
#include <cmath>

Storage::Storage()
    : maxStorageCapacity(50)
{
}

void Storage::Init() {
    maxStorageCapacity = 50;
    for (auto type : GetAllProductTypes()) {
        storageStocks[type] = 0;
    }

    BuildStorageLayout();
    BuildColliders();
}

void Storage::BuildStorageLayout() {
    storagePallets.clear();

    // Storage Zone: located in the North-East / Back section
    // Pallet 1: Minuman / Air Mineral (DRK-001)
    storagePallets.push_back({ { 6.2f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::BEVERAGE, { 185, 122, 87, 255 } });

    // Pallet 2: Teh Botol (DRK-002)
    storagePallets.push_back({ { 4.6f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::TEA_BOTTLE, { 195, 125, 75, 255 } });

    // Pallet 3: Roti Tawar (FOD-001)
    storagePallets.push_back({ { 3.0f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::BREAD, { 205, 133, 63, 255 } });

    // Pallet 4: Makanan Kaleng (FOD-003)
    storagePallets.push_back({ { 1.4f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::CANNED_FOOD, { 160, 82, 45, 255 } });

    // Pallet 5: Sabun Mandi (HOU-001)
    storagePallets.push_back({ { -0.2f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::SOAP_BAR, { 140, 100, 60, 255 } });

    // Pallet 6: Biskuit Cokelat (SNK-001)
    storagePallets.push_back({ { -1.8f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::SNACK_BISCUIT, { 170, 115, 75, 255 } });

    // Pallet 7: Mie Instan (FOD-002)
    storagePallets.push_back({ { -3.4f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::INSTANT_NOODLE, { 190, 125, 80, 255 } });

    // Pallet 8: Tisu Wajah (HOU-002)
    storagePallets.push_back({ { -5.0f, 0.4f, -9.5f }, { 1.4f, 0.8f, 1.4f }, ProductType::TISSUE_PACK, { 175, 140, 95, 255 } });
}

void Storage::BuildColliders() {
    colliders.clear();
    for (const auto& box : storagePallets) {
        AABB aabb;
        aabb.min = { box.position.x - box.size.x / 2.0f, 0.0f, box.position.z - box.size.z / 2.0f };
        aabb.max = { box.position.x + box.size.x / 2.0f, box.position.y + box.size.y / 2.0f, box.position.z + box.size.z / 2.0f };
        colliders.push_back(aabb);
    }
}

std::vector<AABB> Storage::GetColliders() const {
    return colliders;
}

int Storage::GetStock(ProductType type) const {
    auto it = storageStocks.find(type);
    if (it != storageStocks.end()) {
        return it->second;
    }
    return 0;
}

void Storage::AddStock(ProductType type, int amount) {
    if (type != ProductType::NONE && amount > 0) {
        storageStocks[type] = std::min(storageStocks[type] + amount, maxStorageCapacity);
    }
}

bool Storage::TakeStock(ProductType type) {
    if (type != ProductType::NONE && storageStocks[type] > 0) {
        storageStocks[type]--;
        return true;
    }
    return false;
}

ProductType Storage::GetTargetedProduct(Vector3 playerPos, Vector3 playerLookDir, float maxDistance) {
    ProductType bestType = ProductType::NONE;
    float closestDist = maxDistance + 1.0f;

    Vector3 lookNorm = Vector3Normalize(playerLookDir);

    for (const auto& pallet : storagePallets) {
        Vector3 toPallet = Vector3Subtract(pallet.position, playerPos);
        float dist = Vector3Length(toPallet);

        if (dist <= maxDistance) {
            Vector3 toPalletDir = Vector3Normalize(toPallet);
            float dot = Vector3DotProduct(lookNorm, toPalletDir);
            if (dot > 0.65f && dist < closestDist) {
                closestDist = dist;
                bestType = pallet.productType;
            }
        }
    }

    return bestType;
}

void Storage::Render() {
    // 1. Storage Floor Demarcation / Yellow Hazard Striping
    DrawCube({ 0.6f, 0.01f, -9.5f }, 13.0f, 0.02f, 3.0f, { 230, 200, 50, 120 });
    DrawCubeWires({ 0.6f, 0.01f, -9.5f }, 13.0f, 0.02f, 3.0f, { 255, 215, 0, 255 });

    // 2. Storage Area Overhead Sign
    DrawCube({ 0.6f, 3.8f, -9.5f }, 4.5f, 0.5f, 0.15f, { 180, 100, 30, 255 });
    DrawCubeWires({ 0.6f, 3.8f, -9.5f }, 4.5f, 0.5f, 0.15f, RAYWHITE);

    // 3. Render Each Storage Pallet / Cargo Crates
    for (const auto& pallet : storagePallets) {
        int currentStock = GetStock(pallet.productType);
        ProductInfo pInfo = GetProductInfo(pallet.productType);

        // Wooden pallet base
        DrawCube(pallet.position, pallet.size.x, pallet.size.y, pallet.size.z, pallet.boxColor);
        DrawCubeWires(pallet.position, pallet.size.x, pallet.size.y, pallet.size.z, { 80, 50, 20, 255 });

        // Crate Label / Product Color Accent
        Vector3 labelPos = { pallet.position.x, pallet.position.y + pallet.size.y / 2.0f + 0.02f, pallet.position.z };
        DrawCube(labelPos, pallet.size.x * 0.85f, 0.04f, pallet.size.z * 0.85f, pInfo.primaryColor);

        // Stacked Storage Boxes visual representation when stock > 0
        if (currentStock > 0) {
            int visualBoxCount = std::min(currentStock / 3 + 1, 4);
            for (int b = 0; b < visualBoxCount; ++b) {
                Vector3 boxPos = { pallet.position.x, pallet.position.y + pallet.size.y / 2.0f + 0.35f + b * 0.45f, pallet.position.z };
                DrawCube(boxPos, pallet.size.x * 0.75f, 0.40f, pallet.size.z * 0.75f, { 210, 160, 110, 255 });
                DrawCubeWires(boxPos, pallet.size.x * 0.75f, 0.40f, pallet.size.z * 0.75f, { 100, 70, 40, 255 });
            }
        }
    }
}
