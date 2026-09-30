#include "PriceManager.hpp"
#include <algorithm>

PriceManager& PriceManager::Instance() {
    static PriceManager instance;
    return instance;
}

PriceManager::PriceManager()
    : selectedProductIndex(0),
      menuOpen(false)
{
    Init();
}

void PriceManager::Init() {
    managedProducts = {
        ProductType::BEVERAGE,
        ProductType::BREAD,
        ProductType::CANNED_FOOD
    };

    sellPrices.clear();
    for (auto type : managedProducts) {
        ProductInfo baseInfo = GetProductInfo(type);
        sellPrices[type] = baseInfo.sellPrice;
    }

    selectedProductIndex = 0;
    menuOpen = false;
}

int PriceManager::GetSellPrice(ProductType type) const {
    auto it = sellPrices.find(type);
    if (it != sellPrices.end()) {
        return it->second;
    }
    return GetProductInfo(type).sellPrice;
}

int PriceManager::GetBuyPrice(ProductType type) const {
    return GetProductInfo(type).buyPrice;
}

int PriceManager::GetUnitMargin(ProductType type) const {
    return GetSellPrice(type) - GetBuyPrice(type);
}

bool PriceManager::SetSellPrice(ProductType type, int newPrice, std::string& outFeedback) {
    if (newPrice <= 0) {
        outFeedback = "Periksa harga jual! Harga harus lebih dari Rp0";
        return false;
    }

    sellPrices[type] = newPrice;
    ProductInfo info = GetProductInfo(type);
    int buyPrice = GetBuyPrice(type);

    if (newPrice < buyPrice) {
        outFeedback = "Harga " + info.name + " diubah menjadi Rp" + std::to_string(newPrice) + 
                      " (Peringatan: harga jual di bawah harga beli)";
    } else {
        outFeedback = "Harga " + info.name + " diubah menjadi Rp" + std::to_string(newPrice);
    }
    return true;
}

void PriceManager::AdjustSellPrice(ProductType type, int delta) {
    int current = GetSellPrice(type);
    int target = current + delta;
    if (target < 500) target = 500; // Minimal batas Rp500 (> 0)
    std::string dummy;
    SetSellPrice(type, target, dummy);
}

void PriceManager::NextProduct() {
    if (!managedProducts.empty()) {
        selectedProductIndex = (selectedProductIndex + 1) % managedProducts.size();
    }
}

void PriceManager::PreviousProduct() {
    if (!managedProducts.empty()) {
        selectedProductIndex = (selectedProductIndex - 1 + (int)managedProducts.size()) % (int)managedProducts.size();
    }
}

ProductType PriceManager::GetSelectedProductType() const {
    if (selectedProductIndex >= 0 && selectedProductIndex < (int)managedProducts.size()) {
        return managedProducts[selectedProductIndex];
    }
    return ProductType::NONE;
}
