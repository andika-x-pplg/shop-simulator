#pragma once
#include <string>
#include <vector>
#include <map>
#include "raylib.h"

// Forward declarations of systems
class Player;
class Shop;
class Finance;
class PriceManager;
class Reputation;
class ShopUpgrade;
class Furniture;
class Supplier;
class GameTime;
class DailyStats;
class EmployeeManager;
class RandomEventManager;
class ShopExpansion;
class MarketSystem;

struct MarketSaveData {
    int productType;
    int currentMarketPrice;
    int currentSupplierPrice;
    int demand;
    int trend;
};

struct EmployeeSaveData {
    int id;
    std::string name;
    int role;
    int salary;
    int hiringCost;
    int level;
    int experience;
    int skill;
    int morale;
    int productivity;
    int status;
    bool isHired;
};

struct SaveData {
    int saveVersion; // default: 1

    // Player
    float playerPosX;
    float playerPosY;
    float playerPosZ;

    // Time
    int currentDay;
    int currentHour;
    int currentMinute;
    bool isShopOpen;

    // Economy
    int currentBalance;
    int totalRevenue;
    int totalExpenses;

    // Daily Stats
    int dailyRevenue;
    int dailyExpenses;
    int dailyCustomers;

    // Reputation
    int reputation;
    int totalRatings;
    int totalRatingPoints;

    // Product Stocks (Extended for Tahap 15 & Stage 19)
    int rackBeverageStock;
    int rackBreadStock;
    int rackCannedFoodStock;
    int rackSoapStock;
    int rackTeaStock;
    int rackNoodleStock;
    int storageBeverageStock;
    int storageBreadStock;
    int storageCannedFoodStock;
    int storageSoapStock;
    int storageTeaStock;
    int storageNoodleStock;
    int storageBiscuitStock;
    int storageTissueStock;

    // Prices (Extended for Tahap 15)
    int sellPriceBeverage;
    int sellPriceBread;
    int sellPriceCannedFood;
    int sellPriceSoap;
    int sellPriceTea;
    int sellPriceNoodle;
    int sellPriceBiscuit;
    int sellPriceJuice;

    // Product Statistics (Tahap 15)
    struct ProductStatSave {
        int totalSold;
        int totalRevenue;
        int totalProfit;
    };
    std::map<int, ProductStatSave> productStats;

    // Upgrades (Levels 1-3)
    int shopSizeLevel;
    int shelfCapacityLevel;
    int storageCapacityLevel;
    int customerCapacityLevel;

    // Stage 19: Shop Expansion Level & Zones
    int expansionLevel;

    // Furniture Owned (bools)
    bool tableOwned;
    bool chairOwned;
    bool displayShelfOwned;
    bool cabinetOwned;
    bool decorationPlantOwned;
    bool expansionTableOwned;
    bool expansionChairOwned;
    bool expansionShelfOwned;
    bool expansionCabinetOwned;
    bool expansionPlantOwned;

    // Equipment Owned (bools)
    bool betterDisplayOwned;
    bool extraStorageRackOwned;
    bool betterCashierOwned;

    // Supplier Orders
    struct OrderData {
        int productType;
        int quantity;
        float remainingTime;
    };
    std::vector<OrderData> activeOrders;

    // Employees (Tahap 16)
    std::vector<EmployeeSaveData> activeEmployees;
    std::vector<EmployeeSaveData> candidateEmployees;

    // Random Events & Challenges (Tahap 18)
    int activeEventId;
    float activeEventDuration;
    int activeEventProduct;
    int challengeId;
    int challengeTarget;
    int challengeCurrent;
    int challengeProduct;
    int challengeMoneyReward;
    int challengeRepReward;
    bool challengeCompleted;
    bool challengeRewardClaimed;

    // Stage 20 & 21: Market System Data
    std::vector<MarketSaveData> marketEntries;
    int playerPopularity;
    float playerMarketShare;
    int categoryDrinkTrend;
    int categoryFoodTrend;
    int categorySnackTrend;
    int categoryHouseholdTrend;
    int marketEventId;
    int marketEventDaysLeft;
    struct CompetitorSaveData {
        int id;
        std::string name;
        int reputation;
        int popularity;
        float marketShare;
        std::vector<std::pair<int, int>> prices;
    };
    std::vector<CompetitorSaveData> competitorEntries;

    SaveData();
};

class SaveSystem {
public:
    SaveSystem();

    void Init();

    // Core Save / Load / New Game
    bool SaveGame(const std::string& filepath,
                  const Player& player,
                  const Shop& shop,
                  const Finance& finance,
                  const PriceManager& priceMgr,
                  const Reputation& reputation,
                  const ShopUpgrade& shopUpgrade,
                  const Furniture& furniture,
                  const Supplier& supplier,
                  const GameTime& gameTime,
                  const DailyStats& dailyStats,
                  const EmployeeManager& employeeMgr,
                  const RandomEventManager& eventMgr,
                  const ShopExpansion& shopExpansion,
                  const MarketSystem& marketSystem,
                  std::string& outMessage);

    bool LoadGame(const std::string& filepath,
                  Player& player,
                  Shop& shop,
                  Finance& finance,
                  PriceManager& priceMgr,
                  Reputation& reputation,
                  ShopUpgrade& shopUpgrade,
                  Furniture& furniture,
                  Supplier& supplier,
                  GameTime& gameTime,
                  DailyStats& dailyStats,
                  EmployeeManager& employeeMgr,
                  RandomEventManager& eventMgr,
                  ShopExpansion& shopExpansion,
                  MarketSystem& marketSystem,
                  std::string& outMessage);

    bool HasSaveGame(const std::string& filepath) const;
    bool GetSaveSlotSummary(const std::string& filepath, int& outDay, int& outHour, int& outMinute, int& outBalance) const;

    // Validation
    static bool ValidateSaveData(SaveData& data, std::string& outWarning);

    // Modal UI Navigation
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    int GetSelectedActionIndex() const { return selectedAction; } // 0: Continue, 1: Save Game, 2: Load Game, 3: New Game, 4: Exit
    void NextAction();
    void PreviousAction();

    void RenderMenu(int screenWidth, int screenHeight, const std::string& savePath, int curDay, const std::string& curTime, int curBalance) const;

private:
    bool menuOpen;
    int selectedAction;
    std::string defaultSavePath;

    void EnsureSaveDirectoryExists(const std::string& filepath) const;
};
