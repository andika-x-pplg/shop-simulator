#include "ShopExpansion.hpp"
#include "Finance.hpp"
#include "DailyStats.hpp"
#include "Shop.hpp"
#include <algorithm>

ShopExpansion& ShopExpansion::Instance() {
    static ShopExpansion instance;
    return instance;
}

ShopExpansion::ShopExpansion()
    : currentExpansionLevel(0),
      selectedIndex(0),
      menuOpen(false)
{
    Init();
}

void ShopExpansion::Init() {
    currentExpansionLevel = 0;
    selectedIndex = 0;
    menuOpen = false;
    expansions.clear();

    // Expansion Tier 1: Perluasan Sisi Kanan (Right Wing)
    // Price: Rp50.000 | +5m Width (Total 25x24m) | +2 Cust | +3 Furn | +2 Shelf Slot/Cap | +25 Storage
    ExpansionData e1;
    e1.id = 1;
    e1.name = "Perluasan Sisi Kanan (Right Wing)";
    e1.description = "Membuka sayap kanan toko seluas +5m, memberi ruang ekstra display & rak baru.";
    e1.tier = 1;
    e1.price = 50000;
    e1.requiredShopLevel = 1;
    e1.areaSizeDesc = "+5m Lebar Sisi Kanan (Total 25 x 24m)";
    e1.additionalCustomerCapacity = 2;
    e1.additionalFurnitureCapacity = 3;
    e1.additionalShelfCapacity = 2;
    e1.additionalStorageCapacity = 25;
    e1.zone = ShopZone::RIGHT_WING;
    e1.isPurchased = false;
    expansions.push_back(e1);

    // Expansion Tier 2: Perluasan Belakang (Back Wing)
    // Price: Rp100.000 | +4m Length (Total 25x28m) | +3 Cust | +4 Furn | +2 Shelf Slot/Cap | +35 Storage
    ExpansionData e2;
    e2.id = 2;
    e2.name = "Perluasan Belakang (Back Wing)";
    e2.description = "Memperluas area belakang toko seluas +4m, memperluas area gudang & lorong.";
    e2.tier = 2;
    e2.price = 100000;
    e2.requiredShopLevel = 1;
    e2.areaSizeDesc = "+4m Panjang Belakang (Total 25 x 28m)";
    e2.additionalCustomerCapacity = 3;
    e2.additionalFurnitureCapacity = 4;
    e2.additionalShelfCapacity = 2;
    e2.additionalStorageCapacity = 35;
    e2.zone = ShopZone::BACK_WING;
    e2.isPurchased = false;
    expansions.push_back(e2);

    // Expansion Tier 3: Perluasan Lantai Utama (Advanced Floor)
    // Price: Rp200.000 | +5m Width, +4m Length (Total 30x32m) | +4 Cust | +5 Furn | +3 Shelf Slot/Cap | +50 Storage
    ExpansionData e3;
    e3.id = 3;
    e3.name = "Perluasan Lantai Utama (Grand Hall)";
    e3.description = "Merombak toko menjadi Supermarket Megah (30x32m) dengan kapasitas maksimum.";
    e3.tier = 3;
    e3.price = 200000;
    e3.requiredShopLevel = 2;
    e3.areaSizeDesc = "+5m Lebar & +4m Panjang (Total 30 x 32m)";
    e3.additionalCustomerCapacity = 4;
    e3.additionalFurnitureCapacity = 5;
    e3.additionalShelfCapacity = 3;
    e3.additionalStorageCapacity = 50;
    e3.zone = ShopZone::MAIN_EXPANSION;
    e3.isPurchased = false;
    expansions.push_back(e3);
}

const ExpansionData& ShopExpansion::GetExpansion(int index) const {
    if (index >= 0 && index < (int)expansions.size()) {
        return expansions[index];
    }
    static ExpansionData dummy;
    return dummy;
}

void ShopExpansion::SetExpansionLevel(int level, Shop& shop) {
    currentExpansionLevel = std::max(0, std::min(level, (int)expansions.size()));
    for (size_t i = 0; i < expansions.size(); ++i) {
        expansions[i].isPurchased = ((int)i < currentExpansionLevel);
    }
    ApplyExpansionToShop(shop);
}

bool ShopExpansion::IsZoneUnlocked(ShopZone zone) const {
    if (zone == ShopZone::MAIN) return true;
    for (const auto& exp : expansions) {
        if (exp.zone == zone) {
            return exp.isPurchased;
        }
    }
    return false;
}

int ShopExpansion::GetUnlockedZoneCount() const {
    return 1 + currentExpansionLevel; // MAIN (1) + purchased tiers
}

int ShopExpansion::GetBonusCustomerCapacity() const {
    int total = 0;
    for (const auto& exp : expansions) {
        if (exp.isPurchased) {
            total += exp.additionalCustomerCapacity;
        }
    }
    return total;
}

