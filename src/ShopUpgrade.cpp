#include "ShopUpgrade.hpp"
#include "Finance.hpp"
#include "Shop.hpp"
#include "Storage.hpp"
#include <algorithm>

ShopUpgrade::ShopUpgrade()
    : selectedUpgradeIndex(0),
      menuOpen(false)
{
    Init();
}

void ShopUpgrade::Init() {
    selectedUpgradeIndex = 0;
    menuOpen = false;
    upgrades.clear();

    // 1. Shop Size (Level 1: 20x24m, Level 2: 25x28m, Level 3: 30x32m)
    UpgradeInfo sizeUp;
    sizeUp.type = UpgradeType::SHOP_SIZE;
    sizeUp.name = "Ukuran Toko (Shop Size)";
    sizeUp.description = "Memperluas area toko agar lebih luas & lega";
    sizeUp.currentLevel = 1;
    sizeUp.maxLevel = 3;
    sizeUp.costs = { 50000, 100000 }; // Lvl 1->2: Rp50.000, Lvl 2->3: Rp100.000
    sizeUp.levelDescriptions = { "Level 1 (20x24m)", "Level 2 (25x28m)", "Level 3 (30x32m)" };
    upgrades.push_back(sizeUp);

    // 2. Shelf Capacity (Level 1: 10 item, Level 2: 16 item, Level 3: 24 item)
    UpgradeInfo shelfUp;
    shelfUp.type = UpgradeType::SHELF_CAPACITY;
    shelfUp.name = "Kapasitas Rak (Shelf Cap)";
    shelfUp.description = "Meningkatkan daya tampung produk pada setiap rak";
    shelfUp.currentLevel = 1;
    shelfUp.maxLevel = 3;
    shelfUp.costs = { 35000, 75000 }; // Lvl 1->2: Rp35.000, Lvl 2->3: Rp75.000
    shelfUp.levelDescriptions = { "10 Item / Rak", "16 Item / Rak", "24 Item / Rak" };
    upgrades.push_back(shelfUp);

    // 3. Storage Capacity (Level 1: 50 item, Level 2: 80 item, Level 3: 120 item)
    UpgradeInfo storageUp;
    storageUp.type = UpgradeType::STORAGE_CAPACITY;
    storageUp.name = "Kapasitas Gudang (Storage)";
    storageUp.description = "Meningkatkan daya simpan pallet barang di storage";
    storageUp.currentLevel = 1;
    storageUp.maxLevel = 3;
    storageUp.costs = { 40000, 85000 }; // Lvl 1->2: Rp40.000, Lvl 2->3: Rp85.000
    storageUp.levelDescriptions = { "50 Item / Pallet", "80 Item / Pallet", "120 Item / Pallet" };
    upgrades.push_back(storageUp);

    // 4. Customer Capacity (Level 1: 3 customer, Level 2: 5 customer, Level 3: 7 customer)
    UpgradeInfo custUp;
    custUp.type = UpgradeType::CUSTOMER_CAPACITY;
    custUp.name = "Kapasitas Customer (Max Customer)";
    custUp.description = "Menampung lebih banyak customer aktif bersamaan di toko";
    custUp.currentLevel = 1;
    custUp.maxLevel = 3;
    custUp.costs = { 45000, 90000 }; // Lvl 1->2: Rp45.000, Lvl 2->3: Rp90.000
    custUp.levelDescriptions = { "Maks 3 Customer", "Maks 5 Customer", "Maks 7 Customer" };
    upgrades.push_back(custUp);
}

int ShopUpgrade::GetLevel(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        return upgrades[idx].currentLevel;
    }
    return 1;
}

void ShopUpgrade::SetLevel(UpgradeType type, int lvl) {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        upgrades[idx].currentLevel = std::max(1, std::min(lvl, upgrades[idx].maxLevel));
    }
}

int ShopUpgrade::GetMaxLevel(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        return upgrades[idx].maxLevel;
    }
    return 3;
}

bool ShopUpgrade::IsMaxLevel(UpgradeType type) const {
    return GetLevel(type) >= GetMaxLevel(type);
}

int ShopUpgrade::GetNextUpgradeCost(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        const auto& up = upgrades[idx];
        if (up.currentLevel < up.maxLevel) {
            size_t costIdx = static_cast<size_t>(up.currentLevel - 1);
            if (costIdx < up.costs.size()) {
                return up.costs[costIdx];
            }
        }
    }
    return 0;
}

