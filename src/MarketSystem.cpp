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
    : playerPopularity(40),
      playerMarketShare(25.0f),
      selectedProductIndex(0),
      currentTab(0),
      menuOpen(false)
{
    productList = GetAllProductTypes();
    Init();
}

void MarketSystem::InitializeCompetitors() {
    competitors.clear();

    // 1. Competitor A: "Toko Berkah Mart" (Aggressive Discount Store)
    CompetitorShop compA;
    compA.id = 1;
    compA.name = "Toko Berkah Mart";
    compA.description = "Minimarket lokal dengan strategi harga miring dan diskon massal.";
    compA.condition = "Hemat & Padat Pengunjung";
    compA.strategy = CompetitorStrategy::AGGRESSIVE;
    compA.reputation = 60;
    compA.popularity = 65;
    compA.customerAttraction = 50.0f;
    compA.marketShare = 30.0f;

    // 2. Competitor B: "Nusantara Fresh" (Balanced Superette)
    CompetitorShop compB;
    compB.id = 2;
    compB.name = "Nusantara Fresh";
    compB.description = "Toko kelontong modern yang mengutamakan kelengkapan barang & pelayanan stabil.";
    compB.condition = "Bersih & Ramai Stabil";
    compB.strategy = CompetitorStrategy::BALANCED;
    compB.reputation = 75;
    compB.popularity = 70;
    compB.customerAttraction = 60.0f;
    compB.marketShare = 35.0f;

    // 3. Competitor C: "Sentosa Premium Mart" (High-end Convenience Store)
    CompetitorShop compC;
    compC.id = 3;
    compC.name = "Sentosa Premium Mart";
    compC.description = "Toko swalayan berkelas dengan barang berkualitas premium dan margin tinggi.";
    compC.condition = "Eksklusif & Rapi";
    compC.strategy = CompetitorStrategy::PREMIUM;
    compC.reputation = 88;
    compC.popularity = 50;
    compC.customerAttraction = 45.0f;
    compC.marketShare = 10.0f;

    competitors.push_back(compA);
    competitors.push_back(compB);
    competitors.push_back(compC);

    // Initial Competitor Prices initialization
    for (auto& comp : competitors) {
        for (auto p : productList) {
            ProductInfo info = GetProductInfo(p);
            int baseP = info.sellPrice;
            if (comp.strategy == CompetitorStrategy::AGGRESSIVE) {
                comp.productPrices[p] = (int)(baseP * 0.90f / 100) * 100; // -10% rounded to 100s
            } else if (comp.strategy == CompetitorStrategy::PREMIUM) {
                comp.productPrices[p] = (int)(baseP * 1.15f / 100) * 100; // +15%
            } else {
                comp.productPrices[p] = (int)(baseP * 1.02f / 100) * 100; // +2%
            }
        }
    }
}

void MarketSystem::Init() {
    marketMap.clear();
    productList = GetAllProductTypes();
    selectedProductIndex = 0;
    currentTab = 0;
    menuOpen = false;
    playerPopularity = 45;
    playerMarketShare = 25.0f;

    // Reset Category Trends
    categoryTrends[ProductCategory::DRINK] = MarketTrend::STABLE;
    categoryTrends[ProductCategory::FOOD] = MarketTrend::STABLE;
    categoryTrends[ProductCategory::SNACK] = MarketTrend::STABLE;
    categoryTrends[ProductCategory::HOUSEHOLD] = MarketTrend::STABLE;

    // Reset Market Event
    activeEvent = { 0, "Pasar Normal", "Kondisi pasar berjalan seperti biasa tanpa anomali.", ProductCategory::FOOD, 0, 1.0f, 0, 0 };

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

    InitializeCompetitors();
    CalculateMarketShare(50); // Initial 50 rep
}

void MarketSystem::SimulateCompetitorPrices(int day) {
    for (size_t cIdx = 0; cIdx < competitors.size(); ++cIdx) {
        auto& comp = competitors[cIdx];
        
        for (auto p : productList) {
            ProductInfo info = GetProductInfo(p);
            int mktBase = marketMap[p].currentMarketPrice;
            int playerPrice = PriceManager::Instance().GetSellPrice(p);
            
            // Pseudo-random modifier per competitor & day
            int hash = (day * 43 + (int)cIdx * 29 + (int)p * 13) % 100;
            float variation = ((hash % 11) - 5) * 0.01f; // -5% to +5% natural shift

            int newPrice = mktBase;
            if (comp.strategy == CompetitorStrategy::AGGRESSIVE) {
                // Aggressive: Targets lower price, responds to player discounts
                float discountRate = 0.90f + variation;
                if (playerPrice < mktBase && (hash % 2 == 0)) {
                    // Match player price if player drops heavily
                    discountRate = std::min(discountRate, ((float)playerPrice / (float)mktBase) * 0.98f);
                }
                newPrice = (int)(mktBase * discountRate);
            } else if (comp.strategy == CompetitorStrategy::PREMIUM) {
                // Premium: Keeps premium markup (+10% to +20%)
                float premiumRate = 1.15f + variation;
                newPrice = (int)(mktBase * premiumRate);
            } else {
                // Balanced: Stays around market price (+/- 3%)
                float balancedRate = 1.00f + variation;
                newPrice = (int)(mktBase * balancedRate);
            }

            // Competitor floor: Never sells below supplier cost + 5%
            int supCost = marketMap[p].currentSupplierPrice;
            int floorPrice = (int)(supCost * 1.05f);
            newPrice = std::max(floorPrice, newPrice);

            // Round to nearest Rp100
            newPrice = (newPrice / 100) * 100;
            comp.productPrices[p] = newPrice;
        }

        // Slight natural drift in competitor reputation & popularity
        int repRoll = (day * 17 + (int)cIdx * 31) % 100;
        int repDelta = (repRoll < 20) ? 1 : (repRoll > 80 ? -1 : 0);
        comp.reputation = std::clamp(comp.reputation + repDelta, 40, 95);

        int popRoll = (day * 23 + (int)cIdx * 19) % 100;
        int popDelta = (popRoll < 25) ? 1 : (popRoll > 75 ? -1 : 0);
        comp.popularity = std::clamp(comp.popularity + popDelta, 35, 90);
    }
}