int ShopExpansion::GetBonusStorageCapacity() const {
    int total = 0;
    for (const auto& exp : expansions) {
        if (exp.isPurchased) {
            total += exp.additionalStorageCapacity;
        }
    }
    return total;
}

int ShopExpansion::GetBonusShelfStockCapacity() const {
    int total = 0;
    for (const auto& exp : expansions) {
        if (exp.isPurchased) {
            total += exp.additionalShelfCapacity;
        }
    }
    return total;
}

int ShopExpansion::GetBonusFurnitureCapacity() const {
    int total = 0;
    for (const auto& exp : expansions) {
        if (exp.isPurchased) {
            total += exp.additionalFurnitureCapacity;
        }
    }
    return total;
}

bool ShopExpansion::PurchaseExpansion(int index, Finance& finance, Shop& shop, DailyStats& dailyStats, std::string& outFeedback) {
    if (index < 0 || index >= (int)expansions.size()) {
        outFeedback = "Expansion tidak valid!";
        return false;
    }

    auto& exp = expansions[index];
    if (exp.isPurchased) {
        outFeedback = "Expansion sudah dibeli!";
        return false;
    }

    // Must buy in sequential order
    if (index > currentExpansionLevel) {
        outFeedback = "Expansion belum dapat dibeli. Beli tier sebelumnya terlebih dahulu!";
        return false;
    }

    // Check shop level requirement
    if (shop.GetShopSizeLevel() < exp.requiredShopLevel) {
        outFeedback = "Expansion belum dapat dibeli! Membutuhkan Shop Upgrade Level " + std::to_string(exp.requiredShopLevel);
        return false;
    }

    if (finance.GetCurrentBalance() < exp.price) {
        outFeedback = "Tidak cukup uang. Butuh Rp" + std::to_string(exp.price) + " (Saldo: Rp" + std::to_string(finance.GetCurrentBalance()) + ")";
        return false;
    }

    // Process single financial deduction
    std::string desc = "Beli Expansion: " + exp.name;
    if (!finance.RecordExpense(exp.price, desc)) {
        outFeedback = "Gagal memproses transaksi expansion!";
        return false;
    }

    // Record to daily stats
    dailyStats.RecordExpense(exp.price);

    // Unlock
    exp.isPurchased = true;
    currentExpansionLevel = index + 1;

    // Apply physical expansion to 3D Shop
    ApplyExpansionToShop(shop);

    outFeedback = "Shop berhasil diperluas! Area baru (" + exp.name + ") telah dibuka.";
    return true;
}

void ShopExpansion::ApplyExpansionToShop(Shop& shop) {
    // Map expansion level to Shop physical dimensions
    // Level 0: 20x24m (Default)
    // Level 1: 25x24m (Right wing)
    // Level 2: 25x28m (Right & Back wing)
    // Level 3: 30x32m (Grand Hall)
    shop.SetExpansionDimensions(currentExpansionLevel);
}

void ShopExpansion::NextItem() {
    if (!expansions.empty()) {
        selectedIndex = (selectedIndex + 1) % expansions.size();
    }
}

void ShopExpansion::PreviousItem() {
    if (!expansions.empty()) {
        selectedIndex = (selectedIndex - 1 + (int)expansions.size()) % (int)expansions.size();
    }
}

