#include "Furniture.hpp"
#include "Finance.hpp"
#include <algorithm>

Furniture::Furniture()
    : menuOpen(false),
      selectedTab(0),
      selectedIndex(0)
{
    Init();
}

void Furniture::Init() {
    menuOpen = false;
    selectedTab = 0;
    selectedIndex = 0;

    furnitureList.clear();
    // 1. Table (Meja Display Kayu) - Predefined spot in left-back lounge
    furnitureList.push_back({
        1, "Meja Kayu Santai (Table)", FurnitureType::TABLE, 50000, false,
        { -7.2f, 0.45f, -7.5f }, { 1.8f, 0.9f, 1.4f },
        { 139, 90, 43, 255 }, { 160, 110, 60, 255 },
        "Meja kayu estetik untuk mempercantik pojok toko"
    });

    // 2. Chair (Kursi Tunggu Pelanggan) - Predefined spot near table
    furnitureList.push_back({
        2, "Kursi Tunggu (Chair)", FurnitureType::CHAIR, 30000, false,
        { -7.2f, 0.45f, -5.8f }, { 0.8f, 0.9f, 0.8f },
        { 100, 60, 30, 255 }, { 180, 50, 50, 255 },
        "Kursi santai bagi pelanggan yang berkunjung"
    });

    // 3. Display Shelf (Rak Display Showcase Premium) - Predefined spot in right wall
    furnitureList.push_back({
        3, "Rak Showcase (Display Shelf)", FurnitureType::DISPLAY_SHELF, 100000, false,
        { 7.2f, 1.2f, 3.5f }, { 1.6f, 2.4f, 3.2f },
        { 45, 52, 60, 255 }, { 218, 165, 32, 255 },
        "Rak pajangan premium berornamen emas modern"
    });

    // 4. Cabinet (Lemari Arsip Toko) - Predefined spot behind cashier
    furnitureList.push_back({
        4, "Lemari Arsip (Cabinet)", FurnitureType::CABINET, 150000, false,
        { 7.5f, 1.25f, 9.8f }, { 1.6f, 2.5f, 1.2f },
        { 70, 75, 85, 255 }, { 200, 205, 215, 255 },
        "Lemari kabinet dokumen toko & perlengkapan kasir"
    });

    // 5. Decoration Plant (Tanaman Hias Toko) - Predefined spot at shop entrance lobby
    furnitureList.push_back({
        5, "Tanaman Hias (Decoration Plant)", FurnitureType::DECORATION_PLANT, 25000, false,
        { -2.8f, 0.6f, 10.5f }, { 0.8f, 1.2f, 0.8f },
        { 120, 70, 40, 255 }, { 34, 139, 34, 255 },
        "Pot tanaman indoor hijau segar di dekat pintu masuk"
    });

    equipmentList.clear();
    // 1. Better Display (+2 kapasitas rak toko)
    equipmentList.push_back({
        1, "Display Showcase Canggih (Better Display)", EquipmentType::BETTER_DISPLAY, 75000, false,
        "Menambah kapasitas pajangan seluruh rak sebesar +2 item"
    });

    // 2. Extra Storage Rack (+15 kapasitas gudang storage)
    equipmentList.push_back({
        2, "Pallet Tier Tambahan (Extra Storage Rack)", EquipmentType::EXTRA_STORAGE_RACK, 80000, false,
        "Menambah daya simpan storage gudang sebesar +15 item"
    });

    // 3. Better Cashier Equipment (Peralatan scanner kasir cepat)
    equipmentList.push_back({
        3, "Mesin Kasir Super Cepat (Better Cashier)", EquipmentType::BETTER_CASHIER, 120000, false,
        "Meningkatkan kepuasan customer saat proses pembayaran di kasir"
    });
}

bool Furniture::IsFurnitureOwned(FurnitureType type) const {
    for (const auto& item : furnitureList) {
        if (item.type == type) return item.isOwned;
    }
    return false;
}

bool Furniture::IsEquipmentOwned(EquipmentType type) const {
    for (const auto& item : equipmentList) {
        if (item.type == type) return item.isOwned;
    }
    return false;
}

int Furniture::GetEquipmentShelfBonus() const {
    return IsEquipmentOwned(EquipmentType::BETTER_DISPLAY) ? 2 : 0;
}

