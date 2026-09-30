#include "SaveSystem.hpp"
#include "Player.hpp"
#include "Shop.hpp"
#include "Finance.hpp"
#include "PriceManager.hpp"
#include "Reputation.hpp"
#include "ShopUpgrade.hpp"
#include "Furniture.hpp"
#include "Supplier.hpp"
#include "GameTime.hpp"
#include "DailyStats.hpp"
#include "EmployeeManager.hpp"
#include "RandomEventManager.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

SaveData::SaveData()
    : saveVersion(1),
      playerPosX(0.0f), playerPosY(0.0f), playerPosZ(10.0f),
      currentDay(1), currentHour(8), currentMinute(0), isShopOpen(true),
      currentBalance(100000), totalRevenue(0), totalExpenses(0),
      dailyRevenue(0), dailyExpenses(0), dailyCustomers(0),
      reputation(50), totalRatings(0), totalRatingPoints(0),
      rackBeverageStock(10), rackBreadStock(8), rackCannedFoodStock(12),
      rackSoapStock(10), rackTeaStock(12), rackNoodleStock(15),
      storageBeverageStock(0), storageBreadStock(0), storageCannedFoodStock(0),
      storageSoapStock(0), storageTeaStock(0), storageNoodleStock(0),
      sellPriceBeverage(5000), sellPriceBread(8000), sellPriceCannedFood(12000),
      sellPriceSoap(6000), sellPriceTea(7000), sellPriceNoodle(4500),
      sellPriceBiscuit(9000), sellPriceJuice(10000),
      shopSizeLevel(1), shelfCapacityLevel(1), storageCapacityLevel(1), customerCapacityLevel(1),
      tableOwned(false), chairOwned(false), displayShelfOwned(false), cabinetOwned(false), decorationPlantOwned(false),
      betterDisplayOwned(false), extraStorageRackOwned(false), betterCashierOwned(false),
      activeEventId(0), activeEventDuration(0.0f), activeEventProduct(0),
      challengeId(0), challengeTarget(0), challengeCurrent(0), challengeProduct(0),
      challengeMoneyReward(0), challengeRepReward(0), challengeCompleted(false), challengeRewardClaimed(false)
{
}

SaveSystem::SaveSystem()
    : menuOpen(false),
      selectedAction(0),
      defaultSavePath("save/savegame.json")
{
}

void SaveSystem::Init() {
    menuOpen = false;
    selectedAction = 0;
    defaultSavePath = "save/savegame.json";
    EnsureSaveDirectoryExists(defaultSavePath);
}

void SaveSystem::EnsureSaveDirectoryExists(const std::string& filepath) const {
    try {
        fs::path p(filepath);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path());
        }
    } catch (...) {
        // Ignored, handled during file open
    }
}

bool SaveSystem::HasSaveGame(const std::string& filepath) const {
    std::ifstream file(filepath);
    return file.good();
}

bool SaveSystem::ValidateSaveData(SaveData& data, std::string& outWarning) {
    if (data.saveVersion != 1) {
        outWarning = "Versi save game tidak kompatibel!";
        return false;
    }

    // Clamp values to safe ranges
    data.currentDay = std::max(1, data.currentDay);
    data.currentHour = std::max(0, std::min(23, data.currentHour));
    data.currentMinute = std::max(0, std::min(59, data.currentMinute));

    data.currentBalance = std::max(0, data.currentBalance);
    data.totalRevenue = std::max(0, data.totalRevenue);
    data.totalExpenses = std::max(0, data.totalExpenses);

    data.reputation = std::max(0, std::min(100, data.reputation));
    data.totalRatings = std::max(0, data.totalRatings);
    data.totalRatingPoints = std::max(0, data.totalRatingPoints);

    data.rackBeverageStock = std::max(0, data.rackBeverageStock);
    data.rackBreadStock = std::max(0, data.rackBreadStock);
    data.rackCannedFoodStock = std::max(0, data.rackCannedFoodStock);
    data.rackSoapStock = std::max(0, data.rackSoapStock);
    data.rackTeaStock = std::max(0, data.rackTeaStock);
    data.rackNoodleStock = std::max(0, data.rackNoodleStock);

    data.storageBeverageStock = std::max(0, data.storageBeverageStock);
    data.storageBreadStock = std::max(0, data.storageBreadStock);
    data.storageCannedFoodStock = std::max(0, data.storageCannedFoodStock);
    data.storageSoapStock = std::max(0, data.storageSoapStock);
    data.storageTeaStock = std::max(0, data.storageTeaStock);
    data.storageNoodleStock = std::max(0, data.storageNoodleStock);

    data.sellPriceBeverage = std::max(500, data.sellPriceBeverage);
    data.sellPriceBread = std::max(500, data.sellPriceBread);
    data.sellPriceCannedFood = std::max(500, data.sellPriceCannedFood);
    data.sellPriceSoap = std::max(500, data.sellPriceSoap);
    data.sellPriceTea = std::max(500, data.sellPriceTea);
    data.sellPriceNoodle = std::max(500, data.sellPriceNoodle);
    data.sellPriceBiscuit = std::max(500, data.sellPriceBiscuit);
    data.sellPriceJuice = std::max(500, data.sellPriceJuice);

    data.shopSizeLevel = std::max(1, std::min(3, data.shopSizeLevel));
    data.shelfCapacityLevel = std::max(1, std::min(3, data.shelfCapacityLevel));
    data.storageCapacityLevel = std::max(1, std::min(3, data.storageCapacityLevel));
    data.customerCapacityLevel = std::max(1, std::min(3, data.customerCapacityLevel));

    return true;
}

