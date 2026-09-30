#pragma once
#include <string>
#include <vector>

class Finance;
class Shop;
class Storage;

enum class UpgradeType {
    SHOP_SIZE = 0,
    SHELF_CAPACITY,
    STORAGE_CAPACITY,
    CUSTOMER_CAPACITY
};

struct UpgradeInfo {
    UpgradeType type;
    std::string name;
    std::string description;
    int currentLevel;
    int maxLevel;
    std::vector<int> costs; // Biaya untuk naik level: index 0 = Lvl 1->2, index 1 = Lvl 2->3
    std::vector<std::string> levelDescriptions;
};

class ShopUpgrade {
public:
    ShopUpgrade();
    ~ShopUpgrade() = default;

    void Init();

    // Queries
    int GetLevel(UpgradeType type) const;
    void SetLevel(UpgradeType type, int lvl);
    int GetMaxLevel(UpgradeType type) const;
    bool IsMaxLevel(UpgradeType type) const;
    int GetNextUpgradeCost(UpgradeType type) const;
    std::string GetCurrentBenefitString(UpgradeType type) const;
    std::string GetNextBenefitString(UpgradeType type) const;
    const UpgradeInfo& GetUpgradeInfo(UpgradeType type) const;

    // Derived gameplay values
    int GetShelfCapacity() const;       // Level 1: 10, Level 2: 16, Level 3: 24
    int GetStorageCapacity() const;     // Level 1: 50, Level 2: 80, Level 3: 120
    size_t GetMaxActiveCustomers() const; // Level 1: 3, Level 2: 5, Level 3: 7
    int GetShopSizeLevel() const;       // Level 1: 1, Level 2: 2, Level 3: 3

    // Purchase execution
    bool PurchaseUpgrade(UpgradeType type, Finance& finance, Shop& shop, std::string& outFeedback);

    // UI Menu state (Tombol U)
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    int GetSelectedUpgradeIndex() const { return selectedUpgradeIndex; }
    void NextUpgrade();
    void PreviousUpgrade();
    UpgradeType GetSelectedUpgradeType() const;

private:
    std::vector<UpgradeInfo> upgrades;
    int selectedUpgradeIndex;
    bool menuOpen;

    void ApplyUpgradeEffects(UpgradeType type, Shop& shop);
};