void MarketSystem::CalculateMarketShare(int playerReputation) {
    // 1. Calculate Player Attraction Score
    // Factors: Price Competitiveness, Reputation, Popularity
    float priceAttractivenessSum = 0.0f;
    int evaluatedCount = 0;
    
    for (auto p : productList) {
        int playerP = PriceManager::Instance().GetSellPrice(p);
        int avgCompP = GetAverageMarketPrice(p);
        if (avgCompP > 0) {
            float ratio = (float)avgCompP / (float)playerP; // >1.0 if player cheaper
            priceAttractivenessSum += std::clamp(ratio, 0.5f, 1.8f);
            evaluatedCount++;
        }
    }
    float playerPriceFactor = (evaluatedCount > 0) ? (priceAttractivenessSum / evaluatedCount) : 1.0f;
    
    // Player Attraction: weighted formula
    float playerAttraction = (playerPriceFactor * 35.0f) + 
                             ((float)playerReputation * 0.35f) + 
                             ((float)playerPopularity * 0.30f);
    playerAttraction = std::max(5.0f, playerAttraction);

    // 2. Calculate Competitors Attraction Scores
    float totalMarketAttraction = playerAttraction;
    for (auto& comp : competitors) {
        float compPriceSum = 0.0f;
        int cCount = 0;
        for (auto p : productList) {
            int compP = comp.GetPrice(p);
            int mktP = marketMap[p].currentMarketPrice;
            if (compP > 0) {
                float r = (float)mktP / (float)compP;
                compPriceSum += std::clamp(r, 0.5f, 1.8f);
                cCount++;
            }
        }
        float cPriceFactor = (cCount > 0) ? (compPriceSum / cCount) : 1.0f;
        comp.customerAttraction = (cPriceFactor * 35.0f) + 
                                  ((float)comp.reputation * 0.35f) + 
                                  ((float)comp.popularity * 0.30f);
        comp.customerAttraction = std::max(5.0f, comp.customerAttraction);
        totalMarketAttraction += comp.customerAttraction;
    }

    // 3. Compute Market Share Percentages (Strictly normalized to 100%)
    if (totalMarketAttraction > 0.0f) {
        playerMarketShare = (playerAttraction / totalMarketAttraction) * 100.0f;
        float remainingShare = 100.0f - playerMarketShare;
        
        float compAttractSum = 0.0f;
        for (const auto& comp : competitors) compAttractSum += comp.customerAttraction;

        for (auto& comp : competitors) {
            if (compAttractSum > 0.0f) {
                comp.marketShare = (comp.customerAttraction / compAttractSum) * remainingShare;
            } else {
                comp.marketShare = remainingShare / (float)competitors.size();
            }
        }
    }
}

void MarketSystem::TriggerRandomMarketEvent(int day) {
    if (activeEvent.remainingDays > 0) {
        activeEvent.remainingDays--;
        if (activeEvent.remainingDays <= 0) {
            activeEvent = { 0, "Pasar Normal", "Kondisi pasar berjalan stabil seperti biasa.", ProductCategory::FOOD, 0, 1.0f, 0, 0 };
        }
        return;
    }

    // Roll for a new event (30% chance every day transition)
    int eventRoll = (day * 53 + 7) % 100;
    if (eventRoll < 30) {
        int eventType = (eventRoll / 5) % 6;
        switch (eventType) {
            case 0:
                activeEvent = { 1, "Pekan Kuliner & Makanan", "Tingkat konsumsi makanan siap saji melonjak tinggi di seluruh kota!", ProductCategory::FOOD, 25, 1.10f, 3, 3 };
                break;
            case 1:
                activeEvent = { 2, "Festival Minuman Dingin", "Cuaca panas memicu lonjakan dahsyat pada pencarian minuman segar!", ProductCategory::DRINK, 30, 1.15f, 3, 3 };
                break;
            case 2:
                activeEvent = { 3, "Pekan Diskon Kompetitor", "Toko Berkah Mart menggelar diskon besar-besaran, persaingan harga memanas!", ProductCategory::SNACK, -10, 0.90f, 2, 2 };
                break;
            case 3:
                activeEvent = { 4, "Kelangkaan Pasokan Rumah Tangga", "Distribusi sabun & tisu tersendat, harga pasar meningkat!", ProductCategory::HOUSEHOLD, 20, 1.20f, 3, 3 };
                break;
            case 4:
                activeEvent = { 5, "Musim Liburan Sekolah", "Penjualan camilan biskuit dan minuman meningkat tajam oleh anak-anak & keluarga!", ProductCategory::SNACK, 25, 1.10f, 4, 4 };
                break;
            case 5:
            default:
                activeEvent = { 6, "Kenaikan Daya Beli Warga", "Peningkatan ekonomi warga lokal memicu belanja lebih banyak di semua sektor!", ProductCategory::FOOD, 15, 1.05f, 3, 3 };
                break;
        }
    }
}