bool SaveSystem::SaveGame(const std::string& filepath,
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
                          std::string& outMessage)
{
    EnsureSaveDirectoryExists(filepath);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        outMessage = "Gagal menyimpan game! File tidak dapat dibuka.";
        return false;
    }

    // Collect data
    SaveData data;
    data.saveVersion = 1;

    data.playerPosX = player.GetPosition().x;
    data.playerPosY = player.GetPosition().y;
    data.playerPosZ = player.GetPosition().z;

    data.currentDay = gameTime.GetCurrentDay();
    data.currentHour = gameTime.GetCurrentHour();
    data.currentMinute = gameTime.GetCurrentMinute();
    data.isShopOpen = gameTime.IsShopOpen();

    data.currentBalance = finance.GetCurrentBalance();
    data.totalRevenue = finance.GetTotalRevenue();
    data.totalExpenses = finance.GetTotalExpenses();

    data.dailyRevenue = dailyStats.GetDailyRevenue();
    data.dailyExpenses = dailyStats.GetDailyExpenses();
    data.dailyCustomers = dailyStats.GetDailyCustomers();

    data.reputation = reputation.GetReputation();
    data.totalRatings = reputation.GetTotalRatings();
    data.totalRatingPoints = reputation.GetTotalRatingPoints();

    const auto& racks = shop.GetRacks();
    if (racks.size() >= 1) data.rackBeverageStock = racks[0].GetStock();
    if (racks.size() >= 2) data.rackBreadStock = racks[1].GetStock();
    if (racks.size() >= 3) data.rackCannedFoodStock = racks[2].GetStock();
    if (racks.size() >= 4) data.rackSoapStock = racks[3].GetStock();
    if (racks.size() >= 5) data.rackTeaStock = racks[4].GetStock();
    if (racks.size() >= 6) data.rackNoodleStock = racks[5].GetStock();

    data.storageBeverageStock = shop.GetStorage().GetStock(ProductType::BEVERAGE);
    data.storageBreadStock = shop.GetStorage().GetStock(ProductType::BREAD);
    data.storageCannedFoodStock = shop.GetStorage().GetStock(ProductType::CANNED_FOOD);
    data.storageSoapStock = shop.GetStorage().GetStock(ProductType::SOAP_BAR);
    data.storageTeaStock = shop.GetStorage().GetStock(ProductType::TEA_BOTTLE);
    data.storageNoodleStock = shop.GetStorage().GetStock(ProductType::INSTANT_NOODLE);

    data.sellPriceBeverage = priceMgr.GetSellPrice(ProductType::BEVERAGE);
    data.sellPriceBread = priceMgr.GetSellPrice(ProductType::BREAD);
    data.sellPriceCannedFood = priceMgr.GetSellPrice(ProductType::CANNED_FOOD);
    data.sellPriceSoap = priceMgr.GetSellPrice(ProductType::SOAP_BAR);
    data.sellPriceTea = priceMgr.GetSellPrice(ProductType::TEA_BOTTLE);
    data.sellPriceNoodle = priceMgr.GetSellPrice(ProductType::INSTANT_NOODLE);
    data.sellPriceBiscuit = priceMgr.GetSellPrice(ProductType::SNACK_BISCUIT);
    data.sellPriceJuice = priceMgr.GetSellPrice(ProductType::TISSUE_PACK);

    const auto& allStats = priceMgr.GetAllStats();
    for (const auto& kv : allStats) {
        data.productStats[static_cast<int>(kv.first)] = { kv.second.totalSold, kv.second.totalRevenue, kv.second.totalProfit };
    }

    data.shopSizeLevel = shopUpgrade.GetLevel(UpgradeType::SHOP_SIZE);
    data.shelfCapacityLevel = shopUpgrade.GetLevel(UpgradeType::SHELF_CAPACITY);
    data.storageCapacityLevel = shopUpgrade.GetLevel(UpgradeType::STORAGE_CAPACITY);
    data.customerCapacityLevel = shopUpgrade.GetLevel(UpgradeType::CUSTOMER_CAPACITY);

    data.tableOwned = furniture.IsFurnitureOwned(FurnitureType::TABLE);
    data.chairOwned = furniture.IsFurnitureOwned(FurnitureType::CHAIR);
    data.displayShelfOwned = furniture.IsFurnitureOwned(FurnitureType::DISPLAY_SHELF);
    data.cabinetOwned = furniture.IsFurnitureOwned(FurnitureType::CABINET);
    data.decorationPlantOwned = furniture.IsFurnitureOwned(FurnitureType::DECORATION_PLANT);

    data.betterDisplayOwned = furniture.IsEquipmentOwned(EquipmentType::BETTER_DISPLAY);
    data.extraStorageRackOwned = furniture.IsEquipmentOwned(EquipmentType::EXTRA_STORAGE_RACK);
    data.betterCashierOwned = furniture.IsEquipmentOwned(EquipmentType::BETTER_CASHIER);

    for (const auto& ord : supplier.GetActiveOrders()) {
        data.activeOrders.push_back({ static_cast<int>(ord.productType), ord.quantity, ord.deliveryTimer });
    }

    for (const auto& emp : employeeMgr.GetActiveEmployees()) {
        data.activeEmployees.push_back({ emp.id, emp.name, (int)emp.role, emp.salary, emp.hiringCost, emp.level, emp.experience, emp.skill, emp.morale, emp.productivity, (int)emp.status, emp.isHired });
    }

    // Stage 18 Events & Challenges
    data.activeEventId = eventMgr.GetActiveEventId();
    data.activeEventDuration = eventMgr.GetActiveEventDuration();
    data.activeEventProduct = eventMgr.GetActiveEventProduct();
    data.challengeId = eventMgr.GetChallengeId();
    data.challengeTarget = eventMgr.GetChallengeTarget();
    data.challengeCurrent = eventMgr.GetChallengeCurrent();
    data.challengeProduct = eventMgr.GetChallengeProduct();
    data.challengeMoneyReward = eventMgr.GetChallengeMoneyReward();
    data.challengeRepReward = eventMgr.GetChallengeRepReward();
    data.challengeCompleted = eventMgr.IsChallengeCompleted();
    data.challengeRewardClaimed = eventMgr.IsChallengeRewardClaimed();

    // Write structured JSON formatted save
    file << "{\n";
    file << "  \"saveVersion\": " << data.saveVersion << ",\n";
    file << "  \"player\": {\n";
    file << "    \"posX\": " << data.playerPosX << ",\n";
    file << "    \"posY\": " << data.playerPosY << ",\n";
    file << "    \"posZ\": " << data.playerPosZ << "\n";
    file << "  },\n";
    file << "  \"time\": {\n";
    file << "    \"day\": " << data.currentDay << ",\n";
    file << "    \"hour\": " << data.currentHour << ",\n";
    file << "    \"minute\": " << data.currentMinute << ",\n";
    file << "    \"isShopOpen\": " << (data.isShopOpen ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"economy\": {\n";
    file << "    \"currentBalance\": " << data.currentBalance << ",\n";
    file << "    \"totalRevenue\": " << data.totalRevenue << ",\n";
    file << "    \"totalExpenses\": " << data.totalExpenses << "\n";
    file << "  },\n";
    file << "  \"dailyStats\": {\n";
    file << "    \"revenue\": " << data.dailyRevenue << ",\n";
    file << "    \"expenses\": " << data.dailyExpenses << ",\n";
    file << "    \"customers\": " << data.dailyCustomers << "\n";
    file << "  },\n";
    file << "  \"reputation\": {\n";
    file << "    \"reputation\": " << data.reputation << ",\n";
    file << "    \"totalRatings\": " << data.totalRatings << ",\n";
    file << "    \"totalRatingPoints\": " << data.totalRatingPoints << "\n";
    file << "  },\n";
    file << "  \"stocks\": {\n";
    file << "    \"rackBeverage\": " << data.rackBeverageStock << ",\n";
    file << "    \"rackBread\": " << data.rackBreadStock << ",\n";
    file << "    \"rackCannedFood\": " << data.rackCannedFoodStock << ",\n";
    file << "    \"rackSoap\": " << data.rackSoapStock << ",\n";
    file << "    \"rackTea\": " << data.rackTeaStock << ",\n";
    file << "    \"rackNoodle\": " << data.rackNoodleStock << ",\n";
    file << "    \"storageBeverage\": " << data.storageBeverageStock << ",\n";
    file << "    \"storageBread\": " << data.storageBreadStock << ",\n";
    file << "    \"storageCannedFood\": " << data.storageCannedFoodStock << ",\n";
    file << "    \"storageSoap\": " << data.storageSoapStock << ",\n";
    file << "    \"storageTea\": " << data.storageTeaStock << ",\n";
    file << "    \"storageNoodle\": " << data.storageNoodleStock << "\n";
    file << "  },\n";
    file << "  \"prices\": {\n";
    file << "    \"beverage\": " << data.sellPriceBeverage << ",\n";
    file << "    \"bread\": " << data.sellPriceBread << ",\n";
    file << "    \"cannedFood\": " << data.sellPriceCannedFood << ",\n";
    file << "    \"soap\": " << data.sellPriceSoap << ",\n";
    file << "    \"tea\": " << data.sellPriceTea << ",\n";
    file << "    \"noodle\": " << data.sellPriceNoodle << ",\n";
    file << "    \"biscuit\": " << data.sellPriceBiscuit << ",\n";
    file << "    \"juice\": " << data.sellPriceJuice << "\n";
    file << "  },\n";
    file << "  \"upgrades\": {\n";
    file << "    \"shopSize\": " << data.shopSizeLevel << ",\n";
    file << "    \"shelfCap\": " << data.shelfCapacityLevel << ",\n";
    file << "    \"storageCap\": " << data.storageCapacityLevel << ",\n";
    file << "    \"customerCap\": " << data.customerCapacityLevel << "\n";
    file << "  },\n";
    file << "  \"furniture\": {\n";
    file << "    \"table\": " << (data.tableOwned ? "true" : "false") << ",\n";
    file << "    \"chair\": " << (data.chairOwned ? "true" : "false") << ",\n";
    file << "    \"displayShelf\": " << (data.displayShelfOwned ? "true" : "false") << ",\n";
    file << "    \"cabinet\": " << (data.cabinetOwned ? "true" : "false") << ",\n";
    file << "    \"plant\": " << (data.decorationPlantOwned ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"equipment\": {\n";
    file << "    \"betterDisplay\": " << (data.betterDisplayOwned ? "true" : "false") << ",\n";
    file << "    \"extraStorage\": " << (data.extraStorageRackOwned ? "true" : "false") << ",\n";
    file << "    \"betterCashier\": " << (data.betterCashierOwned ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"events\": {\n";
    file << "    \"eventId\": " << data.activeEventId << ",\n";
    file << "    \"eventDuration\": " << data.activeEventDuration << ",\n";
    file << "    \"eventProduct\": " << data.activeEventProduct << ",\n";
    file << "    \"chId\": " << data.challengeId << ",\n";
    file << "    \"chTarget\": " << data.challengeTarget << ",\n";
    file << "    \"chCurrent\": " << data.challengeCurrent << ",\n";
    file << "    \"chProduct\": " << data.challengeProduct << ",\n";
    file << "    \"chMoney\": " << data.challengeMoneyReward << ",\n";
    file << "    \"chRep\": " << data.challengeRepReward << ",\n";
    file << "    \"chCompleted\": " << (data.challengeCompleted ? "true" : "false") << ",\n";
    file << "    \"chClaimed\": " << (data.challengeRewardClaimed ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"activeOrders\": [\n";
    for (size_t i = 0; i < data.activeOrders.size(); ++i) {
        const auto& ord = data.activeOrders[i];
        file << "    { \"type\": " << ord.productType << ", \"qty\": " << ord.quantity << ", \"timer\": " << ord.remainingTime << " }"
             << (i + 1 < data.activeOrders.size() ? ",\n" : "\n");
    }
    file << "  ],\n";
    file << "  \"employees\": [\n";
    for (size_t i = 0; i < data.activeEmployees.size(); ++i) {
        const auto& emp = data.activeEmployees[i];
        file << "    { \"id\": " << emp.id << ", \"name\": \"" << emp.name << "\", \"role\": " << emp.role << ", \"salary\": " << emp.salary
             << ", \"hiringCost\": " << emp.hiringCost << ", \"level\": " << emp.level << ", \"exp\": " << emp.experience
             << ", \"skill\": " << emp.skill << ", \"morale\": " << emp.morale << ", \"prod\": " << emp.productivity
             << ", \"status\": " << emp.status << " }" << (i + 1 < data.activeEmployees.size() ? ",\n" : "\n");
    }
    file << "  ]\n";
    file << "}\n";

    file.close();
    outMessage = "Game berhasil disimpan!";
    return true;
}

static bool ExtractJsonValue(const std::string& content, const std::string& key, std::string& outVal) {
    size_t pos = content.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;

    size_t colon = content.find(':', pos);
    if (colon == std::string::npos) return false;

    size_t start = content.find_first_not_of(" \t\r\n", colon + 1);
    if (start == std::string::npos) return false;

    size_t end = content.find_first_of(",}\r\n", start);
    if (end == std::string::npos) end = content.size();

    outVal = content.substr(start, end - start);
    // Trim quotes
    if (!outVal.empty() && outVal.front() == '"') outVal.erase(outVal.begin());
    if (!outVal.empty() && outVal.back() == '"') outVal.pop_back();
    return true;
}

static int ReadInt(const std::string& content, const std::string& key, int defaultVal) {
    std::string val;
    if (ExtractJsonValue(content, key, val)) {
        try { return std::stoi(val); } catch (...) {}
    }
    return defaultVal;
}

static float ReadFloat(const std::string& content, const std::string& key, float defaultVal) {
    std::string val;
    if (ExtractJsonValue(content, key, val)) {
        try { return std::stof(val); } catch (...) {}
    }
    return defaultVal;
}

static bool ReadBool(const std::string& content, const std::string& key, bool defaultVal) {
    std::string val;
    if (ExtractJsonValue(content, key, val)) {
        return (val == "true" || val == "1");
    }
    return defaultVal;
}

bool SaveSystem::LoadGame(const std::string& filepath,
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
                          std::string& outMessage)
{
    if (!HasSaveGame(filepath)) {
        outMessage = "Tidak ada save game yang ditemukan!";
        return false;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        outMessage = "Gagal membuka file save!";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    SaveData data;
    data.saveVersion = ReadInt(content, "saveVersion", 1);
    std::string warn;
    if (data.saveVersion != 1) {
        outMessage = "Versi save game tidak kompatibel!";
        return false;
    }

    data.playerPosX = ReadFloat(content, "posX", 0.0f);
    data.playerPosY = ReadFloat(content, "posY", 0.0f);
    data.playerPosZ = ReadFloat(content, "posZ", 10.0f);

    data.currentDay = ReadInt(content, "day", 1);
    data.currentHour = ReadInt(content, "hour", 8);
    data.currentMinute = ReadInt(content, "minute", 0);
    data.isShopOpen = ReadBool(content, "isShopOpen", true);

    data.currentBalance = ReadInt(content, "currentBalance", 100000);
    data.totalRevenue = ReadInt(content, "totalRevenue", 0);
    data.totalExpenses = ReadInt(content, "totalExpenses", 0);

    data.dailyRevenue = ReadInt(content, "revenue", 0);
    data.dailyExpenses = ReadInt(content, "expenses", 0);
    data.dailyCustomers = ReadInt(content, "customers", 0);

    data.reputation = ReadInt(content, "reputation", 50);
    data.totalRatings = ReadInt(content, "totalRatings", 0);
    data.totalRatingPoints = ReadInt(content, "totalRatingPoints", 0);

    data.rackBeverageStock = ReadInt(content, "rackBeverage", 10);
    data.rackBreadStock = ReadInt(content, "rackBread", 8);
    data.rackCannedFoodStock = ReadInt(content, "rackCannedFood", 12);
    data.rackSoapStock = ReadInt(content, "rackSoap", 10);
    data.rackTeaStock = ReadInt(content, "rackTea", 12);
    data.rackNoodleStock = ReadInt(content, "rackNoodle", 15);

    data.storageBeverageStock = ReadInt(content, "storageBeverage", 0);
    data.storageBreadStock = ReadInt(content, "storageBread", 0);
    data.storageCannedFoodStock = ReadInt(content, "storageCannedFood", 0);
    data.storageSoapStock = ReadInt(content, "storageSoap", 0);
    data.storageTeaStock = ReadInt(content, "storageTea", 0);
    data.storageNoodleStock = ReadInt(content, "storageNoodle", 0);

    data.sellPriceBeverage = ReadInt(content, "beverage", 5000);
    data.sellPriceBread = ReadInt(content, "bread", 8000);
    data.sellPriceCannedFood = ReadInt(content, "cannedFood", 12000);
    data.sellPriceSoap = ReadInt(content, "soap", 6000);
    data.sellPriceTea = ReadInt(content, "tea", 7000);
    data.sellPriceNoodle = ReadInt(content, "noodle", 4500);
    data.sellPriceBiscuit = ReadInt(content, "biscuit", 9000);
    data.sellPriceJuice = ReadInt(content, "juice", 10000);

    data.shopSizeLevel = ReadInt(content, "shopSize", 1);
    data.shelfCapacityLevel = ReadInt(content, "shelfCap", 1);
    data.storageCapacityLevel = ReadInt(content, "storageCap", 1);
    data.customerCapacityLevel = ReadInt(content, "customerCap", 1);

    data.tableOwned = ReadBool(content, "table", false);
    data.chairOwned = ReadBool(content, "chair", false);
    data.displayShelfOwned = ReadBool(content, "displayShelf", false);
    data.cabinetOwned = ReadBool(content, "cabinet", false);
    data.decorationPlantOwned = ReadBool(content, "plant", false);

    data.betterDisplayOwned = ReadBool(content, "betterDisplay", false);
    data.extraStorageRackOwned = ReadBool(content, "extraStorage", false);
    data.betterCashierOwned = ReadBool(content, "betterCashier", false);

    // Events & Challenges (Stage 18)
    data.activeEventId = ReadInt(content, "eventId", 0);
    data.activeEventDuration = ReadFloat(content, "eventDuration", 0.0f);
    data.activeEventProduct = ReadInt(content, "eventProduct", 0);
    data.challengeId = ReadInt(content, "chId", 0);
    data.challengeTarget = ReadInt(content, "chTarget", 0);
    data.challengeCurrent = ReadInt(content, "chCurrent", 0);
    data.challengeProduct = ReadInt(content, "chProduct", 0);
    data.challengeMoneyReward = ReadInt(content, "chMoney", 0);
    data.challengeRepReward = ReadInt(content, "chRep", 0);
    data.challengeCompleted = ReadBool(content, "chCompleted", false);
    data.challengeRewardClaimed = ReadBool(content, "chClaimed", false);

    ValidateSaveData(data, warn);

    // Apply to Game Systems
    player.SetPosition({ data.playerPosX, data.playerPosY, data.playerPosZ });

    gameTime.LoadTimeData(data.currentDay, data.currentHour, data.currentMinute, data.isShopOpen);
    finance.LoadFinancialData(data.currentBalance, data.totalRevenue, data.totalExpenses);
    dailyStats.LoadDailyStats(data.dailyRevenue, data.dailyExpenses, data.dailyCustomers);
    reputation.LoadReputationData(data.reputation, data.totalRatings, data.totalRatingPoints);

    // Upgrades
    shopUpgrade.SetLevel(UpgradeType::SHOP_SIZE, data.shopSizeLevel);
    shopUpgrade.SetLevel(UpgradeType::SHELF_CAPACITY, data.shelfCapacityLevel);
    shopUpgrade.SetLevel(UpgradeType::STORAGE_CAPACITY, data.storageCapacityLevel);
    shopUpgrade.SetLevel(UpgradeType::CUSTOMER_CAPACITY, data.customerCapacityLevel);
    shop.SetShopSizeLevel(data.shopSizeLevel);

    // Furniture & Equipment
    furniture.SetFurnitureOwned(FurnitureType::TABLE, data.tableOwned);
    furniture.SetFurnitureOwned(FurnitureType::CHAIR, data.chairOwned);
    furniture.SetFurnitureOwned(FurnitureType::DISPLAY_SHELF, data.displayShelfOwned);
    furniture.SetFurnitureOwned(FurnitureType::CABINET, data.cabinetOwned);
    furniture.SetFurnitureOwned(FurnitureType::DECORATION_PLANT, data.decorationPlantOwned);

    furniture.SetEquipmentOwned(EquipmentType::BETTER_DISPLAY, data.betterDisplayOwned);
    furniture.SetEquipmentOwned(EquipmentType::EXTRA_STORAGE_RACK, data.extraStorageRackOwned);
    furniture.SetEquipmentOwned(EquipmentType::BETTER_CASHIER, data.betterCashierOwned);

    // Capacity sync
    int finalShelfCap = shopUpgrade.GetShelfCapacity() + furniture.GetEquipmentShelfBonus();
    for (auto& r : shop.GetRacks()) {
        r.SetMaxStock(finalShelfCap);
    }
    int finalStorageCap = shopUpgrade.GetStorageCapacity() + furniture.GetEquipmentStorageBonus();
    shop.GetStorage().SetMaxCapacity(finalStorageCap);

    // Stocks
    auto& racks = shop.GetRacks();
    if (racks.size() >= 1) racks[0].SetStock(data.rackBeverageStock);
    if (racks.size() >= 2) racks[1].SetStock(data.rackBreadStock);
    if (racks.size() >= 3) racks[2].SetStock(data.rackCannedFoodStock);
    if (racks.size() >= 4) racks[3].SetStock(data.rackSoapStock);
    if (racks.size() >= 5) racks[4].SetStock(data.rackTeaStock);
    if (racks.size() >= 6) racks[5].SetStock(data.rackNoodleStock);

    shop.GetStorage().SetStock(ProductType::BEVERAGE, data.storageBeverageStock);
    shop.GetStorage().SetStock(ProductType::BREAD, data.storageBreadStock);
    shop.GetStorage().SetStock(ProductType::CANNED_FOOD, data.storageCannedFoodStock);
    shop.GetStorage().SetStock(ProductType::SOAP_BAR, data.storageSoapStock);
    shop.GetStorage().SetStock(ProductType::TEA_BOTTLE, data.storageTeaStock);
    shop.GetStorage().SetStock(ProductType::INSTANT_NOODLE, data.storageNoodleStock);

    // Prices
    std::string fb;
    priceMgr.SetSellPrice(ProductType::BEVERAGE, data.sellPriceBeverage, fb);
    priceMgr.SetSellPrice(ProductType::BREAD, data.sellPriceBread, fb);
    priceMgr.SetSellPrice(ProductType::CANNED_FOOD, data.sellPriceCannedFood, fb);
    priceMgr.SetSellPrice(ProductType::SOAP_BAR, data.sellPriceSoap, fb);
    priceMgr.SetSellPrice(ProductType::TEA_BOTTLE, data.sellPriceTea, fb);
    priceMgr.SetSellPrice(ProductType::INSTANT_NOODLE, data.sellPriceNoodle, fb);
    priceMgr.SetSellPrice(ProductType::SNACK_BISCUIT, data.sellPriceBiscuit, fb);
    priceMgr.SetSellPrice(ProductType::TISSUE_PACK, data.sellPriceJuice, fb);

    // Deserialize Employees (Tahap 16)
    employeeMgr.ClearAllEmployees();
    size_t empArrayPos = content.find("\"employees\": [");
    if (empArrayPos != std::string::npos) {
        size_t arrayEnd = content.find(']', empArrayPos);
        if (arrayEnd != std::string::npos) {
            std::string empSection = content.substr(empArrayPos, arrayEnd - empArrayPos);
            size_t objStart = 0;
            while ((objStart = empSection.find('{', objStart)) != std::string::npos) {
                size_t objEnd = empSection.find('}', objStart);
                if (objEnd == std::string::npos) break;

                std::string objStr = empSection.substr(objStart, objEnd - objStart + 1);

                Employee emp;
                emp.id = ReadInt(objStr, "id", 1);
                std::string n;
                if (ExtractJsonValue(objStr, "name", n)) emp.name = n; else emp.name = "Employee";
                emp.role = static_cast<EmployeeRole>(ReadInt(objStr, "role", 0));
                emp.salary = ReadInt(objStr, "salary", 40000);
                emp.hiringCost = ReadInt(objStr, "hiringCost", 80000);
                emp.level = ReadInt(objStr, "level", 1);
                emp.experience = ReadInt(objStr, "exp", 0);
                emp.skill = ReadInt(objStr, "skill", 60);
                emp.morale = ReadInt(objStr, "morale", 80);
                emp.productivity = ReadInt(objStr, "prod", 75);
                emp.status = static_cast<EmployeeStatus>(ReadInt(objStr, "status", 1));
                emp.isHired = true;

                // Set appropriate colors based on role
                emp.skinColor = { 245, 210, 180, 255 };
                if (emp.role == EmployeeRole::CASHIER) {
                    emp.shirtColor = { 41, 128, 185, 255 };
                    emp.accessoryColor = { 241, 196, 15, 255 };
                } else if (emp.role == EmployeeRole::STOCKER) {
                    emp.shirtColor = { 230, 126, 34, 255 };
                    emp.accessoryColor = { 192, 57, 43, 255 };
                } else {
                    emp.shirtColor = { 39, 174, 96, 255 };
                    emp.accessoryColor = { 46, 204, 113, 255 };
                }
                emp.pantsColor = { 45, 52, 54, 255 };
                emp.heightScale = 1.0f;
                emp.idleTimer = 0.0f;

                employeeMgr.AddEmployeeDirect(emp);
                objStart = objEnd + 1;
            }
        }
    }
    employeeMgr.RefreshCandidates();

    // Deserialize Stage 18 Events & Challenges
    eventMgr.LoadEventState(data.activeEventId, data.activeEventDuration, data.activeEventProduct,
                           data.challengeId, data.challengeTarget, data.challengeCurrent, data.challengeProduct,
                           data.challengeMoneyReward, data.challengeRepReward, data.challengeCompleted, data.challengeRewardClaimed);

    outMessage = "Game berhasil dimuat!";
    return true;
}

bool SaveSystem::GetSaveSlotSummary(const std::string& filepath, int& outDay, int& outHour, int& outMinute, int& outBalance) const {
    if (!HasSaveGame(filepath)) return false;

    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    outDay = ReadInt(content, "day", 1);
    outHour = ReadInt(content, "hour", 8);
    outMinute = ReadInt(content, "minute", 0);
    outBalance = ReadInt(content, "currentBalance", 100000);
    return true;
}

void SaveSystem::NextAction() {
    selectedAction = (selectedAction + 1) % 5;
}

void SaveSystem::PreviousAction() {
    selectedAction = (selectedAction - 1 + 5) % 5;
}

void SaveSystem::RenderMenu(int screenWidth, int screenHeight, const std::string& savePath, int curDay, const std::string& curTime, int curBalance) const {
    // Dim background
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 185 });

    int modalW = 600;
    int modalH = 480;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    DrawRectangle(modalX, modalY, modalW, modalH, { 22, 28, 38, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 52, 152, 219, 255 });

    DrawText("GAME MENU & SAVE SYSTEM (Tahap 11)", modalX + 30, modalY + 22, 20, { 255, 215, 0, 255 });
    DrawText("Kelola penyimpanan, pemuatan, atau memulai progres baru", modalX + 30, modalY + 48, 13, { 180, 195, 210, 255 });

    // Save Slot 1 Info Box
    int slotBoxY = modalY + 75;
    DrawRectangle(modalX + 30, slotBoxY, modalW - 60, 68, { 28, 35, 46, 240 });
    DrawRectangleLines(modalX + 30, slotBoxY, modalW - 60, 68, { 70, 85, 100, 255 });

    DrawText("SAVE SLOT 1 (Lokal):", modalX + 45, slotBoxY + 10, 14, { 100, 220, 255, 255 });
    
    int sDay = 0, sHour = 0, sMin = 0, sBal = 0;
    if (GetSaveSlotSummary(savePath, sDay, sHour, sMin, sBal)) {
        char timeBuf[16];
        std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", sHour, sMin);
        std::string slotInfo = "Day " + std::to_string(sDay) + "  |  " + std::string(timeBuf) + "  |  Saldo: Rp" + std::to_string(sBal);
        DrawText(slotInfo.c_str(), modalX + 45, slotBoxY + 34, 15, { 50, 255, 120, 255 });
    } else {
        DrawText("[Slot Kosong - Belum ada Save]", modalX + 45, slotBoxY + 34, 15, { 180, 190, 200, 255 });
    }

    // Actions List
    const std::vector<std::string> actionNames = {
        "1. Lanjutkan Game (Continue)",
        "2. Simpan Game (Save Game)",
        "3. Muat Game (Load Game)",
        "4. Permainan Baru (New Game)",
        "5. Keluar Game (Exit)"
    };
    const std::vector<std::string> actionDescs = {
        "Kembali ke permainan yang sedang berjalan",
        "Menyimpan seluruh kondisi progres toko ke Save Slot 1",
        "Memuat progres terakhir dari Save Slot 1",
        "Mereset permainan ke kondisi awal (tidak menghapus save)",
        "Keluar dari simulasi permainan"
    };

    int listY = modalY + 155;
    for (size_t i = 0; i < actionNames.size(); ++i) {
        bool isSel = (selectedAction == (int)i);
        Color itemBg = isSel ? Color{ 35, 65, 100, 240 } : Color{ 28, 34, 45, 200 };
        Color itemBorder = isSel ? Color{ 0, 220, 255, 255 } : Color{ 55, 65, 78, 255 };

        DrawRectangle(modalX + 30, listY, modalW - 60, 48, itemBg);
        DrawRectangleLines(modalX + 30, listY, modalW - 60, 48, itemBorder);

        std::string title = actionNames[i] + (isSel ? "  <-- ENTER" : "");
        DrawText(title.c_str(), modalX + 45, listY + 8, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);
        DrawText(actionDescs[i].c_str(), modalX + 45, listY + 28, 11, { 180, 195, 210, 255 });

        listY += 56;
    }

    // Footer
    int footerY = modalY + 438;
    DrawText("[W / S / Panah] Pilih Opsi    [ENTER] Eksekusi    [ESC] Tutup Menu",
             modalX + 45, footerY, 13, { 255, 220, 120, 255 });
}