int Furniture::GetEquipmentStorageBonus() const {
    return IsEquipmentOwned(EquipmentType::EXTRA_STORAGE_RACK) ? 15 : 0;
}

bool Furniture::HasBetterCashierEquipment() const {
    return IsEquipmentOwned(EquipmentType::BETTER_CASHIER);
}

bool Furniture::PurchaseFurniture(int index, Finance& finance, std::string& outFeedback) {
    if (index < 0 || index >= (int)furnitureList.size()) return false;

    auto& item = furnitureList[index];
    if (item.isOwned) {
        outFeedback = item.name + " sudah dimiliki!";
        return false;
    }

    if (finance.GetCurrentBalance() < item.price) {
        outFeedback = "Saldo tidak cukup! Butuh Rp" + std::to_string(item.price) + " (Saldo: Rp" + std::to_string(finance.GetCurrentBalance()) + ")";
        return false;
    }

    std::string desc = "Beli Furniture: " + item.name;
    if (!finance.RecordExpense(item.price, desc)) {
        outFeedback = "Gagal memproses transaksi!";
        return false;
    }

    item.isOwned = true;
    outFeedback = "Furniture berhasil dibeli! " + item.name + " kini terpasang di toko.";
    return true;
}

bool Furniture::PurchaseEquipment(int index, Finance& finance, std::string& outFeedback) {
    if (index < 0 || index >= (int)equipmentList.size()) return false;

    auto& item = equipmentList[index];
    if (item.isOwned) {
        outFeedback = item.name + " sudah dimiliki!";
        return false;
    }

    if (finance.GetCurrentBalance() < item.price) {
        outFeedback = "Saldo tidak cukup! Butuh Rp" + std::to_string(item.price) + " (Saldo: Rp" + std::to_string(finance.GetCurrentBalance()) + ")";
        return false;
    }

    std::string desc = "Beli Equipment: " + item.name;
    if (!finance.RecordExpense(item.price, desc)) {
        outFeedback = "Gagal memproses transaksi!";
        return false;
    }

    item.isOwned = true;
    outFeedback = "Equipment berhasil dibeli! " + item.name + " aktif memberikan bonus.";
    return true;
}

void Furniture::NextItem() {
    int maxItems = (selectedTab == 0) ? (int)furnitureList.size() : (int)equipmentList.size();
    if (maxItems > 0) {
        selectedIndex = (selectedIndex + 1) % maxItems;
    }
}

void Furniture::PreviousItem() {
    int maxItems = (selectedTab == 0) ? (int)furnitureList.size() : (int)equipmentList.size();
    if (maxItems > 0) {
        selectedIndex = (selectedIndex - 1 + maxItems) % maxItems;
    }
}

std::vector<AABB> Furniture::GetColliders() const {
    std::vector<AABB> colliders;
    for (const auto& item : furnitureList) {
        if (item.isOwned) {
            AABB aabb;
            aabb.min = { item.position.x - item.size.x / 2.0f, 0.0f, item.position.z - item.size.z / 2.0f };
            aabb.max = { item.position.x + item.size.x / 2.0f, item.position.y + item.size.y / 2.0f, item.position.z + item.size.z / 2.0f };
            colliders.push_back(aabb);
        }
    }
    return colliders;
}

void Furniture::DrawTable(const FurnitureItem& item) {
    // Table Top
    Vector3 topPos = { item.position.x, item.position.y + item.size.y / 2.0f - 0.05f, item.position.z };
    DrawCube(topPos, item.size.x, 0.1f, item.size.z, item.secondaryColor);
    DrawCubeWires(topPos, item.size.x, 0.1f, item.size.z, { 60, 40, 20, 255 });

    // Table 4 Legs
    float legW = 0.12f;
    float legH = item.size.y - 0.1f;
    float xOff = item.size.x / 2.0f - 0.15f;
    float zOff = item.size.z / 2.0f - 0.15f;
    float legY = legH / 2.0f;

    DrawCube({ item.position.x - xOff, legY, item.position.z - zOff }, legW, legH, legW, item.primaryColor);
    DrawCube({ item.position.x + xOff, legY, item.position.z - zOff }, legW, legH, legW, item.primaryColor);
    DrawCube({ item.position.x - xOff, legY, item.position.z + zOff }, legW, legH, legW, item.primaryColor);
    DrawCube({ item.position.x + xOff, legY, item.position.z + zOff }, legW, legH, legW, item.primaryColor);
}