void MarketSystem::UpdateDailyMarket(int day, int playerReputation, int dailyCustomersServed, int dailyRevenue, std::string& outSummary, Color& outColor) {
    // 1. Update Market Event Lifecycle
    TriggerRandomMarketEvent(day);

    // 2. Update Category Trends
    int drinkRoll = (day * 37 + 3) % 100;
    categoryTrends[ProductCategory::DRINK] = (drinkRoll < 25) ? MarketTrend::RISING : (drinkRoll > 75 ? MarketTrend::FALLING : MarketTrend::STABLE);

    int foodRoll = (day * 41 + 11) % 100;
    categoryTrends[ProductCategory::FOOD] = (foodRoll < 25) ? MarketTrend::RISING : (foodRoll > 75 ? MarketTrend::FALLING : MarketTrend::STABLE);

    int snackRoll = (day * 47 + 19) % 100;
    categoryTrends[ProductCategory::SNACK] = (snackRoll < 25) ? MarketTrend::RISING : (snackRoll > 75 ? MarketTrend::FALLING : MarketTrend::STABLE);

    int houseRoll = (day * 53 + 23) % 100;
    categoryTrends[ProductCategory::HOUSEHOLD] = (houseRoll < 25) ? MarketTrend::RISING : (houseRoll > 75 ? MarketTrend::FALLING : MarketTrend::STABLE);

    // 3. Update Individual Products
    std::string trendingProductName = "";
    for (size_t i = 0; i < productList.size(); ++i) {
        ProductType type = productList[i];
        auto& data = marketMap[type];

        MarketTrend catTrend = categoryTrends[data.category];
        int roll = (day * 31 + (int)i * 17 + (int)type * 7) % 100;
        
        // Align product trend with category trend with slight individual noise
        if (catTrend == MarketTrend::RISING || roll < 20) {
            data.trend = MarketTrend::RISING;
            if (trendingProductName.empty()) trendingProductName = data.name;
            data.demand = 65 + (roll % 26);
            float priceBonus = 0.05f + ((roll % 11) * 0.01f);
            data.dailyPriceFluctuation = (int)(data.baseMarketPrice * priceBonus);
        } else if (catTrend == MarketTrend::FALLING || roll > 80) {
            data.trend = MarketTrend::FALLING;
            data.demand = 20 + (roll % 26);
            float priceCut = 0.05f + ((roll % 8) * 0.01f);
            data.dailyPriceFluctuation = -(int)(data.baseMarketPrice * priceCut);
        } else {
            data.trend = MarketTrend::STABLE;
            data.demand = 45 + (roll % 16);
            data.dailyPriceFluctuation = ((roll % 5) - 2) * (data.baseMarketPrice / 100);
        }

        // Apply Market Event Modifier if applicable
        if (HasActiveMarketEvent() && activeEvent.affectedCategory == data.category) {
            data.demand = std::clamp(data.demand + activeEvent.demandModifier, 10, 100);
            data.dailyPriceFluctuation += (int)(data.baseMarketPrice * (activeEvent.priceModifier - 1.0f));
        }

        // Compute dynamic market price bounded safely (+/- 25% max from base)
        int minMarket = (int)(data.baseMarketPrice * 0.75f);
        int maxMarket = (int)(data.baseMarketPrice * 1.30f);
        data.currentMarketPrice = std::clamp(data.baseMarketPrice + data.dailyPriceFluctuation + data.eventPriceModifier, minMarket, maxMarket);

        // Compute dynamic supplier wholesale price (+/- 15% fluctuation with market)
        int supplierFluctuation = (int)(data.dailyPriceFluctuation * 0.6f);
        int minSupplier = (int)(data.baseSupplierPrice * 0.80f);
        int maxSupplier = (int)(data.baseSupplierPrice * 1.25f);
        data.currentSupplierPrice = std::clamp(data.baseSupplierPrice + supplierFluctuation, minSupplier, maxSupplier);

        // Compute Demand Multiplier (0.5x to 1.7x)
        int finalDemand = std::clamp(data.demand + data.eventDemandModifier, 5, 100);
        data.demandMultiplier = 0.5f + ((float)finalDemand / 100.0f) * 1.2f;
    }

    // 4. Update Competitor Prices & Behaviors
    SimulateCompetitorPrices(day);

    // 5. Update Player Popularity based on Daily Performance
    // Factors: Customers served, revenue, and reputation level
    int popChange = 0;
    if (dailyCustomersServed >= 12 && playerReputation >= 70) popChange += 3;
    else if (dailyCustomersServed >= 6 && playerReputation >= 50) popChange += 1;
    else if (dailyCustomersServed == 0 || playerReputation < 40) popChange -= 2;

    if (dailyRevenue >= 100000) popChange += 2;
    else if (dailyRevenue >= 50000) popChange += 1;

    playerPopularity = std::clamp(playerPopularity + popChange, 10, 100);

    // 6. Recalculate Market Share for all participants
    CalculateMarketShare(playerReputation);

    // 7. Generate Daily Summary Notice
    if (HasActiveMarketEvent()) {
        outSummary = "Laporan Pasar Hari " + std::to_string(day) + " [EVENT: " + activeEvent.name + " (" + std::to_string(activeEvent.remainingDays) + " hari)]! Market Share: " + TextFormat("%.0f%%", playerMarketShare);
        outColor = { 241, 196, 15, 255 };
    } else if (!trendingProductName.empty()) {
        outSummary = "Laporan Pasar Hari " + std::to_string(day) + ": " + trendingProductName + " sedang tren naik! Market Share: " + TextFormat("%.0f%%", playerMarketShare);
        outColor = { 46, 204, 113, 255 };
    } else {
        outSummary = "Laporan Pasar Hari " + std::to_string(day) + ": Pasar lokal stabil. Market Share: " + TextFormat("%.0f%%", playerMarketShare);
        outColor = { 52, 152, 219, 255 };
    }
}

int MarketSystem::GetAverageMarketPrice(ProductType type) const {
    if (competitors.empty()) return GetCurrentMarketPrice(type);
    int sum = 0;
    for (const auto& comp : competitors) {
        sum += comp.GetPrice(type, GetCurrentMarketPrice(type));
    }
    return sum / (int)competitors.size();
}

int MarketSystem::GetOverallAverageMarketPrice() const {
    if (productList.empty()) return 0;
    int total = 0;
    for (auto p : productList) {
        total += GetAverageMarketPrice(p);
    }
    return total / (int)productList.size();
}

