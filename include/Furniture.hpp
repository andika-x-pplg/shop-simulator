#pragma once
#include "Common.hpp"
#include <string>
#include <vector>

class Finance;

enum class FurnitureType {
    TABLE = 0,
    CHAIR,
    DISPLAY_SHELF,
    CABINET,
    DECORATION_PLANT
};

enum class EquipmentType {
    BETTER_DISPLAY = 0,     // Bonus kapasitas rak (+2)
    EXTRA_STORAGE_RACK,     // Bonus kapasitas storage (+15)
    BETTER_CASHIER          // Bonus rating/kecepatan kasir (+5% kepuasan customer)
};

struct FurnitureItem {
    int id;
    std::string name;
    FurnitureType type;
    int price;
    bool isOwned;
    Vector3 position;
    Vector3 size;
    Color primaryColor;
    Color secondaryColor;
    std::string description;
};

struct EquipmentItem {
    int id;
    std::string name;
    EquipmentType type;
    int price;
    bool isOwned;
    std::string bonusDescription;
};

class Furniture {
public:
    Furniture();
    ~Furniture() = default;

    void Init();
    void Render();

    // Colliders untuk physics
    std::vector<AABB> GetColliders() const;

    // Queries
    const std::vector<FurnitureItem>& GetFurnitureList() const { return furnitureList; }
    const std::vector<EquipmentItem>& GetEquipmentList() const { return equipmentList; }
    bool IsFurnitureOwned(FurnitureType type) const;
    bool IsEquipmentOwned(EquipmentType type) const;

    // Equipment bonus queries
    int GetEquipmentShelfBonus() const;     // +2 per rack jika memiliki BETTER_DISPLAY
    int GetEquipmentStorageBonus() const;   // +15 storage jika memiliki EXTRA_STORAGE_RACK
    bool HasBetterCashierEquipment() const; // true jika BETTER_CASHIER dibeli

    // Purchase execution
    bool PurchaseFurniture(int index, Finance& finance, std::string& outFeedback);
    bool PurchaseEquipment(int index, Finance& finance, std::string& outFeedback);

    // Menu UI state (Tombol B)
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    int GetSelectedTabIndex() const { return selectedTab; } // 0: Furniture, 1: Equipment
    void SwitchTab() { selectedTab = (selectedTab == 0) ? 1 : 0; selectedIndex = 0; }

    int GetSelectedIndex() const { return selectedIndex; }
    void NextItem();
    void PreviousItem();

private:
    std::vector<FurnitureItem> furnitureList;
    std::vector<EquipmentItem> equipmentList;
    
    bool menuOpen;
    int selectedTab; // 0 = Furniture, 1 = Equipment
    int selectedIndex;

    void DrawTable(const FurnitureItem& item);
    void DrawChair(const FurnitureItem& item);
    void DrawDisplayShelf(const FurnitureItem& item);
    void DrawCabinet(const FurnitureItem& item);
    void DrawPlant(const FurnitureItem& item);
};
