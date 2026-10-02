#pragma once
#include <string>
#include <vector>
#include "raylib.h"

class Finance;
class Shop;
class DailyStats;

// Expansion Zones in the Shop
enum class ShopZone {
    MAIN = 0,       // Level 0 (Awal) - Toko Utama (20x24m)
    RIGHT_WING,     // Level 1 - Perluasan Sisi Kanan (+5m Width)
    BACK_WING,      // Level 2 - Perluasan Belakang (+4m Length)
    MAIN_EXPANSION  // Level 3 - Perluasan Lantai Utama (+5m Width, +4m Length, Advanced Shop)
};

struct ExpansionData {
    int id;                           // 1, 2, 3...
    std::string name;                 // e.g. "Perluasan Sisi Kanan"
    std::string description;          // Deskripsi manfaat
    int tier;                         // Level progression (1, 2, 3)
    int price;                        // Biaya pembelian
    int requiredShopLevel;            // Syarat shop level (atau tier sebelumnya)
    std::string areaSizeDesc;         // e.g. "+5m Lebar Sisi Kanan (Total 25x24m)"
    int additionalCustomerCapacity;   // +2 customer
    int additionalFurnitureCapacity;  // +3 furniture slots
    int additionalShelfCapacity;      // +2 bonus shelf stock capacity / racks
    int additionalStorageCapacity;    // +30 bonus storage capacity
    ShopZone zone;                    // Zone identifier
    bool isPurchased;                 // Status pembelian
};

class ShopExpansion {
public:
    static ShopExpansion& Instance();

    void Init();

    // Query expansions
    int GetExpansionCount() const { return (int)expansions.size(); }
    const std::vector<ExpansionData>& GetExpansions() const { return expansions; }
    const ExpansionData& GetExpansion(int index) const;
    int GetCurrentExpansionLevel() const { return currentExpansionLevel; }
    void SetExpansionLevel(int level, Shop& shop);

    // Zone queries
    bool IsZoneUnlocked(ShopZone zone) const;
    int GetUnlockedZoneCount() const;

    // Cumulative Expansion Bonuses
    int GetBonusCustomerCapacity() const;
    int GetBonusStorageCapacity() const;
    int GetBonusShelfStockCapacity() const;
    int GetBonusFurnitureCapacity() const;

    // Purchase execution
    bool PurchaseExpansion(int index, Finance& finance, Shop& shop, DailyStats& dailyStats, std::string& outFeedback);

    // UI Menu State (Key: X / expansion menu)
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    int GetSelectedIndex() const { return selectedIndex; }
    void NextItem();
    void PreviousItem();

    void RenderUI(int screenWidth, int screenHeight, int currentBalance);

private:
    ShopExpansion();
    ~ShopExpansion() = default;

    std::vector<ExpansionData> expansions;
    int currentExpansionLevel;
    int selectedIndex;
    bool menuOpen;

    void ApplyExpansionToShop(Shop& shop);
};
