#include "PriceManager.hpp"
#include <algorithm>

PriceManager& PriceManager::Instance() {
    static PriceManager instance;
    return instance;
}

PriceManager::PriceManager()
    : selectedProductIndex(0),
      menuOpen(false),
      isEditingPrice(false),
      inputBuffer("")
{
    Init();
}

void PriceManager::Init() {
    managedProducts = GetAllProductTypes();

    sellPrices.clear();
    salesStats.clear();

    for (auto type : managedProducts) {
        ProductInfo baseInfo = GetProductInfo(type);
        sellPrices[type] = baseInfo.sellPrice;
        salesStats[type] = { 0, 0, 0, 0 };
    }

    selectedProductIndex = 0;
    menuOpen = false;
    isEditingPrice = false;
    inputBuffer.clear();
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
                      " (Peringatan: Di bawah harga modal Rp" + std::to_string(buyPrice) + "!)";
    } else {
        outFeedback = "Harga jual " + info.name + " berhasil diubah menjadi Rp" + std::to_string(newPrice);
    }
    return true;
}

void PriceManager::AdjustSellPrice(ProductType type, int delta) {
    int current = GetSellPrice(type);
    int updated = std::max(500, current + delta);
    sellPrices[type] = updated;
}

void PriceManager::RecordSale(ProductType type, int quantity, int actualUnitPrice) {
    if (type == ProductType::NONE || quantity <= 0) return;

    int buyPrice = GetBuyPrice(type);
    int unitProfit = actualUnitPrice - buyPrice;

    salesStats[type].totalSold += quantity;
    salesStats[type].dailySold += quantity;
    salesStats[type].totalRevenue += (actualUnitPrice * quantity);
    salesStats[type].totalProfit += (unitProfit * quantity);
}

ProductSalesStats PriceManager::GetProductStats(ProductType type) const {
    auto it = salesStats.find(type);
    if (it != salesStats.end()) {
        return it->second;
    }
    return { 0, 0, 0, 0 };
}

void PriceManager::ResetDailyStats() {
    for (auto& pair : salesStats) {
        pair.second.dailySold = 0;
    }
}

ProductType PriceManager::GetBestSeller() const {
    ProductType best = ProductType::NONE;
    int maxSold = -1;

    for (const auto& pair : salesStats) {
        if (pair.second.totalSold > maxSold) {
            maxSold = pair.second.totalSold;
            best = pair.first;
        }
    }
    return (maxSold > 0) ? best : ProductType::NONE;
}

ProductType PriceManager::GetSlowSeller() const {
    ProductType slowest = ProductType::NONE;
    int minSold = 9999999;

    for (const auto& pair : salesStats) {
        if (pair.second.totalSold < minSold) {
            minSold = pair.second.totalSold;
            slowest = pair.first;
        }
    }
    return slowest;
}

int PriceManager::GetTotalItemsSold() const {
    int total = 0;
    for (const auto& pair : salesStats) {
        total += pair.second.totalSold;
    }
    return total;
}

std::string PriceManager::GetPopularityLevel(ProductType type) const {
    int sold = GetProductStats(type).totalSold;
    if (sold >= 25) return "Sangat Tinggi (Viral)";
    if (sold >= 12) return "Tinggi (Populer)";
    if (sold >= 5)  return "Sedang (Normal)";
    return "Rendah (Baru/Slow)";
}

void PriceManager::SetProductStats(ProductType type, int totalSold, int totalRevenue, int totalProfit) {
    salesStats[type].totalSold = std::max(0, totalSold);
    salesStats[type].totalRevenue = std::max(0, totalRevenue);
    salesStats[type].totalProfit = totalProfit;
}

void PriceManager::ToggleMenu() {
    menuOpen = !menuOpen;
    if (!menuOpen) {
        CancelEditingPrice();
    }
}

void PriceManager::SetMenuOpen(bool open) {
    menuOpen = open;
    if (!menuOpen) {
        CancelEditingPrice();
    }
}

void PriceManager::StartEditingPrice() {
    ProductType cur = GetSelectedProductType();
    if (cur != ProductType::NONE) {
        isEditingPrice = true;
        inputBuffer = std::to_string(GetSellPrice(cur));
    }
}

void PriceManager::CancelEditingPrice() {
    isEditingPrice = false;
    inputBuffer.clear();
}

void PriceManager::AppendCharToInput(char c) {
    if (!isEditingPrice) return;
    if (c >= '0' && c <= '9') {
        // Prevent leading zeros overflow or excessively long input (> 8 digits)
        if (inputBuffer.size() >= 8) return;
        if (inputBuffer == "0") {
            inputBuffer.clear();
        }
        inputBuffer.push_back(c);
    }
}

void PriceManager::BackspaceInput() {
    if (!isEditingPrice) return;
    if (!inputBuffer.empty()) {
        inputBuffer.pop_back();
    }
}

bool PriceManager::ConfirmEditingPrice(std::string& outFeedback) {
    if (!isEditingPrice) return false;

    if (inputBuffer.empty()) {
        outFeedback = "Harga tidak boleh kosong!";
        return false;
    }

    int parsedPrice = 0;
    try {
        parsedPrice = std::stoi(inputBuffer);
    } catch (...) {
        outFeedback = "Harga tidak valid!";
        return false;
    }

    if (parsedPrice <= 0) {
        outFeedback = "Harga tidak boleh 0 atau negatif!";
        return false;
    }

    ProductType curType = GetSelectedProductType();
    if (curType == ProductType::NONE) {
        outFeedback = "Produk tidak valid!";
        CancelEditingPrice();
        return false;
    }

    bool success = SetSellPrice(curType, parsedPrice, outFeedback);
    if (success) {
        isEditingPrice = false;
        inputBuffer.clear();
    }
    return success;
}

void PriceManager::NextProduct() {
    if (managedProducts.empty() || isEditingPrice) return;
    selectedProductIndex = (selectedProductIndex + 1) % managedProducts.size();
}

void PriceManager::PreviousProduct() {
    if (managedProducts.empty() || isEditingPrice) return;
    selectedProductIndex = (selectedProductIndex - 1 + managedProducts.size()) % managedProducts.size();
}

ProductType PriceManager::GetSelectedProductType() const {
    if (selectedProductIndex >= 0 && selectedProductIndex < (int)managedProducts.size()) {
        return managedProducts[selectedProductIndex];
    }
    return ProductType::NONE;
}
