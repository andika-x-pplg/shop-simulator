#pragma once
#include "Product.hpp"
#include <map>
#include <vector>
#include <string>

struct ProductSalesStats {
    int totalSold;     // Cumulative units sold
    int totalRevenue;  // Cumulative gross revenue
    int totalProfit;   // Cumulative net profit
    int dailySold;     // Units sold today
};

class PriceManager {
public:
    static PriceManager& Instance();

    void Init();

    // Selling Prices (Dynamic per store management)
    int GetSellPrice(ProductType type) const;
    bool SetSellPrice(ProductType type, int newPrice, std::string& outFeedback);
    void AdjustSellPrice(ProductType type, int delta);

    // Dynamic Percentage & Reference Pricing
    int GetReferencePrice(ProductType type) const;
    float GetMarkupPercent(ProductType type) const;
    void SetMarkupPercent(ProductType type, float percent);
    void AdjustMarkupPercent(ProductType type, float deltaPercent);
    std::string GetPriceStatusLabel(ProductType type) const;
    Color GetPriceStatusColor(ProductType type) const;

    // Supplier Buy Prices (Constant/Fixed from supplier)
    int GetBuyPrice(ProductType type) const;

    // Unit Profit / Margin: Margin = SellPrice - BuyPrice
    int GetUnitMargin(ProductType type) const;

    // Product Sales Statistics (Tahap 15)
    void RecordSale(ProductType type, int quantity, int actualUnitPrice);
    ProductSalesStats GetProductStats(ProductType type) const;
    void ResetDailyStats();

    // Best Seller / Slow Seller Analysis (Tahap 15)
    ProductType GetBestSeller() const;
    ProductType GetSlowSeller() const;
    int GetTotalItemsSold() const;
    std::string GetPopularityLevel(ProductType type) const;

    // UI Menu Navigation & Price Edit Mode
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu();
    void SetMenuOpen(bool open);

    int GetSelectedProductIndex() const { return selectedProductIndex; }
    void NextProduct();
    void PreviousProduct();
    ProductType GetSelectedProductType() const;

    const std::vector<ProductType>& GetManagedProducts() const { return managedProducts; }

    // Direct Price Editing Mode (Tahap 15 expansion)
    bool IsEditingPrice() const { return isEditingPrice; }
    void StartEditingPrice();
    void CancelEditingPrice();
    bool ConfirmEditingPrice(std::string& outFeedback);
    void AppendCharToInput(char c);
    void BackspaceInput();
    const std::string& GetInputBuffer() const { return inputBuffer; }

    // Save / Load Support for Product Statistics & Custom Prices
    const std::map<ProductType, int>& GetAllSellPrices() const { return sellPrices; }
    const std::map<ProductType, ProductSalesStats>& GetAllStats() const { return salesStats; }
    void SetProductStats(ProductType type, int totalSold, int totalRevenue, int totalProfit);

private:
    PriceManager();
    ~PriceManager() = default;

    PriceManager(const PriceManager&) = delete;
    PriceManager& operator=(const PriceManager&) = delete;

    std::map<ProductType, int> sellPrices;
    std::map<ProductType, ProductSalesStats> salesStats;
    std::vector<ProductType> managedProducts;
    int selectedProductIndex;
    bool menuOpen;

    // Direct input editing state
    bool isEditingPrice;
    std::string inputBuffer;
};
