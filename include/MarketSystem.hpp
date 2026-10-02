#pragma once
#include "Product.hpp"
#include <string>
#include <vector>
#include <map>
#include "raylib.h"

// Market Trend Direction
enum class MarketTrend {
    FALLING = 0, // Demand & market price trending down
    STABLE,      // Steady demand & reference market price
    RISING       // High customer search frequency & premium price tolerance
};

inline std::string GetMarketTrendName(MarketTrend trend) {
    switch (trend) {
        case MarketTrend::RISING: return "RISING (Naik)";
        case MarketTrend::FALLING: return "FALLING (Turun)";
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

class Finance; // Forward declaration

class MarketSystem {
public:
    static MarketSystem& Instance();

    void Init();
    
    // Day Transition & Market Update
    void UpdateDailyMarket(int day, std::string& outSummary, Color& outColor);
    
    // Market Queries per Product
    const ProductMarketData& GetMarketData(ProductType type) const;
    int GetCurrentMarketPrice(ProductType type) const;
    int GetCurrentSupplierPrice(ProductType type) const;
    int GetDemand(ProductType type) const;
    MarketTrend GetTrend(ProductType type) const;
    float GetDemandMultiplier(ProductType type) const;
    
    // Demand & Price Attractiveness Evaluation
    std::string GetDemandLabel(ProductType type) const;
    Color GetDemandColor(ProductType type) const;
    std::string GetPriceAttractivenessLabel(ProductType type, int playerSalePrice) const;
    Color GetPriceAttractivenessColor(ProductType type, int playerSalePrice) const;
    
    // Profitability Analysis
    int CalculateUnitProfit(ProductType type, int playerSalePrice) const;
    float CalculateProfitMarginPercent(ProductType type, int playerSalePrice) const;
    
    // Customer Purchasing Influence Hook
    // Evaluates purchase probability modifier given player price vs current market price & product demand
    float EvaluateCustomerBuyChance(ProductType type, int playerSalePrice, int customerTypeInt, float baseChance) const;
    
    // Product Search Weight for Customer Shopping Lists
    float GetProductSelectionWeight(ProductType type) const;

    // Temporary Event Integration Modifiers (Stage 18 / Stage 20)
    void SetEventModifier(ProductType type, int priceMod, int demandMod);
    void ClearEventModifiers();

    // UI Menu Navigation
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu();
    void SetMenuOpen(bool open);
    void NextProduct();
    void PreviousProduct();
    int GetSelectedIndex() const { return selectedProductIndex; }
    ProductType GetSelectedProductType() const;

    // Render 2D Modal UI
    void RenderUI(int screenWidth, int screenHeight, int currentBalance, int currentDay);

    // Save / Load Serialization Support
    struct MarketSaveEntry {
        int productType;
        int currentMarketPrice;
        int currentSupplierPrice;
        int demand;
        int trend;
    };
    std::vector<MarketSaveEntry> ExportMarketSaveData() const;
    void ImportMarketSaveData(const std::vector<MarketSaveEntry>& data);

private:
    MarketSystem();
    ~MarketSystem() = default;

    MarketSystem(const MarketSystem&) = delete;
    MarketSystem& operator=(const MarketSystem&) = delete;

    std::map<ProductType, ProductMarketData> marketMap;
    std::vector<ProductType> productList;
    int selectedProductIndex;
    bool menuOpen;
};
