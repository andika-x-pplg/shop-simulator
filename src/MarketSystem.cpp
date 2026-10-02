#include "MarketSystem.hpp"
#include "PriceManager.hpp"
#include "Finance.hpp"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

MarketSystem& MarketSystem::Instance() {
    static MarketSystem instance;
    return instance;
}

MarketSystem::MarketSystem()
    : selectedProductIndex(0),
      menuOpen(false)
{
    productList = GetAllProductTypes();
    Init();
}

void MarketSystem::Init() {
    marketMap.clear();
    productList = GetAllProductTypes();
    selectedProductIndex = 0;
    menuOpen = false;

    // Seed market base profiles
    for (auto type : productList) {
        ProductInfo info = GetProductInfo(type);
        ProductMarketData data;
        data.type = type;
        data.sku = info.sku;
        data.name = info.name;
        data.category = info.category;
        
        data.baseMarketPrice = info.sellPrice;
        data.currentMarketPrice = info.sellPrice;
        data.baseSupplierPrice = info.buyPrice;
        data.currentSupplierPrice = info.buyPrice;
        
        data.demand = 50; // Normal baseline
        data.demandMultiplier = 1.0f;
        data.trend = MarketTrend::STABLE;
        
        data.dailyPriceFluctuation = 0;
        data.eventPriceModifier = 0;
        data.eventDemandModifier = 0;

        marketMap[type] = data;
    }
}

void MarketSystem::UpdateDailyMarket(int day, std::string& outSummary, Color& outColor) {
    // Deterministic yet dynamic daily market fluctuation based on day seed
    int risingCount = 0;
    int fallingCount = 0;
    std::string trendingProductName = "";

    for (size_t i = 0; i < productList.size(); ++i) {
        ProductType type = productList[i];
        auto& data = marketMap[type];

        // Pseudo-random roll using day and product index
        int roll = (day * 31 + (int)i * 17 + (int)type * 7) % 100;
        
        // 1. Determine Market Trend (25% Rising, 25% Falling, 50% Stable)
        if (roll < 25) {
            data.trend = MarketTrend::RISING;
            risingCount++;
            if (trendingProductName.empty()) trendingProductName = data.name;
            // Demand surges between 65 - 90
            data.demand = 65 + (roll % 26);
            // Market price rises +5% to +15%
            float priceBonus = 0.05f + ((roll % 11) * 0.01f);
            data.dailyPriceFluctuation = (int)(data.baseMarketPrice * priceBonus);
        } else if (roll < 50) {
            data.trend = MarketTrend::FALLING;
            fallingCount++;
            // Demand drops between 20 - 45
            data.demand = 20 + (roll % 26);
            // Market price falls -5% to -12%
            float priceCut = 0.05f + ((roll % 8) * 0.01f);
            data.dailyPriceFluctuation = -(int)(data.baseMarketPrice * priceCut);
        } else {
            data.trend = MarketTrend::STABLE;
            // Demand stays around 45 - 60
            data.demand = 45 + (roll % 16);
            // Slight noise fluctuation (+/- 2%)
            data.dailyPriceFluctuation = ((roll % 5) - 2) * (data.baseMarketPrice / 100);
        }

        // 2. Compute dynamic market price bounded safely (+/- 25% max from base)
        int minMarket = (int)(data.baseMarketPrice * 0.75f);
        int maxMarket = (int)(data.baseMarketPrice * 1.30f);
        data.currentMarketPrice = std::clamp(data.baseMarketPrice + data.dailyPriceFluctuation + data.eventPriceModifier, minMarket, maxMarket);

        // 3. Compute dynamic supplier wholesale price (+/- 15% fluctuation with market)
        int supplierFluctuation = (int)(data.dailyPriceFluctuation * 0.6f);
        int minSupplier = (int)(data.baseSupplierPrice * 0.80f);
        int maxSupplier = (int)(data.baseSupplierPrice * 1.25f);
        data.currentSupplierPrice = std::clamp(data.baseSupplierPrice + supplierFluctuation, minSupplier, maxSupplier);

        // 4. Compute Demand Multiplier (0.5x to 1.7x)
        int finalDemand = std::clamp(data.demand + data.eventDemandModifier, 5, 100);
        data.demandMultiplier = 0.5f + ((float)finalDemand / 100.0f) * 1.2f;
    }

    if (!trendingProductName.empty()) {
        outSummary = "Laporan Pasar Hari " + std::to_string(day) + ": " + trendingProductName + " sedang tren naik!";
        outColor = { 46, 204, 113, 255 };
    } else {
        outSummary = "Kondisi Pasar Hari " + std::to_string(day) + ": Pasar stabil.";
        outColor = { 52, 152, 219, 255 };
    }
}