std::string ShopUpgrade::GetCurrentBenefitString(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        const auto& up = upgrades[idx];
        size_t descIdx = static_cast<size_t>(up.currentLevel - 1);
        if (descIdx < up.levelDescriptions.size()) {
            return up.levelDescriptions[descIdx];
        }
    }
    return "";
}

std::string ShopUpgrade::GetNextBenefitString(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < upgrades.size()) {
        const auto& up = upgrades[idx];
        if (up.currentLevel < up.maxLevel) {
            size_t descIdx = static_cast<size_t>(up.currentLevel);
            if (descIdx < up.levelDescriptions.size()) {
                return up.levelDescriptions[descIdx];
            }
        }
    }
    return "MAX LEVEL";
}

const UpgradeInfo& ShopUpgrade::GetUpgradeInfo(UpgradeType type) const {
    size_t idx = static_cast<size_t>(type);
    return upgrades[idx];
}

int ShopUpgrade::GetShelfCapacity() const {
    int lvl = GetLevel(UpgradeType::SHELF_CAPACITY);
    if (lvl == 1) return 10;
    if (lvl == 2) return 16;
    return 24; // lvl >= 3
}

int ShopUpgrade::GetStorageCapacity() const {
    int lvl = GetLevel(UpgradeType::STORAGE_CAPACITY);
    if (lvl == 1) return 50;
    if (lvl == 2) return 80;
    return 120; // lvl >= 3
}

size_t ShopUpgrade::GetMaxActiveCustomers() const {
    int lvl = GetLevel(UpgradeType::CUSTOMER_CAPACITY);
    if (lvl == 1) return 3;
    if (lvl == 2) return 5;
    return 7; // lvl >= 3
}

int ShopUpgrade::GetShopSizeLevel() const {
    return GetLevel(UpgradeType::SHOP_SIZE);
}

bool ShopUpgrade::PurchaseUpgrade(UpgradeType type, Finance& finance, Shop& shop, std::string& outFeedback) {
    size_t idx = static_cast<size_t>(type);
    if (idx >= upgrades.size()) return false;

    auto& up = upgrades[idx];
    if (up.currentLevel >= up.maxLevel) {
        outFeedback = "Sudah MAX LEVEL!";
        return false;
    }

    int cost = GetNextUpgradeCost(type);
    if (finance.GetCurrentBalance() < cost) {
        outFeedback = "Saldo tidak cukup! Butuh Rp" + std::to_string(cost) + " (Saldo: Rp" + std::to_string(finance.GetCurrentBalance()) + ")";
        return false;
    }

    // Process expense via single Finance system
    std::string desc = "Upgrade " + up.name + " ke Level " + std::to_string(up.currentLevel + 1);
    if (!finance.RecordExpense(cost, desc)) {
        outFeedback = "Gagal memproses transaksi!";
        return false;
    }

    // Level up
    up.currentLevel++;
    ApplyUpgradeEffects(type, shop);

    outFeedback = "Upgrade berhasil! " + up.name + " sekarang Level " + std::to_string(up.currentLevel);
    return true;
}

void ShopUpgrade::ApplyUpgradeEffects(UpgradeType type, Shop& shop) {
    if (type == UpgradeType::SHOP_SIZE) {
        shop.SetShopSizeLevel(GetShopSizeLevel());
    } else if (type == UpgradeType::SHELF_CAPACITY) {
        int newShelfCap = GetShelfCapacity();
        for (auto& r : shop.GetRacks()) {
            r.SetMaxStock(newShelfCap);
        }
    } else if (type == UpgradeType::STORAGE_CAPACITY) {
        shop.GetStorage().SetMaxCapacity(GetStorageCapacity());
    }
}

void ShopUpgrade::NextUpgrade() {
    if (!upgrades.empty()) {
        selectedUpgradeIndex = (selectedUpgradeIndex + 1) % upgrades.size();
    }
}

void ShopUpgrade::PreviousUpgrade() {
    if (!upgrades.empty()) {
        selectedUpgradeIndex = (selectedUpgradeIndex - 1 + (int)upgrades.size()) % (int)upgrades.size();
    }
}

UpgradeType ShopUpgrade::GetSelectedUpgradeType() const {
    if (selectedUpgradeIndex >= 0 && selectedUpgradeIndex < (int)upgrades.size()) {
        return upgrades[selectedUpgradeIndex].type;
    }
    return UpgradeType::SHOP_SIZE;
}
