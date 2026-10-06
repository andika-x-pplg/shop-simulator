#pragma once
#include "Product.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "raylib.h"

// ==========================================
// STAGE 21: COMPETITION & LOCAL MARKET
// ==========================================

// Market Trend Direction per Category / General
enum class MarketTrend {
    FALLING = 0, // Demand & market price trending down
    STABLE,      // Steady demand & reference market price
    RISING       // High customer search frequency & premium price tolerance
};

inline std::string GetMarketTrendName(MarketTrend trend) {
    switch (trend) {
        case MarketTrend::RISING: return "RISING (Permintaan Naik)";
        case MarketTrend::FALLING: return "FALLING (Permintaan Turun)";
        case MarketTrend::STABLE:
        default: return "STABLE (Stabil)";
    }
}

inline Color GetMarketTrendColor(MarketTrend trend) {
    switch (trend) {
        case MarketTrend::RISING: return Color{ 46, 204, 113, 255 };  // Green
        case MarketTrend::FALLING: return Color{ 231, 76, 60, 255 };  // Red
        case MarketTrend::STABLE:
        default: return Color{ 52, 152, 219, 255 };                   // Blue
    }
}

// Competitor Pricing Strategy Archetypes
enum class CompetitorStrategy {
    BALANCED = 0,    // Follows market reference with minor standard markup (~0% to +5%)
    AGGRESSIVE,      // Discount / price-cutter strategy (-5% to -15% below market)
    PREMIUM          // Premium / high-service pricing (+10% to +20% above market)
};

inline std::string GetCompetitorStrategyName(CompetitorStrategy strat) {
    switch (strat) {
        case CompetitorStrategy::AGGRESSIVE: return "Aggressive (Pemotong Harga)";
        case CompetitorStrategy::PREMIUM:    return "Premium (Kualitas Tinggi)";
        case CompetitorStrategy::BALANCED:
        default:                             return "Balanced (Seimbang)";
    }
}

inline Color GetCompetitorStrategyColor(CompetitorStrategy strat) {
    switch (strat) {
        case CompetitorStrategy::AGGRESSIVE: return Color{ 230, 126, 34, 255 }; // Orange
        case CompetitorStrategy::PREMIUM:    return Color{ 155, 89, 182, 255 }; // Purple
        case CompetitorStrategy::BALANCED:
        default:                             return Color{ 52, 152, 219, 255 };  // Blue
    }
}

// Data representation for a single competitor shop
struct CompetitorShop {
    int id;
    std::string name;
    std::string description;
    std::string condition;          // e.g. "Ramai & Bersih", "Modern & Lengkap", "Hemat & Padat"
    CompetitorStrategy strategy;
    
    int reputation;                 // 0 - 100
    int popularity;                 // 0 - 100
    float customerAttraction;       // Attraction score used for market share
    float marketShare;              // Computed percentage (0% - 100%)
    
    std::map<ProductType, int> productPrices; // Individual retail prices per product
    
    int GetPrice(ProductType type, int fallbackPrice = 5000) const {
        auto it = productPrices.find(type);
        if (it != productPrices.end()) {
            return it->second;
        }
        return fallbackPrice;
    }
    
    int GetAveragePrice(const std::vector<ProductType>& productList) const {
        if (productList.empty()) return 0;
        int sum = 0;
        for (auto p : productList) {
            sum += GetPrice(p);
        }
        return sum / (int)productList.size();
    }
};

// Market Event definition (Weekly or periodic market conditions)
struct MarketEvent {
    int id;
    std::string name;
    std::string description;
    ProductCategory affectedCategory; // Which category is boosted/affected
    int demandModifier;               // -30 to +30 demand points
    float priceModifier;              // Price multiplier factor (e.g. 0.9f for sale, 1.15f for shortage)
    int durationDays;                 // Active duration in days
    int remainingDays;
};

// Product Market Data representation
struct ProductMarketData {
    ProductType type;
    std::string sku;
    std::string name;
    ProductCategory category;
    
    int baseMarketPrice;       // Standard base market retail price
    int currentMarketPrice;    // Dynamic current market retail price (influenced by trend, events)
    int baseSupplierPrice;     // Standard wholesale supplier procurement cost
    int currentSupplierPrice;  // Dynamic supplier price (fluctuates with market conditions)
    
    int demand;                // 0 - 100 demand index (0-20: Very Low, 21-40: Low, 41-60: Normal, 61-80: High, 81-100: Very High)
    float demandMultiplier;    // Factor (0.5x to 1.8x) applied to customer purchase desire
    MarketTrend trend;         // Market trend status
    
    // Historical / Temporary Modifiers
    int dailyPriceFluctuation; // + / - offset applied at day start
    int eventPriceModifier;    // Temporary event modifier
    int eventDemandModifier;   // Temporary event demand boost
};

// Forward declaration
class Reputation;

class MarketSystem {
public:
    static MarketSystem& Instance();

    void Init();
    
    // ==========================================
    // Day Transition & Market Simulation (Daily)
    // ==========================================
    void UpdateDailyMarket(int day, int playerReputation, int dailyCustomersServed, int dailyRevenue, std::string& outSummary, Color& outColor);
    
    // ==========================================
    // Competitors & Market Share Queries
    // ==========================================
    const std::vector<CompetitorShop>& GetCompetitors() const { return competitors; }
    std::vector<CompetitorShop>& GetCompetitorsRef() { return competitors; }
    