const ProductMarketData& MarketSystem::GetMarketData(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return it->second;
    }
    static ProductMarketData defaultData;
    return defaultData;
}

int MarketSystem::GetCurrentMarketPrice(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return it->second.currentMarketPrice;
    }
    return GetProductInfo(type).sellPrice;
}

int MarketSystem::GetCurrentSupplierPrice(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return it->second.currentSupplierPrice;
    }
    return GetProductInfo(type).buyPrice;
}

int MarketSystem::GetDemand(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return std::clamp(it->second.demand + it->second.eventDemandModifier, 0, 100);
    }
    return 50;
}

MarketTrend MarketSystem::GetTrend(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return it->second.trend;
    }
    return MarketTrend::STABLE;
}

float MarketSystem::GetDemandMultiplier(ProductType type) const {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        return it->second.demandMultiplier;
    }
    return 1.0f;
}

std::string MarketSystem::GetDemandLabel(ProductType type) const {
    int d = GetDemand(type);
    if (d <= 20) return "Sangat Rendah (Very Low)";
    if (d <= 40) return "Rendah (Low)";
    if (d <= 60) return "Normal (Medium)";
    if (d <= 80) return "Tinggi (High)";
    return "Sangat Tinggi (Very High)";
}

Color MarketSystem::GetDemandColor(ProductType type) const {
    int d = GetDemand(type);
    if (d <= 20) return Color{ 231, 76, 60, 255 };
    if (d <= 40) return Color{ 230, 126, 34, 255 };
    if (d <= 60) return Color{ 52, 152, 219, 255 };
    if (d <= 80) return Color{ 46, 204, 113, 255 };
    return Color{ 241, 196, 15, 255 };
}

std::string MarketSystem::GetPriceAttractivenessLabel(ProductType type, int playerSalePrice) const {
    int marketPrice = GetCurrentMarketPrice(type);
    if (marketPrice <= 0) return "Normal";

    float ratio = ((float)playerSalePrice - (float)marketPrice) / (float)marketPrice * 100.0f;
    if (ratio <= -15.0f) return "Sangat Murah (Bargain)";
    if (ratio <= -5.0f)  return "Lebih Murah (Below Market)";
    if (ratio <= 8.0f)   return "Harga Wajar (Near Market)";
    if (ratio <= 25.0f)  return "Agak Mahal (Above Market)";
    if (ratio <= 50.0f)  return "Mahal (Expensive)";
    return "Sangat Mahal (Overpriced)";
}

Color MarketSystem::GetPriceAttractivenessColor(ProductType type, int playerSalePrice) const {
    int marketPrice = GetCurrentMarketPrice(type);
    if (marketPrice <= 0) return RAYWHITE;

    float ratio = ((float)playerSalePrice - (float)marketPrice) / (float)marketPrice * 100.0f;
    if (ratio <= -5.0f)  return Color{ 46, 204, 113, 255 }; // Green
    if (ratio <= 8.0f)   return Color{ 52, 152, 219, 255 }; // Blue
    if (ratio <= 25.0f)  return Color{ 241, 196, 15, 255 }; // Yellow
    return Color{ 231, 76, 60, 255 }; // Red
}

int MarketSystem::CalculateUnitProfit(ProductType type, int playerSalePrice) const {
    int supplierPrice = GetCurrentSupplierPrice(type);
    return playerSalePrice - supplierPrice;
}