int MarketSystem::GetPlayerAveragePrice() const {
    if (productList.empty()) return 0;
    int total = 0;
    for (auto p : productList) {
        total += PriceManager::Instance().GetSellPrice(p);
    }
    return total / (int)productList.size();
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

MarketTrend MarketSystem::GetCategoryTrend(ProductCategory cat) const {
    auto it = categoryTrends.find(cat);
    if (it != categoryTrends.end()) return it->second;
    return MarketTrend::STABLE;
}

int MarketSystem::GetCategoryDemandBonus(ProductCategory cat) const {
    if (HasActiveMarketEvent() && activeEvent.affectedCategory == cat) {
        return activeEvent.demandModifier;
    }
    MarketTrend tr = GetCategoryTrend(cat);
    if (tr == MarketTrend::RISING) return 15;
    if (tr == MarketTrend::FALLING) return -15;
    return 0;
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
    int avgMarketPrice = GetAverageMarketPrice(type);
    if (avgMarketPrice <= 0) return "Normal";

    float ratio = ((float)playerSalePrice - (float)avgMarketPrice) / (float)avgMarketPrice * 100.0f;
    if (ratio <= -15.0f) return "Sangat Murah (Bargain)";
    if (ratio <= -5.0f)  return "Lebih Murah (Below Market)";
    if (ratio <= 8.0f)   return "Harga Wajar (Competitive)";
    if (ratio <= 25.0f)  return "Agak Mahal (Above Market)";
    if (ratio <= 50.0f)  return "Mahal (Expensive)";
    return "Sangat Mahal (Overpriced)";
}

Color MarketSystem::GetPriceAttractivenessColor(ProductType type, int playerSalePrice) const {
    int avgMarketPrice = GetAverageMarketPrice(type);
    if (avgMarketPrice <= 0) return RAYWHITE;

    float ratio = ((float)playerSalePrice - (float)avgMarketPrice) / (float)avgMarketPrice * 100.0f;
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

float MarketSystem::EvaluateCustomerBuyChance(ProductType type, int playerSalePrice, int customerTypeInt, float baseChance, int playerReputation) const {
    int avgMarketPrice = GetAverageMarketPrice(type);
    if (avgMarketPrice <= 0) avgMarketPrice = GetCurrentMarketPrice(type);
    if (avgMarketPrice <= 0) return baseChance;

    // Price difference percentage compared to competitor average price
    float priceDiffPct = ((float)playerSalePrice - (float)avgMarketPrice) / (float)avgMarketPrice * 100.0f;
    float chance = baseChance;

    // 1. Base Price Sensitivity Curve
    if (priceDiffPct <= -20.0f) {
        chance = 1.00f; // Extreme discount -> 100% buy
    } else if (priceDiffPct <= -5.0f) {
        chance = 0.98f; // Below market -> 98%
    } else if (priceDiffPct <= 5.0f) {
        chance = 0.94f; // Competitive near market -> 94%
    } else if (priceDiffPct <= 20.0f) {
        chance = 0.82f; // Slight markup -> 82%
    } else if (priceDiffPct <= 40.0f) {
        chance = 0.58f; // Moderately expensive -> 58%
    } else if (priceDiffPct <= 70.0f) {
        chance = 0.32f; // High markup -> 32%
    } else if (priceDiffPct <= 100.0f) {
        chance = 0.15f; // Overpriced -> 15%
    } else {
        chance = 0.04f; // Excessive -> 4%
    }

    // 2. High Reputation Buffer: If player has stellar reputation, customer is more forgiving of slightly higher prices
    if (playerReputation >= 80) {
        if (priceDiffPct > 0.0f && priceDiffPct <= 30.0f) {
            chance = std::min(1.0f, chance + 0.12f); // +12% loyalty bonus
        }
    } else if (playerReputation < 40) {
        chance *= 0.85f; // Low reputation penalty
    }

    // 3. Customer Persona Sensitivity
    // 0: NORMAL, 1: IMPATIENT, 2: PATIENT, 3: BIG_SHOPPER, 4: PRICE_SENSITIVE
    if (customerTypeInt == 4) { // PRICE_SENSITIVE
        if (priceDiffPct > 5.0f) chance *= 0.50f;
        if (priceDiffPct > 25.0f) chance *= 0.25f;
        if (priceDiffPct < 0.0f) chance = std::min(1.0f, chance + 0.15f);
    } else if (customerTypeInt == 3) { // BIG_SHOPPER
        if (priceDiffPct > 0.0f && priceDiffPct <= 25.0f) chance = std::min(0.96f, chance + 0.08f);
        if (priceDiffPct > 50.0f) chance *= 0.70f;
    } else if (customerTypeInt == 2) { // PATIENT
        if (priceDiffPct > 20.0f) chance *= 0.88f;
    } else if (customerTypeInt == 1) { // IMPATIENT
        if (priceDiffPct > 35.0f) chance *= 0.65f;
    }

    // 4. Demand factor (High demand boosts buying tolerance)
    int demand = GetDemand(type);
    if (demand >= 75) {
        chance = std::min(1.0f, chance * 1.15f);
    } else if (demand <= 30) {
        chance *= 0.88f;
    }

    return std::clamp(chance, 0.02f, 1.0f);
}

float MarketSystem::GetCustomerAttractionMultiplier() const {
    // Multiplier for customer spawn rate: 0.7x to 1.5x based on popularity and market share
    float popFactor = (float)playerPopularity / 50.0f; // 1.0 at 50 popularity
    float shareFactor = playerMarketShare / 25.0f;     // 1.0 at 25% market share
    float mult = 0.6f + (popFactor * 0.4f) + (shareFactor * 0.3f);
    return std::clamp(mult, 0.7f, 1.6f);
}

float MarketSystem::GetProductSelectionWeight(ProductType type) const {
    int demand = GetDemand(type);
    MarketTrend trend = GetTrend(type);

    float weight = (float)demand / 50.0f; // 1.0 at 50 demand
    if (trend == MarketTrend::RISING) weight *= 1.35f;
    else if (trend == MarketTrend::FALLING) weight *= 0.75f;

    return std::max(0.2f, weight);
}

std::string MarketSystem::GetMarketEventSummary() const {
    if (!HasActiveMarketEvent()) return "Tidak ada event pasar aktif.";
    return activeEvent.name + " (" + std::to_string(activeEvent.remainingDays) + " hari tersisa) - " + activeEvent.description;
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
    productList = GetAllProductTypes();
    if (!productList.empty()) {
        selectedProductIndex = (selectedProductIndex + 1) % productList.size();
    }
}

void MarketSystem::PreviousProduct() {
    productList = GetAllProductTypes();
    if (!productList.empty()) {
        selectedProductIndex = (selectedProductIndex - 1 + (int)productList.size()) % productList.size();
    }
}

void MarketSystem::NextTab() {
    currentTab = (currentTab + 1) % 3;
}

void MarketSystem::PreviousTab() {
    currentTab = (currentTab - 1 + 3) % 3;
}

ProductType MarketSystem::GetSelectedProductType() const {
    if (selectedProductIndex >= 0 && selectedProductIndex < (int)productList.size()) {
        return productList[selectedProductIndex];
    }
    return ProductType::NONE;
}

void MarketSystem::RenderUI(int screenWidth, int screenHeight, int currentBalance, int currentDay, int playerReputation) {
    if (!menuOpen) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 0, 0, 0, 180 });

    int modalW = 920;
    int modalH = 570;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    // Main Modal Box
    DrawRectangle(modalX, modalY, modalW, modalH, Color{ 18, 24, 34, 252 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 46, 204, 113, 255 });

    // Header Title
    DrawText("PASAR LOKAL & KOMPETISI TOKO (STAGE 21)", modalX + 30, modalY + 16, 20, Color{ 46, 204, 113, 255 });
    std::string subHead = "Simulasi persaingan toko kompetitor, pangsa pasar (market share), reputasi, popularitas & tren harga";
    DrawText(subHead.c_str(), modalX + 30, modalY + 40, 12, Color{ 170, 190, 210, 255 });

    // Tabs Bar (Tab 0: Kompetitor & Market Share, Tab 1: Matriks Harga & Produk, Tab 2: Tren & Event Pasar)
    int tabY = modalY + 62;
    int tabW = 275;
    int tabH = 30;

    const char* tabNames[3] = {
        "1. Toko Kompetitor & Market Share",
        "2. Matriks Harga & Produk Pasar",
        "3. Tren Kategori & Event Pasar"
    };

    for (int t = 0; t < 3; ++t) {
        int tX = modalX + 30 + t * (tabW + 12);
        bool isAct = (currentTab == t);
        DrawRectangle(tX, tabY, tabW, tabH, isAct ? Color{ 46, 204, 113, 220 } : Color{ 30, 40, 52, 200 });
        DrawRectangleLines(tX, tabY, tabW, tabH, isAct ? Color{ 255, 255, 255, 255 } : Color{ 60, 75, 90, 255 });
        DrawText(tabNames[t], tX + 12, tabY + 8, 12, isAct ? Color{ 15, 25, 20, 255 } : RAYWHITE);
    }

    // ==========================================
    // TAB 0: TOKO KOMPETITOR & MARKET SHARE
    // ==========================================
    if (currentTab == 0) {
        // Player Summary Card vs Market Summary
        int cardY = tabY + 42;
        int cardW = (modalW - 75) / 2;
        int cardH = 92;

        // Player Store Card
        DrawRectangle(modalX + 30, cardY, cardW, cardH, Color{ 25, 45, 65, 220 });
        DrawRectangleLines(modalX + 30, cardY, cardW, cardH, Color{ 52, 152, 219, 255 });
        DrawText("TOKO ANDA (PLAYER SHOP)", modalX + 42, cardY + 10, 14, Color{ 100, 220, 255, 255 });
        
        std::string pRepStr = "Reputasi: " + std::to_string(playerReputation) + "/100  |  Popularitas: " + std::to_string(playerPopularity) + "/100";
        DrawText(pRepStr.c_str(), modalX + 42, cardY + 34, 12, Color{ 241, 196, 15, 255 });

        std::string pShareStr = "Pangsa Pasar (Market Share): " + std::string(TextFormat("%.1f%%", playerMarketShare)) + "  |  Rata2 Harga: Rp" + std::to_string(GetPlayerAveragePrice());
        DrawText(pShareStr.c_str(), modalX + 42, cardY + 54, 12, Color{ 46, 204, 113, 255 });

        // Market Overview Card
        int mX = modalX + 30 + cardW + 15;
        DrawRectangle(mX, cardY, cardW, cardH, Color{ 35, 30, 48, 220 });
        DrawRectangleLines(mX, cardY, cardW, cardH, Color{ 155, 89, 182, 255 });
        DrawText("STATUS PASAR LOKAL (LOCAL MARKET)", mX + 12, cardY + 10, 14, Color{ 220, 160, 255, 255 });
        
        std::string mEvent = HasActiveMarketEvent() ? ("[Event: " + activeEvent.name + "]") : "[Event: Normal]";
        DrawText(mEvent.c_str(), mX + 12, cardY + 34, 12, HasActiveMarketEvent() ? Color{ 241, 196, 15, 255 } : Color{ 180, 190, 210, 255 });

        std::string mAvgStr = "Rata-rata Harga Pasar: Rp" + std::to_string(GetOverallAverageMarketPrice()) + "  |  Jumlah Kompetitor: " + std::to_string(competitors.size());
        DrawText(mAvgStr.c_str(), mX + 12, cardY + 54, 12, Color{ 200, 220, 240, 255 });

        // Competitors List Header
        int listHeaderY = cardY + cardH + 12;
        DrawText("PROFIL TOKO KOMPETITOR SEKITAR:", modalX + 30, listHeaderY, 13, Color{ 255, 220, 100, 255 });

        // Competitors 3-Column Cards
        int cCardY = listHeaderY + 22;
        int compCardW = (modalW - 80) / 3;
        int compCardH = 220;

        for (size_t i = 0; i < competitors.size() && i < 3; ++i) {
            const auto& comp = competitors[i];
            int cX = modalX + 30 + (int)i * (compCardW + 10);
            Color stratColor = GetCompetitorStrategyColor(comp.strategy);

            DrawRectangle(cX, cCardY, compCardW, compCardH, Color{ 24, 30, 42, 240 });
            DrawRectangleLines(cX, cCardY, compCardW, compCardH, stratColor);

            // Header Banner
            DrawRectangle(cX, cCardY, compCardW, 30, Color{ 32, 42, 58, 255 });
            DrawText(comp.name.c_str(), cX + 10, cCardY + 7, 13, Color{ 255, 255, 255, 255 });

            // Strategy Badge
            std::string stratStr = "Strategi: " + GetCompetitorStrategyName(comp.strategy);
            DrawText(stratStr.c_str(), cX + 10, cCardY + 38, 11, stratColor);

            // Condition
            std::string condStr = "Kondisi: " + comp.condition;
            DrawText(condStr.c_str(), cX + 10, cCardY + 56, 11, Color{ 180, 200, 220, 255 });

            // Rep & Pop Bars
            std::string repText = "Reputasi: " + std::to_string(comp.reputation) + "/100";
            DrawText(repText.c_str(), cX + 10, cCardY + 78, 11, RAYWHITE);
            DrawRectangle(cX + 10, cCardY + 94, compCardW - 20, 6, Color{ 40, 50, 65, 255 });
            DrawRectangle(cX + 10, cCardY + 94, (int)((compCardW - 20) * (comp.reputation / 100.0f)), 6, Color{ 241, 196, 15, 255 });

            std::string popText = "Popularitas: " + std::to_string(comp.popularity) + "/100";
            DrawText(popText.c_str(), cX + 10, cCardY + 108, 11, RAYWHITE);
            DrawRectangle(cX + 10, cCardY + 124, compCardW - 20, 6, Color{ 40, 50, 65, 255 });
            DrawRectangle(cX + 10, cCardY + 124, (int)((compCardW - 20) * (comp.popularity / 100.0f)), 6, Color{ 52, 152, 219, 255 });

            // Market Share & Average Price
            int avgP = comp.GetAveragePrice(productList);
            std::string avgPStr = "Rata2 Harga: Rp" + std::to_string(avgP);
            DrawText(avgPStr.c_str(), cX + 10, cCardY + 140, 11, Color{ 100, 230, 255, 255 });

            std::string shareText = "Pangsa Pasar: " + std::string(TextFormat("%.1f%%", comp.marketShare));
            DrawText(shareText.c_str(), cX + 10, cCardY + 160, 12, Color{ 46, 204, 113, 255 });
            DrawRectangle(cX + 10, cCardY + 178, compCardW - 20, 8, Color{ 40, 50, 65, 255 });
            DrawRectangle(cX + 10, cCardY + 178, (int)((compCardW - 20) * (comp.marketShare / 100.0f)), 8, Color{ 46, 204, 113, 255 });

            // Description
            DrawText(comp.description.c_str(), cX + 10, cCardY + 194, 9, Color{ 140, 160, 180, 255 });
        }
    }
    // ==========================================
    // TAB 1: MATRIKS HARGA & PRODUK PASAR
    // ==========================================
    else if (currentTab == 1) {
        int tableY = tabY + 38;
        DrawRectangle(modalX + 25, tableY, modalW - 50, 26, Color{ 30, 40, 55, 255 });
        DrawRectangleLines(modalX + 25, tableY, modalW - 50, 26, Color{ 65, 80, 100, 255 });

        DrawText("SKU / Produk", modalX + 35, tableY + 7, 11, Color{ 200, 220, 240, 255 });
        DrawText("Harga Anda", modalX + 195, tableY + 7, 11, Color{ 50, 255, 120, 255 });
        DrawText("Berkah Mart", modalX + 295, tableY + 7, 11, Color{ 230, 126, 34, 255 });
        DrawText("Nusantara", modalX + 395, tableY + 7, 11, Color{ 52, 152, 219, 255 });
        DrawText("Sentosa Mart", modalX + 495, tableY + 7, 11, Color{ 155, 89, 182, 255 });
        DrawText("Rata2 Pasar", modalX + 605, tableY + 7, 11, Color{ 241, 196, 15, 255 });
        DrawText("Daya Saing Harga", modalX + 720, tableY + 7, 11, Color{ 200, 220, 240, 255 });

        int rowY = tableY + 30;
        for (size_t i = 0; i < productList.size(); ++i) {
            ProductType type = productList[i];
            bool isSel = ((int)i == selectedProductIndex);
            ProductInfo info = GetProductInfo(type);
            int playerPrice = PriceManager::Instance().GetSellPrice(type);
            
            int comp0Price = (competitors.size() > 0) ? competitors[0].GetPrice(type) : 0;
            int comp1Price = (competitors.size() > 1) ? competitors[1].GetPrice(type) : 0;
            int comp2Price = (competitors.size() > 2) ? competitors[2].GetPrice(type) : 0;
            int avgMarket = GetAverageMarketPrice(type);

            Color rowBg = isSel ? Color{ 35, 65, 95, 230 } : (i % 2 == 0 ? Color{ 25, 32, 44, 200 } : Color{ 22, 28, 38, 200 });
            Color rowBorder = isSel ? Color{ 46, 204, 113, 255 } : Color{ 45, 55, 70, 200 };

            DrawRectangle(modalX + 25, rowY, modalW - 50, 36, rowBg);
            DrawRectangleLines(modalX + 25, rowY, modalW - 50, 36, rowBorder);

            // SKU & Name
            std::string nameLabel = info.sku + " " + info.name;
            DrawText(nameLabel.c_str(), modalX + 35, rowY + 11, 12, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

            // Player Price
            DrawText(("Rp" + std::to_string(playerPrice)).c_str(), modalX + 195, rowY + 11, 12, Color{ 50, 255, 120, 255 });

            // Comp A (Berkah)
            DrawText(("Rp" + std::to_string(comp0Price)).c_str(), modalX + 295, rowY + 11, 12, Color{ 230, 126, 34, 255 });

            // Comp B (Nusantara)
            DrawText(("Rp" + std::to_string(comp1Price)).c_str(), modalX + 395, rowY + 11, 12, Color{ 52, 152, 219, 255 });

            // Comp C (Sentosa)
            DrawText(("Rp" + std::to_string(comp2Price)).c_str(), modalX + 495, rowY + 11, 12, Color{ 155, 89, 182, 255 });

            // Average Market Price
            DrawText(("Rp" + std::to_string(avgMarket)).c_str(), modalX + 605, rowY + 11, 12, Color{ 241, 196, 15, 255 });

            // Attractiveness Label
            std::string attrLabel = GetPriceAttractivenessLabel(type, playerPrice);
            DrawText(attrLabel.c_str(), modalX + 720, rowY + 11, 11, GetPriceAttractivenessColor(type, playerPrice));

            rowY += 40;
        }

        // Bottom Selected Product Info
        ProductType selType = GetSelectedProductType();
        if (selType != ProductType::NONE) {
            int detailY = modalY + 440;
            DrawRectangle(modalX + 25, detailY, modalW - 50, 56, Color{ 16, 22, 30, 240 });
            DrawRectangleLines(modalX + 25, detailY, modalW - 50, 56, Color{ 60, 75, 90, 255 });

            ProductInfo selInfo = GetProductInfo(selType);
            int selAvg = GetAverageMarketPrice(selType);
            int selSup = GetCurrentSupplierPrice(selType);
            int selSale = PriceManager::Instance().GetSellPrice(selType);
            int profit = CalculateUnitProfit(selType, selSale);
            float margin = CalculateProfitMarginPercent(selType, selSale);

            std::string line1 = "Analisis " + selInfo.name + " (" + selInfo.sku + ")  |  Grosir Supplier: Rp" + std::to_string(selSup) +
                                "  |  Rata2 Kompetitor: Rp" + std::to_string(selAvg) + "  |  Margin: Rp" + std::to_string(profit) + " (" + TextFormat("%.0f%%", margin) + ")";
            DrawText(line1.c_str(), modalX + 38, detailY + 10, 12, Color{ 200, 230, 255, 255 });

            std::string line2 = "Harga Toko Anda: Rp" + std::to_string(selSale) + " -> " + GetPriceAttractivenessLabel(selType, selSale) +
                                "  |  Permintaan Pasar: " + GetDemandLabel(selType) + " (" + std::to_string(GetDemand(selType)) + "/100)";
            DrawText(line2.c_str(), modalX + 38, detailY + 30, 12, GetPriceAttractivenessColor(selType, selSale));
        }
    }
    // ==========================================
    // TAB 2: TREN KATEGORI & EVENT PASAR
    // ==========================================
    else if (currentTab == 2) {
        int contentY = tabY + 40;
        
        // Active Market Event Banner
        DrawRectangle(modalX + 30, contentY, modalW - 60, 80, HasActiveMarketEvent() ? Color{ 45, 38, 20, 240 } : Color{ 25, 32, 44, 240 });
        DrawRectangleLines(modalX + 30, contentY, modalW - 60, 80, HasActiveMarketEvent() ? Color{ 241, 196, 15, 255 } : Color{ 52, 152, 219, 255 });

        std::string evTitle = HasActiveMarketEvent() ? ("[EVENT PASAR AKTIF] " + activeEvent.name + " (" + std::to_string(activeEvent.remainingDays) + " Hari Tersisa)") : "[EVENT PASAR] Pasar Lokal Berjalan Normal";
        DrawText(evTitle.c_str(), modalX + 45, contentY + 12, 15, HasActiveMarketEvent() ? Color{ 255, 215, 0, 255 } : Color{ 100, 220, 255, 255 });
        DrawText(activeEvent.description.c_str(), modalX + 45, contentY + 36, 12, Color{ 220, 230, 240, 255 });

        if (HasActiveMarketEvent()) {
            std::string evMod = "Dampak Kategori: " + GetCategoryName(activeEvent.affectedCategory) + " | Demand: " + ((activeEvent.demandModifier >= 0) ? "+" : "") + std::to_string(activeEvent.demandModifier) + " | Pengaruh Harga: " + TextFormat("%.0f%%", (activeEvent.priceModifier - 1.0f) * 100);
            DrawText(evMod.c_str(), modalX + 45, contentY + 56, 11, Color{ 46, 204, 113, 255 });
        }

        // Category Trends Grid (4 categories)
        int gridY = contentY + 95;
        DrawText("TREN PERMINTAAN KATEGORI PRODUK:", modalX + 30, gridY, 13, Color{ 255, 220, 100, 255 });

        int cGridY = gridY + 22;
        int catCardW = (modalW - 90) / 4;
        int catCardH = 175;

        ProductCategory cats[4] = { ProductCategory::DRINK, ProductCategory::FOOD, ProductCategory::SNACK, ProductCategory::HOUSEHOLD };
        const char* catDesc[4] = { "Air mineral, teh botol, minuman dingin", "Roti tawar, mie instan, makanan kaleng", "Biskuit cokelat, aneka camilan", "Sabun mandi, tisu wajah & kebersihan" };

        for (int c = 0; c < 4; ++c) {
            ProductCategory cat = cats[c];
            MarketTrend trend = GetCategoryTrend(cat);
            Color trendColor = GetMarketTrendColor(trend);
            int cX = modalX + 30 + c * (catCardW + 10);

            DrawRectangle(cX, cGridY, catCardW, catCardH, Color{ 24, 30, 42, 240 });
            DrawRectangleLines(cX, cGridY, catCardW, catCardH, trendColor);

            DrawRectangle(cX, cGridY, catCardW, 28, Color{ 32, 42, 58, 255 });
            DrawText(GetCategoryName(cat).c_str(), cX + 8, cGridY + 7, 12, RAYWHITE);

            std::string trStr = "Tren: " + GetMarketTrendName(trend);
            DrawText(trStr.c_str(), cX + 8, cGridY + 36, 11, trendColor);

            int dBonus = GetCategoryDemandBonus(cat);
            std::string dStr = "Mod Permintaan: " + ((dBonus >= 0) ? ("+" + std::to_string(dBonus)) : std::to_string(dBonus)) + "%";
            DrawText(dStr.c_str(), cX + 8, cGridY + 56, 11, dBonus >= 0 ? Color{ 46, 204, 113, 255 } : Color{ 231, 76, 60, 255 });

            DrawText(catDesc[c], cX + 8, cGridY + 80, 10, Color{ 160, 180, 200, 255 });

            // Strategic Tip
            if (trend == MarketTrend::RISING) {
                DrawText("Saran: Naikkan harga sedikit / stok lebih banyak!", cX + 8, cGridY + 120, 10, Color{ 255, 230, 100, 255 });
            } else if (trend == MarketTrend::FALLING) {
                DrawText("Saran: Berikan diskon untuk habiskan stok.", cX + 8, cGridY + 120, 10, Color{ 230, 126, 34, 255 });
            } else {
                DrawText("Saran: Pertahankan harga stabil kompetitif.", cX + 8, cGridY + 120, 10, Color{ 140, 200, 255, 255 });
            }
        }
    }

    // Footer Controls
    int footY = modalY + modalH - 32;
    DrawText("[Q / E / TAB] Ganti Tab    [W / S / Panah] Pilih Produk    [P] Sesuaikan Harga Jual    [ESC / J] Tutup",
             modalX + 35, footY, 12, Color{ 255, 220, 120, 255 });
}

MarketSystem::MarketStateSave MarketSystem::ExportFullSaveData() const {
    MarketStateSave save;
    save.playerPopularity = playerPopularity;
    save.playerMarketShare = playerMarketShare;
    save.categoryDrinkTrend = static_cast<int>(GetCategoryTrend(ProductCategory::DRINK));
    save.categoryFoodTrend = static_cast<int>(GetCategoryTrend(ProductCategory::FOOD));
    save.categorySnackTrend = static_cast<int>(GetCategoryTrend(ProductCategory::SNACK));
    save.categoryHouseholdTrend = static_cast<int>(GetCategoryTrend(ProductCategory::HOUSEHOLD));
    
    save.activeEventId = activeEvent.id;
    save.activeEventDaysLeft = activeEvent.remainingDays;

    for (const auto& kv : marketMap) {
        MarketSaveEntry entry;
        entry.productType = static_cast<int>(kv.first);
        entry.currentMarketPrice = kv.second.currentMarketPrice;
        entry.currentSupplierPrice = kv.second.currentSupplierPrice;
        entry.demand = kv.second.demand;
        entry.trend = static_cast<int>(kv.second.trend);
        save.marketEntries.push_back(entry);
    }

    for (const auto& comp : competitors) {
        CompetitorSaveEntry cEntry;
        cEntry.id = comp.id;
        cEntry.name = comp.name;
        cEntry.reputation = comp.reputation;
        cEntry.popularity = comp.popularity;
        cEntry.marketShare = comp.marketShare;
        for (const auto& pPair : comp.productPrices) {
            cEntry.prices.push_back({ static_cast<int>(pPair.first), pPair.second });
        }
        save.competitorEntries.push_back(cEntry);
    }

    return save;
}

void MarketSystem::ImportFullSaveData(const MarketStateSave& save) {
    playerPopularity = std::clamp(save.playerPopularity, 0, 100);
    playerMarketShare = save.playerMarketShare;
    
    categoryTrends[ProductCategory::DRINK] = static_cast<MarketTrend>(save.categoryDrinkTrend);
    categoryTrends[ProductCategory::FOOD] = static_cast<MarketTrend>(save.categoryFoodTrend);
    categoryTrends[ProductCategory::SNACK] = static_cast<MarketTrend>(save.categorySnackTrend);
    categoryTrends[ProductCategory::HOUSEHOLD] = static_cast<MarketTrend>(save.categoryHouseholdTrend);

    // Restore Event
    if (save.activeEventId > 0 && save.activeEventDaysLeft > 0) {
        switch (save.activeEventId) {
            case 1: activeEvent = { 1, "Pekan Kuliner & Makanan", "Tingkat konsumsi makanan siap saji melonjak tinggi di seluruh kota!", ProductCategory::FOOD, 25, 1.10f, 3, save.activeEventDaysLeft }; break;
            case 2: activeEvent = { 2, "Festival Minuman Dingin", "Cuaca panas memicu lonjakan dahsyat pada pencarian minuman segar!", ProductCategory::DRINK, 30, 1.15f, 3, save.activeEventDaysLeft }; break;
            case 3: activeEvent = { 3, "Pekan Diskon Kompetitor", "Toko Berkah Mart menggelar diskon besar-besaran, persaingan harga memanas!", ProductCategory::SNACK, -10, 0.90f, 2, save.activeEventDaysLeft }; break;
            case 4: activeEvent = { 4, "Kelangkaan Pasokan Rumah Tangga", "Distribusi sabun & tisu tersendat, harga pasar meningkat!", ProductCategory::HOUSEHOLD, 20, 1.20f, 3, save.activeEventDaysLeft }; break;
            case 5: activeEvent = { 5, "Musim Liburan Sekolah", "Penjualan camilan biskuit dan minuman meningkat tajam oleh anak-anak & keluarga!", ProductCategory::SNACK, 25, 1.10f, 4, save.activeEventDaysLeft }; break;
            case 6: default: activeEvent = { 6, "Kenaikan Daya Beli Warga", "Peningkatan ekonomi warga lokal memicu belanja lebih banyak di semua sektor!", ProductCategory::FOOD, 15, 1.05f, 3, save.activeEventDaysLeft }; break;
        }
    } else {
        activeEvent = { 0, "Pasar Normal", "Kondisi pasar berjalan stabil seperti biasa.", ProductCategory::FOOD, 0, 1.0f, 0, 0 };
    }

    // Restore Product Market Data
    for (const auto& entry : save.marketEntries) {
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

    // Restore Competitors
    for (const auto& cEntry : save.competitorEntries) {
        for (auto& comp : competitors) {
            if (comp.id == cEntry.id) {
                comp.reputation = cEntry.reputation;
                comp.popularity = cEntry.popularity;
                comp.marketShare = cEntry.marketShare;
                for (const auto& p : cEntry.prices) {
                    comp.productPrices[static_cast<ProductType>(p.first)] = p.second;
                }
                break;
            }
        }
    }
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