void Furniture::DrawChair(const FurnitureItem& item) {
    // Seat Cushion
    float seatH = 0.45f;
    Vector3 seatPos = { item.position.x, seatH, item.position.z };
    DrawCube(seatPos, item.size.x, 0.08f, item.size.z, item.secondaryColor);
    DrawCubeWires(seatPos, item.size.x, 0.08f, item.size.z, { 100, 30, 30, 255 });

    // Backrest (Facing +Z)
    Vector3 backPos = { item.position.x, seatH + 0.35f, item.position.z - item.size.z / 2.0f + 0.05f };
    DrawCube(backPos, item.size.x * 0.9f, 0.60f, 0.10f, item.secondaryColor);
    DrawCubeWires(backPos, item.size.x * 0.9f, 0.60f, 0.10f, { 80, 20, 20, 255 });

    // 4 Legs
    float legW = 0.08f;
    float xOff = item.size.x / 2.0f - 0.1f;
    float zOff = item.size.z / 2.0f - 0.1f;
    DrawCube({ item.position.x - xOff, seatH / 2.0f, item.position.z - zOff }, legW, seatH, legW, item.primaryColor);
    DrawCube({ item.position.x + xOff, seatH / 2.0f, item.position.z - zOff }, legW, seatH, legW, item.primaryColor);
    DrawCube({ item.position.x - xOff, seatH / 2.0f, item.position.z + zOff }, legW, seatH, legW, item.primaryColor);
    DrawCube({ item.position.x + xOff, seatH / 2.0f, item.position.z + zOff }, legW, seatH, legW, item.primaryColor);
}

void Furniture::DrawDisplayShelf(const FurnitureItem& item) {
    // Main Body Frame
    DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
    DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, { 25, 30, 35, 255 });

    // 3 Golden Shelf Plates
    for (int s = -1; s <= 1; ++s) {
        Vector3 platePos = { item.position.x, item.position.y + s * 0.7f, item.position.z };
        DrawCube(platePos, item.size.x + 0.06f, 0.06f, item.size.z + 0.06f, item.secondaryColor);
        DrawCubeWires(platePos, item.size.x + 0.06f, 0.06f, item.size.z + 0.06f, RAYWHITE);
    }
}

void Furniture::DrawCabinet(const FurnitureItem& item) {
    // Wooden / Metal Cabinet Body
    DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
    DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, { 35, 40, 45, 255 });

    // Front Drawer Dividers
    for (int d = -1; d <= 1; ++d) {
        Vector3 handlePos = { item.position.x, item.position.y + d * 0.65f, item.position.z - item.size.z / 2.0f - 0.02f };
        DrawCube(handlePos, 0.40f, 0.08f, 0.04f, item.secondaryColor);
    }
}

void Furniture::DrawPlant(const FurnitureItem& item) {
    // Terracotta Pot Base
    float potH = 0.5f;
    Vector3 potPos = { item.position.x, potH / 2.0f, item.position.z };
    DrawCube(potPos, item.size.x, potH, item.size.z, item.primaryColor);
    DrawCubeWires(potPos, item.size.x, potH, item.size.z, { 80, 40, 20, 255 });

    // Foliage Spheres / Bushes
    Vector3 bushPos = { item.position.x, potH + 0.35f, item.position.z };
    DrawSphere(bushPos, 0.45f, item.secondaryColor);
    DrawSphereWires(bushPos, 0.45f, 6, 6, { 20, 90, 20, 255 });

    Vector3 topBushPos = { item.position.x, potH + 0.65f, item.position.z };
    DrawSphere(topBushPos, 0.30f, { 46, 204, 113, 255 });
}

void Furniture::Render() {
    for (const auto& item : furnitureList) {
        if (!item.isOwned) continue;

        switch (item.type) {
            case FurnitureType::TABLE:
                DrawTable(item);
                break;
            case FurnitureType::CHAIR:
                DrawChair(item);
                break;
            case FurnitureType::DISPLAY_SHELF:
                DrawDisplayShelf(item);
                break;
            case FurnitureType::CABINET:
                DrawCabinet(item);
                break;
            case FurnitureType::DECORATION_PLANT:
                DrawPlant(item);
                break;
        }
    }
}