float MarketSystem::CalculateProfitMarginPercent(ProductType type, int playerSalePrice) const {
    if (playerSalePrice <= 0) return 0.0f;
    int profit = CalculateUnitProfit(type, playerSalePrice);
    return ((float)profit / (float)playerSalePrice) * 100.0f;
}

float MarketSystem::EvaluateCustomerBuyChance(ProductType type, int playerSalePrice, int customerTypeInt, float baseChance) const {
    int marketPrice = GetCurrentMarketPrice(type);
    if (marketPrice <= 0) return baseChance;

    float priceDiffPct = ((float)playerSalePrice - (float)marketPrice) / (float)marketPrice * 100.0f;
    float chance = baseChance;

    // Price Ratio Modifiers
    if (priceDiffPct <= -20.0f) {
        chance = 1.00f; // Extreme discount -> 100% buy
    } else if (priceDiffPct <= -5.0f) {
        chance = 0.98f; // Below market -> 98%
    } else if (priceDiffPct <= 5.0f) {
        chance = 0.92f; // Near market -> 92%
    } else if (priceDiffPct <= 20.0f) {
        chance = 0.80f; // Moderate -> 80%
    } else if (priceDiffPct <= 40.0f) {
        chance = 0.55f; // Expensive -> 55%
    } else if (priceDiffPct <= 70.0f) {
        chance = 0.28f; // High markup -> 28%
    } else if (priceDiffPct <= 100.0f) {
        chance = 0.12f; // Overpriced -> 12%
    } else {
        chance = 0.03f; // Excessive -> 3%
    }

    // Customer Persona Sensitivity
    // 0: NORMAL, 1: IMPATIENT, 2: PATIENT, 3: BIG_SHOPPER, 4: PRICE_SENSITIVE
    if (customerTypeInt == 4) { // PRICE_SENSITIVE
        if (priceDiffPct > 5.0f) chance *= 0.50f;
        if (priceDiffPct > 25.0f) chance *= 0.25f;
        if (priceDiffPct < 0.0f) chance = std::min(1.0f, chance + 0.12f);
    } else if (customerTypeInt == 3) { // BIG_SHOPPER
        if (priceDiffPct > 0.0f && priceDiffPct <= 25.0f) chance = std::min(0.95f, chance + 0.10f);
        if (priceDiffPct > 50.0f) chance *= 0.70f;
    } else if (customerTypeInt == 2) { // PATIENT
        if (priceDiffPct > 20.0f) chance *= 0.85f;
    } else if (customerTypeInt == 1) { // IMPATIENT
        if (priceDiffPct > 35.0f) chance *= 0.60f;
    }

    // Apply Demand factor (High demand boosts buying tolerance)
    int demand = GetDemand(type);
    if (demand >= 75) {
        chance = std::min(1.0f, chance * 1.15f);
    } else if (demand <= 30) {
        chance *= 0.88f;
    }

    return std::clamp(chance, 0.02f, 1.0f);
}

float MarketSystem::GetProductSelectionWeight(ProductType type) const {
    int demand = GetDemand(type);
    MarketTrend trend = GetTrend(type);

    float weight = (float)demand / 50.0f; // 1.0 at 50 demand
    if (trend == MarketTrend::RISING) weight *= 1.35f;
    else if (trend == MarketTrend::FALLING) weight *= 0.75f;

    return std::max(0.2f, weight);
}

void MarketSystem::SetEventModifier(ProductType type, int priceMod, int demandMod) {
    auto it = marketMap.find(type);
    if (it != marketMap.end()) {
        it->second.eventPriceModifier = priceMod;
        it->second.eventDemandModifier = demandMod;
    }
}

void MarketSystem::ClearEventModifiers() {
    for (auto& pair : marketMap) {
        pair.second.eventPriceModifier = 0;
        pair.second.eventDemandModifier = 0;
    }
}

void MarketSystem::ToggleMenu() {
    menuOpen = !menuOpen;
}

void MarketSystem::SetMenuOpen(bool open) {
    menuOpen = open;
}

void MarketSystem::NextProduct() {
    selectedProductIndex = (selectedProductIndex + 1) % productList.size();
}