    float GetPlayerMarketShare() const { return playerMarketShare; }
    int GetPlayerPopularity() const { return playerPopularity; }
    void SetPlayerPopularity(int pop) { playerPopularity = std::clamp(pop, 0, 100); }
    void AdjustPlayerPopularity(int delta) { playerPopularity = std::clamp(playerPopularity + delta, 0, 100); }

    // Market Averages
    int GetAverageMarketPrice(ProductType type) const;
    int GetOverallAverageMarketPrice() const;
    int GetPlayerAveragePrice() const;

    // ==========================================
    // Product Market Queries
    // ==========================================
    const ProductMarketData& GetMarketData(ProductType type) const;
    int GetCurrentMarketPrice(ProductType type) const;
    int GetCurrentSupplierPrice(ProductType type) const;
    int GetDemand(ProductType type) const;
    MarketTrend GetTrend(ProductType type) const;
    float GetDemandMultiplier(ProductType type) const;
    
    // Category Trends
    MarketTrend GetCategoryTrend(ProductCategory cat) const;
    int GetCategoryDemandBonus(ProductCategory cat) const;

    // Demand & Price Attractiveness Evaluation
    std::string GetDemandLabel(ProductType type) const;
    Color GetDemandColor(ProductType type) const;
    std::string GetPriceAttractivenessLabel(ProductType type, int playerSalePrice) const;
    Color GetPriceAttractivenessColor(ProductType type, int playerSalePrice) const;
    
    // Profitability Analysis
    int CalculateUnitProfit(ProductType type, int playerSalePrice) const;
    float CalculateProfitMarginPercent(ProductType type, int playerSalePrice) const;
    
    // ==========================================
    // Customer Purchasing Influence Hook (Stage 21)
    // Evaluates purchase chance based on Player Price vs Competitor Average, Player Reputation & Popularity
    // ==========================================
    float EvaluateCustomerBuyChance(ProductType type, int playerSalePrice, int customerTypeInt, float baseChance, int playerReputation) const;
    
    // Customer Spawning Rate Modifier based on Player Popularity & Market Share
    float GetCustomerAttractionMultiplier() const;

    // Product Search Weight for Customer Shopping Lists
    float GetProductSelectionWeight(ProductType type) const;

    // ==========================================
    // Market Events (Stage 21)
    // ==========================================
    bool HasActiveMarketEvent() const { return activeEvent.remainingDays > 0; }
    const MarketEvent& GetActiveMarketEvent() const { return activeEvent; }
    std::string GetMarketEventSummary() const;

    // Temporary Event Integration Modifiers (Stage 18 / Stage 20)
    void SetEventModifier(ProductType type, int priceMod, int demandMod);
    void ClearEventModifiers();

    // ==========================================
    // UI Menu Navigation
    // ==========================================
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu();
    void SetMenuOpen(bool open);
    void NextProduct();
    void PreviousProduct();
    void NextTab();
    void PreviousTab();
    int GetCurrentTab() const { return currentTab; } // 0: Overview & Competitors, 1: Product Market & Pricing, 2: Trends & Events
    int GetSelectedIndex() const { return selectedProductIndex; }
    ProductType GetSelectedProductType() const;

    // Render 2D Modal UI
    void RenderUI(int screenWidth, int screenHeight, int currentBalance, int currentDay, int playerReputation);

    // ==========================================
    // Save / Load Serialization Support
    // ==========================================
    struct MarketSaveEntry {
        int productType;
        int currentMarketPrice;
        int currentSupplierPrice;
        int demand;
        int trend;
    };
    
    struct CompetitorSaveEntry {
        int id;
        std::string name;
        int reputation;
        int popularity;
        float marketShare;
        std::vector<std::pair<int, int>> prices; // productType, price
    };

    struct MarketStateSave {
        int playerPopularity;
        float playerMarketShare;
        int categoryDrinkTrend;
        int categoryFoodTrend;
        int categorySnackTrend;
        int categoryHouseholdTrend;
        int activeEventId;
        int activeEventDaysLeft;
        std::vector<MarketSaveEntry> marketEntries;
        std::vector<CompetitorSaveEntry> competitorEntries;
    };

    MarketStateSave ExportFullSaveData() const;
    void ImportFullSaveData(const MarketStateSave& save);

    // Legacy Stage 20 compatibility
    std::vector<MarketSaveEntry> ExportMarketSaveData() const;
    void ImportMarketSaveData(const std::vector<MarketSaveEntry>& data);

private:
    MarketSystem();
    ~MarketSystem() = default;

    MarketSystem(const MarketSystem&) = delete;
    MarketSystem& operator=(const MarketSystem&) = delete;

    void InitializeCompetitors();
    void SimulateCompetitorPrices(int day);
    void CalculateMarketShare(int playerReputation);
    void TriggerRandomMarketEvent(int day);

    std::map<ProductType, ProductMarketData> marketMap;
    std::vector<ProductType> productList;
    std::vector<CompetitorShop> competitors;
    
    std::map<ProductCategory, MarketTrend> categoryTrends;
    MarketEvent activeEvent;

    int playerPopularity;       // 0 - 100
    float playerMarketShare;    // 0% - 100%

    int selectedProductIndex;
    int currentTab;             // 0: Competitors & Market Share, 1: Product Price Matrix, 2: Trends & Events
    bool menuOpen;
};