void ShopExpansion::RenderUI(int screenWidth, int screenHeight, int currentBalance) {
    // Dim background
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 165 });

    int modalW = 760;
    int modalH = 530;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    DrawRectangle(modalX, modalY, modalW, modalH, { 22, 28, 38, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 46, 204, 113, 255 });

    // Header
    DrawText("PERLUASAN FISIK TOKO (SHOP EXPANSION)", modalX + 30, modalY + 18, 20, { 46, 204, 113, 255 });
    DrawText("Perbesar ukuran fisik toko 3D untuk menambah area belanja, rak, gudang, dan pelanggan",
             modalX + 30, modalY + 44, 12, { 180, 195, 210, 255 });

    // Status Summary Bar
    int sumY = modalY + 68;
    DrawRectangle(modalX + 30, sumY, modalW - 60, 42, { 30, 38, 50, 240 });
    DrawRectangleLines(modalX + 30, sumY, modalW - 60, 42, { 70, 85, 100, 255 });

    std::string expLvlText = "Expansion: Tier " + std::to_string(currentExpansionLevel) + " / " + std::to_string(expansions.size());
    DrawText(expLvlText.c_str(), modalX + 45, sumY + 13, 14, { 255, 215, 0, 255 });

    std::string zoneText = "Zona Terbuka: " + std::to_string(GetUnlockedZoneCount()) + " Zona";
    DrawText(zoneText.c_str(), modalX + 260, sumY + 13, 14, { 100, 220, 255, 255 });

    std::string balText = "Saldo: Rp" + std::to_string(currentBalance);
    DrawText(balText.c_str(), modalX + 500, sumY + 13, 14, { 50, 255, 120, 255 });

    // Expansion List Items
    int listY = modalY + 120;
    for (size_t i = 0; i < expansions.size(); ++i) {
        const auto& exp = expansions[i];
        bool isSel = (selectedIndex == (int)i);
        bool isUnlocked = exp.isPurchased;
        bool isAvailable = (!isUnlocked && (int)i == currentExpansionLevel);

        Color itemBg = isSel ? Color{ 35, 65, 95, 240 } : Color{ 26, 32, 42, 210 };
        Color itemBorder = isSel ? (isUnlocked ? Color{ 46, 204, 113, 255 } : Color{ 0, 220, 255, 255 }) : Color{ 55, 65, 78, 255 };

        DrawRectangle(modalX + 30, listY, modalW - 60, 95, itemBg);
        DrawRectangleLines(modalX + 30, listY, modalW - 60, 95, itemBorder);

        // Title & Tier
        std::string title = "[" + std::to_string(exp.tier) + "] " + exp.name + (isSel ? "  [DIPILIH]" : "");
        DrawText(title.c_str(), modalX + 45, listY + 10, 15, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

        // Area & description
        DrawText(exp.description.c_str(), modalX + 45, listY + 30, 12, { 180, 195, 210, 255 });

        // Benefits info
        std::string benefitStr = "Area: " + exp.areaSizeDesc + "  |  + " + std::to_string(exp.additionalCustomerCapacity) + " Cust  |  + " +
                                 std::to_string(exp.additionalFurnitureCapacity) + " Furn  |  + " +
                                 std::to_string(exp.additionalStorageCapacity) + " Storage";
        DrawText(benefitStr.c_str(), modalX + 45, listY + 50, 12, { 100, 220, 255, 255 });

        // Price & Status Badge
        int badgeX = modalX + modalW - 170;
        int badgeY = listY + 12;
        int badgeW = 125;
        int badgeH = 70;

        if (isUnlocked) {
            DrawRectangle(badgeX, badgeY, badgeW, badgeH, { 30, 80, 45, 240 });
            DrawRectangleLines(badgeX, badgeY, badgeW, badgeH, { 46, 204, 113, 255 });
            DrawText("TERBUKA", badgeX + 28, badgeY + 16, 13, { 100, 255, 150, 255 });
            DrawText("(PURCHASED)", badgeX + 16, badgeY + 38, 11, RAYWHITE);
        } else if (isAvailable) {
            bool canAfford = (currentBalance >= exp.price);
            Color availBg = canAfford ? Color{ 25, 90, 130, 240 } : Color{ 110, 40, 40, 240 };
            DrawRectangle(badgeX, badgeY, badgeW, badgeH, availBg);
            DrawRectangleLines(badgeX, badgeY, badgeW, badgeH, canAfford ? Color{ 52, 152, 219, 255 } : Color{ 231, 76, 60, 255 });
            std::string priceStr = "Rp" + std::to_string(exp.price);
            DrawText(priceStr.c_str(), badgeX + 14, badgeY + 14, 13, { 255, 215, 0, 255 });
            DrawText("[AVAILABLE]", badgeX + 22, badgeY + 38, 11, canAfford ? Color{ 50, 255, 120, 255 } : Color{ 255, 120, 120, 255 });
        } else {
            DrawRectangle(badgeX, badgeY, badgeW, badgeH, { 40, 45, 55, 240 });
            DrawRectangleLines(badgeX, badgeY, badgeW, badgeH, { 80, 90, 100, 255 });
            DrawText("TERKUNCI", badgeX + 26, badgeY + 16, 13, { 160, 170, 180, 255 });
            DrawText("[LOCKED]", badgeX + 28, badgeY + 38, 11, { 130, 140, 150, 255 });
        }

        listY += 105;
    }

    // Footer Controls
    int footerY = modalY + 450;
    DrawRectangle(modalX + 30, footerY, modalW - 60, 60, { 18, 22, 28, 240 });
    DrawRectangleLines(modalX + 30, footerY, modalW - 60, 60, { 70, 80, 90, 255 });

    DrawText("[W / S / Panah] Pilih Expansion    [ENTER] Beli Expansion    [X / ESC] Tutup Menu",
             modalX + 45, footerY + 12, 13, { 255, 220, 120, 255 });
    DrawText("Efek fisik 3D, perluasan dinding, lantai baru, rak tambahan, dan kapasitas langsung aktif.",
             modalX + 45, footerY + 34, 11, { 170, 185, 200, 255 });
}