void MarketSystem::PreviousProduct() {
    selectedProductIndex = (selectedProductIndex - 1 + (int)productList.size()) % productList.size();
}

ProductType MarketSystem::GetSelectedProductType() const {
    if (selectedProductIndex >= 0 && selectedProductIndex < (int)productList.size()) {
        return productList[selectedProductIndex];
    }
    return ProductType::NONE;
}

void MarketSystem::RenderUI(int screenWidth, int screenHeight, int currentBalance, int currentDay) {
    if (!menuOpen) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 0, 0, 0, 170 });

    int modalW = 860;
    int modalH = 550;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    // Main Modal Box
    DrawRectangle(modalX, modalY, modalW, modalH, Color{ 20, 26, 36, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 46, 204, 113, 255 });

    // Header
    DrawText("DINAMIKA PASAR & EKONOMI TOKO (MARKET SYSTEM)", modalX + 30, modalY + 20, 20, Color{ 46, 204, 113, 255 });
    std::string subHead = "Analisis tren pasar harian, permintaan konsumen (demand), harga pasar & margin keuntungan produk";
    DrawText(subHead.c_str(), modalX + 30, modalY + 46, 12, Color{ 170, 190, 210, 255 });

    // Table Header Bar
    int tableY = modalY + 75;
    DrawRectangle(modalX + 25, tableY, modalW - 50, 28, Color{ 30, 40, 55, 255 });
    DrawRectangleLines(modalX + 25, tableY, modalW - 50, 28, Color{ 65, 80, 100, 255 });

    DrawText("SKU / Produk", modalX + 35, tableY + 8, 12, Color{ 200, 220, 240, 255 });
    DrawText("Harga Pasar", modalX + 215, tableY + 8, 12, Color{ 200, 220, 240, 255 });
    DrawText("Harga Jual", modalX + 315, tableY + 8, 12, Color{ 200, 220, 240, 255 });
    DrawText("Demand / Tren", modalX + 415, tableY + 8, 12, Color{ 200, 220, 240, 255 });
    DrawText("Margin / Profit", modalX + 575, tableY + 8, 12, Color{ 200, 220, 240, 255 });
    DrawText("Daya Tarik", modalX + 715, tableY + 8, 12, Color{ 200, 220, 240, 255 });

    // Table Item Rows
    int rowY = tableY + 34;
    for (size_t i = 0; i < productList.size(); ++i) {
        ProductType type = productList[i];
        bool isSel = ((int)i == selectedProductIndex);
        ProductInfo info = GetProductInfo(type);
        int marketPrice = GetCurrentMarketPrice(type);
        int salePrice = PriceManager::Instance().GetSellPrice(type);
        int demandVal = GetDemand(type);
        MarketTrend trend = GetTrend(type);
        int profit = CalculateUnitProfit(type, salePrice);
        float marginPct = CalculateProfitMarginPercent(type, salePrice);

        Color rowBg = isSel ? Color{ 35, 65, 95, 230 } : (i % 2 == 0 ? Color{ 25, 32, 44, 200 } : Color{ 22, 28, 38, 200 });
        Color rowBorder = isSel ? Color{ 46, 204, 113, 255 } : Color{ 45, 55, 70, 200 };

        DrawRectangle(modalX + 25, rowY, modalW - 50, 42, rowBg);
        DrawRectangleLines(modalX + 25, rowY, modalW - 50, 42, rowBorder);

        // 1. SKU & Name
        std::string nameLabel = info.sku + " " + info.name;
        DrawText(nameLabel.c_str(), modalX + 35, rowY + 12, 13, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

        // 2. Current Market Price
        std::string mktPriceStr = "Rp" + std::to_string(marketPrice);
        DrawText(mktPriceStr.c_str(), modalX + 215, rowY + 12, 13, Color{ 140, 200, 255, 255 });

        // 3. Player Sale Price
        std::string salePriceStr = "Rp" + std::to_string(salePrice);
        DrawText(salePriceStr.c_str(), modalX + 315, rowY + 12, 13, Color{ 50, 255, 120, 255 });

        // 4. Demand Index & Trend Badge
        std::string demandStr = std::to_string(demandVal) + "/100 (" + (trend == MarketTrend::RISING ? "+Tren" : trend == MarketTrend::FALLING ? "-Tren" : "Pas") + ")";
        DrawText(demandStr.c_str(), modalX + 415, rowY + 12, 12, GetMarketTrendColor(trend));

        // 5. Profit per unit & Margin
        std::string marginStr = "+Rp" + std::to_string(profit) + " (" + TextFormat("%.0f%%", marginPct) + ")";
        DrawText(marginStr.c_str(), modalX + 575, rowY + 12, 12, profit >= 0 ? Color{ 255, 215, 0, 255 } : Color{ 231, 76, 60, 255 });

        // 6. Price Attractiveness status
        std::string attrLabel = GetPriceAttractivenessLabel(type, salePrice);
        DrawText(attrLabel.c_str(), modalX + 715, rowY + 12, 11, GetPriceAttractivenessColor(type, salePrice));

        rowY += 46;
    }

    // Selected Product Insight Box (Bottom)
    int detailY = modalY + 450;
    DrawRectangle(modalX + 25, detailY, modalW - 50, 50, Color{ 16, 22, 30, 240 });
    DrawRectangleLines(modalX + 25, detailY, modalW - 50, 50, Color{ 60, 75, 90, 255 });

    ProductType selType = GetSelectedProductType();
    if (selType != ProductType::NONE) {
        ProductInfo selInfo = GetProductInfo(selType);
        int selMarket = GetCurrentMarketPrice(selType);
        int selSupplier = GetCurrentSupplierPrice(selType);
        int selSale = PriceManager::Instance().GetSellPrice(selType);
        std::string popLabel = PriceManager::Instance().GetPopularityLevel(selType);

        std::string line1 = "Detail " + selInfo.name + " (" + selInfo.sku + ")  |  Harga Grosir Supplier: Rp" + std::to_string(selSupplier) +
                            "  |  Harga Referensi Pasar: Rp" + std::to_string(selMarket) + "  |  Status Penjualan: " + popLabel;
        DrawText(line1.c_str(), modalX + 38, detailY + 9, 12, Color{ 200, 230, 255, 255 });

        std::string line2 = "Kondisi Permintaan: " + GetDemandLabel(selType) + "  |  Harga Toko Anda: Rp" + std::to_string(selSale) +
                            " (" + GetPriceAttractivenessLabel(selType, selSale) + ")";
        DrawText(line2.c_str(), modalX + 38, detailY + 28, 12, GetPriceAttractivenessColor(selType, selSale));
    }

    // Footer Controls
    int footY = modalY + modalH - 32;
    DrawText("[W / S / Panah] Pilih Produk    [P] Sesuaikan Harga Jual    [TAB] Beli di Supplier    [ESC / J] Tutup",
             modalX + 35, footY, 12, Color{ 255, 220, 120, 255 });
}

std::vector<MarketSystem::MarketSaveEntry> MarketSystem::ExportMarketSaveData() const {
    std::vector<MarketSaveEntry> list;
    for (const auto& kv : marketMap) {
        MarketSaveEntry entry;
        entry.productType = static_cast<int>(kv.first);
        entry.currentMarketPrice = kv.second.currentMarketPrice;
        entry.currentSupplierPrice = kv.second.currentSupplierPrice;
        entry.demand = kv.second.demand;
        entry.trend = static_cast<int>(kv.second.trend);
        list.push_back(entry);
    }
    return list;
}

void MarketSystem::ImportMarketSaveData(const std::vector<MarketSaveEntry>& data) {
    for (const auto& entry : data) {
        ProductType type = static_cast<ProductType>(entry.productType);
        auto it = marketMap.find(type);
        if (it != marketMap.end()) {
            it->second.currentMarketPrice = entry.currentMarketPrice;
            it->second.currentSupplierPrice = entry.currentSupplierPrice;
            it->second.demand = entry.demand;
            it->second.trend = static_cast<MarketTrend>(entry.trend);
            it->second.demandMultiplier = 0.5f + ((float)entry.demand / 100.0f) * 1.2f;
        }
    }
}
