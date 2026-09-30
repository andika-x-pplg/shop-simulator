#pragma once
#include "Product.hpp"
#include <map>
#include <vector>
#include <string>

class PriceManager {
public:
    static PriceManager& Instance();

    void Init();

    // Selling Prices (Dynamic per store management)
    int GetSellPrice(ProductType type) const;
    bool SetSellPrice(ProductType type, int newPrice, std::string& outFeedback);
    void AdjustSellPrice(ProductType type, int delta);

    // Supplier Buy Prices (Constant/Fixed from supplier)
    int GetBuyPrice(ProductType type) const;

    // Unit Profit / Margin: Margin = SellPrice - BuyPrice
    int GetUnitMargin(ProductType type) const;

    // UI Menu Navigation
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    int GetSelectedProductIndex() const { return selectedProductIndex; }
    void NextProduct();
    void PreviousProduct();
    ProductType GetSelectedProductType() const;

    const std::vector<ProductType>& GetManagedProducts() const { return managedProducts; }

private:
    PriceManager();
    ~PriceManager() = default;

    PriceManager(const PriceManager&) = delete;
    PriceManager& operator=(const PriceManager&) = delete;

    std::map<ProductType, int> sellPrices;
    std::vector<ProductType> managedProducts;
    int selectedProductIndex;
    bool menuOpen;
};
