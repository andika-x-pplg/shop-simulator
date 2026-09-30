#include "RandomEventManager.hpp"
#include <algorithm>
#include <cstdio>

RandomEventManager& RandomEventManager::Instance() {
    static RandomEventManager instance;
    return instance;
}

RandomEventManager::RandomEventManager()
    : menuOpen(false),
      lastRolledDay(0),
      eventCooldownTimer(0),
      lastMajorEvent(RandomEventType::NONE)
{
    Init();
}

void RandomEventManager::Init() {
    activeEvent = { RandomEventType::NONE, "", "", "", ProductType::NONE, 0.0f, 0.0f, false, RAYWHITE };
    dailyChallenge = { ChallengeType::NONE, "", "", 0, 0, ProductType::NONE, 0, 0, false, false, false };
    history.clear();
    menuOpen = false;
    lastRolledDay = 0;
    eventCooldownTimer = 0;
    lastMajorEvent = RandomEventType::NONE;
}

float RandomEventManager::GetCustomerSpawnRateMultiplier() const {
    if (activeEvent.type == RandomEventType::BUSY_DAY) return 1.45f;
    if (activeEvent.type == RandomEventType::CUSTOMER_RUSH) return 1.65f;
    if (activeEvent.type == RandomEventType::GREAT_SALES_DAY) return 1.30f;
    if (activeEvent.type == RandomEventType::QUIET_DAY) return 0.65f;
    return 1.0f;
}

float RandomEventManager::GetSupplierBuyPriceMultiplier(ProductType type) const {
    if (activeEvent.type == RandomEventType::SUPPLIER_DISCOUNT) {
        if (activeEvent.affectedProduct == ProductType::NONE || activeEvent.affectedProduct == type) {
            return 0.75f; // 25% discount
        }
    } else if (activeEvent.type == RandomEventType::SUPPLIER_PRICE_HIKE) {
        if (activeEvent.affectedProduct == ProductType::NONE || activeEvent.affectedProduct == type) {
            return 1.20f; // 20% surge
        }
    }
    return 1.0f;
}

float RandomEventManager::GetSupplierDeliveryTimeMultiplier() const {
    if (activeEvent.type == RandomEventType::SUPPLIER_DELAY) return 1.50f; // 50% longer wait
    return 1.0f;
}

float RandomEventManager::GetEmployeeProductivityMultiplier() const {
    if (activeEvent.type == RandomEventType::EMPLOYEE_BONUS_DAY) return 1.20f;
    if (activeEvent.type == RandomEventType::GREAT_SALES_DAY) return 1.10f;
    return 1.0f;
}

void RandomEventManager::TriggerEvent(RandomEventType type, float durationSeconds, ProductType targetProd) {
    activeEvent.type = type;
    activeEvent.durationRemaining = durationSeconds;
    activeEvent.totalDuration = durationSeconds;
    activeEvent.affectedProduct = targetProd;
    activeEvent.isDaily = (durationSeconds >= 600.0f); // Consider >= 10 real minutes as full day event

    switch (type) {
        case RandomEventType::BUSY_DAY:
            activeEvent.name = "Hari Ramai (Busy Day)";
            activeEvent.description = "Banyak pelanggan berdatangan ke toko hari ini!";
            activeEvent.effectDescription = "Frekuensi kedatangan customer meningkat +45%.";
            activeEvent.bannerColor = { 46, 204, 113, 240 };
            break;
        case RandomEventType::QUIET_DAY:
            activeEvent.name = "Hari Santai (Quiet Day)";
            activeEvent.description = "Suasana toko lebih tenang dari biasanya hari ini.";
            activeEvent.effectDescription = "Arus customer berkurang -35%, cocok untuk restock dan tata barang.";
            activeEvent.bannerColor = { 52, 152, 219, 240 };
            break;
        case RandomEventType::POPULAR_PRODUCT: {
            std::string pName = (targetProd != ProductType::NONE) ? GetProductInfo(targetProd).name : "Produk Tertentu";
            activeEvent.name = "Tren Populer: " + pName;
            activeEvent.description = pName + " sedang viral dan sangat diminati pembeli!";
            activeEvent.effectDescription = "Customer lebih sering memasukkan " + pName + " ke daftar belanja.";
            activeEvent.bannerColor = { 241, 196, 15, 240 };
            break;
        }
        case RandomEventType::SUPPLIER_DISCOUNT: {
            std::string pName = (targetProd != ProductType::NONE) ? GetProductInfo(targetProd).name : "Semua Produk";
            activeEvent.name = "Diskon Supplier (Wholesale Sale)";
            activeEvent.description = "Supplier memberikan potongan harga khusus untuk pengadaan barang!";
            activeEvent.effectDescription = "Harga beli supplier diskon -25% untuk " + pName + ".";
            activeEvent.bannerColor = { 39, 174, 96, 240 };
            break;
        }
        case RandomEventType::SUPPLIER_PRICE_HIKE: {
            std::string pName = (targetProd != ProductType::NONE) ? GetProductInfo(targetProd).name : "Bahan Pokok";
            activeEvent.name = "Kenaikan Harga Supplier";
            activeEvent.description = "Biaya produksi dari pabrik mengalami kenaikan sementara.";
            activeEvent.effectDescription = "Harga beli supplier naik +20% untuk " + pName + ".";
            activeEvent.bannerColor = { 231, 76, 60, 240 };
            break;
        }
        case RandomEventType::SUPPLIER_DELAY:
            activeEvent.name = "Kendala Logistik Supplier";
            activeEvent.description = "Kurir pengiriman logistik mengalami kepadatan rute.";
            activeEvent.effectDescription = "Waktu tunggu pengiriman pesanan bertambah +50%.";
            activeEvent.bannerColor = { 230, 126, 34, 240 };
            break;
        case RandomEventType::CUSTOMER_RUSH:
            activeEvent.name = "Serbuan Pembeli (Flash Rush)";
            activeEvent.description = "Rombongan pembeli mendadak datang ke toko secara serentak!";
            activeEvent.effectDescription = "Lonjakan customer +65% selama jam sibuk ini.";
            activeEvent.bannerColor = { 155, 89, 182, 240 };
            break;
        case RandomEventType::MESSY_DAY:
            activeEvent.name = "Hari Berdebu & Ramai (Messy Day)";
            activeEvent.description = "Aktivitas pembeli meninggalkan lebih banyak jejak kotoran di lantai.";
            activeEvent.effectDescription = "Lantai toko lebih cepat kotor, tugas ekstra untuk Cleaner!";
            activeEvent.bannerColor = { 120, 90, 60, 240 };
            break;
        case RandomEventType::GREAT_SALES_DAY:
            activeEvent.name = "Hari Belanja Bahagia (Great Sales Day)";
            activeEvent.description = "Customer memiliki kepuasan tinggi dan membeli lebih banyak!";
            activeEvent.effectDescription = "Customer lebih loyal dan staf toko bekerja lebih semangat (+10%).";
            activeEvent.bannerColor = { 243, 156, 18, 240 };
            break;
        case RandomEventType::EMPLOYEE_BONUS_DAY:
            activeEvent.name = "Semangat Staf Toko (Employee Morale Boost)";
            activeEvent.description = "Semua staf bekerja dengan antusiasme dan produktivitas tinggi!";
            activeEvent.effectDescription = "Kecepatan kerja seluruh karyawan meningkat +20%.";
            activeEvent.bannerColor = { 26, 188, 156, 240 };
            break;
        default:
            activeEvent.type = RandomEventType::NONE;
            break;
    }
}

void RandomEventManager::EndCurrentEvent(std::string& outSummary, Color& outColor) {
    if (activeEvent.type == RandomEventType::NONE) return;

    outSummary = "Event Berakhir: " + activeEvent.name;
    outColor = activeEvent.bannerColor;

    // Record into history
    EventHistoryEntry entry;
    entry.day = lastRolledDay;
    entry.eventName = activeEvent.name;
    entry.resultSummary = "Selesai beroperasi";
    entry.color = activeEvent.bannerColor;
    history.insert(history.begin(), entry);
    if (history.size() > 8) {
        history.pop_back();
    }

    lastMajorEvent = activeEvent.type;
    activeEvent.type = RandomEventType::NONE;
    activeEvent.durationRemaining = 0.0f;
}

void RandomEventManager::SetupChallenge(int currentDay) {
    auto allProds = GetAllProductTypes();
    int challengeIndex = (currentDay + 2) % 5;

    dailyChallenge.isCompleted = false;
    dailyChallenge.isFailed = false;
    dailyChallenge.rewardClaimed = false;
    dailyChallenge.currentValue = 0;

    switch (challengeIndex) {
        case 0:
            dailyChallenge.type = ChallengeType::SERVE_CUSTOMERS;
            dailyChallenge.targetValue = 8 + (currentDay % 4) * 2;
            dailyChallenge.targetProduct = ProductType::NONE;
            dailyChallenge.title = "Target Pelayanan Pembeli";
            dailyChallenge.description = "Layani total " + std::to_string(dailyChallenge.targetValue) + " pembeli sampai selesai belanja hari ini.";
            dailyChallenge.moneyReward = 15000 + (dailyChallenge.targetValue * 1000);
            dailyChallenge.reputationReward = 2;
            break;
        case 1:
            dailyChallenge.type = ChallengeType::EARN_REVENUE;
            dailyChallenge.targetValue = 60000 + (currentDay % 5) * 20000;
            dailyChallenge.targetProduct = ProductType::NONE;
            dailyChallenge.title = "Target Omzet Toko";
            dailyChallenge.description = "Raih pendapatan harian minimal Rp" + std::to_string(dailyChallenge.targetValue) + " hari ini.";
            dailyChallenge.moneyReward = 20000;
            dailyChallenge.reputationReward = 2;
            break;
        case 2:
            dailyChallenge.type = ChallengeType::RESTOCK_PRODUCTS;
            dailyChallenge.targetValue = 6 + (currentDay % 3) * 2;
            dailyChallenge.targetProduct = ProductType::NONE;
            dailyChallenge.title = "Operasional Restock Rak";
            dailyChallenge.description = "Isi ulang / restock " + std::to_string(dailyChallenge.targetValue) + " unit produk ke rak toko.";
            dailyChallenge.moneyReward = 12000;
            dailyChallenge.reputationReward = 1;
            break;
        case 3: {
            ProductType p = allProds[currentDay % allProds.size()];
            dailyChallenge.type = ChallengeType::SELL_SPECIFIC_ITEM;
            dailyChallenge.targetValue = 4 + (currentDay % 3);
            dailyChallenge.targetProduct = p;
            dailyChallenge.title = "Penjualan Khusus: " + GetProductInfo(p).name;
            dailyChallenge.description = "Berhasil menjual " + std::to_string(dailyChallenge.targetValue) + " unit " + GetProductInfo(p).name + " hari ini.";
            dailyChallenge.moneyReward = 18000;
            dailyChallenge.reputationReward = 2;
            break;
        }
        case 4:
        default:
            dailyChallenge.type = ChallengeType::KEEP_STORE_CLEAN;
            dailyChallenge.targetValue = 85;
            dailyChallenge.targetProduct = ProductType::NONE;
            dailyChallenge.title = "Toko Bersih & Nyaman";
            dailyChallenge.description = "Pertahankan kebersihan toko minimal 85% saat jam tutup pukul 21:00.";
            dailyChallenge.moneyReward = 15000;
            dailyChallenge.reputationReward = 2;
            break;
    }
}

void RandomEventManager::RollDailyEventAndChallenge(int currentDay, std::string& outNotification, Color& outNoticeColor) {
    lastRolledDay = currentDay;

    // 1. Roll Challenge for the day
    SetupChallenge(currentDay);

    // 2. Roll Daily Event with balanced probability (approx 45% event chance)
    int roll = GetRandomValue(1, 100);
    auto allProds = GetAllProductTypes();
    ProductType randomProd = allProds[GetRandomValue(0, (int)allProds.size() - 1)];

    if (roll <= 12) {
        TriggerEvent(RandomEventType::BUSY_DAY, 780.0f); // Lasts full day
    } else if (roll <= 20) {
        TriggerEvent(RandomEventType::POPULAR_PRODUCT, 780.0f, randomProd);
    } else if (roll <= 28) {
        TriggerEvent(RandomEventType::SUPPLIER_DISCOUNT, 780.0f, randomProd);
    } else if (roll <= 34) {
        TriggerEvent(RandomEventType::MESSY_DAY, 780.0f);
    } else if (roll <= 40) {
        TriggerEvent(RandomEventType::GREAT_SALES_DAY, 780.0f);
    } else if (roll <= 45) {
        TriggerEvent(RandomEventType::EMPLOYEE_BONUS_DAY, 780.0f);
    } else if (roll <= 50) {
        TriggerEvent(RandomEventType::QUIET_DAY, 780.0f);
    } else {
        // No major day-long event, but flash rush or logistics surge could occur later
        activeEvent.type = RandomEventType::NONE;
    }

    if (activeEvent.type != RandomEventType::NONE) {
        outNotification = "EVENT HARI INI: " + activeEvent.name + " (" + activeEvent.effectDescription + ")";
        outNoticeColor = activeEvent.bannerColor;
    }
}

void RandomEventManager::Update(float deltaTime, int currentDay, int currentHour, int currentMinute, bool isShopOpen, std::string& outNotification, Color& outNoticeColor) {
    // Check if new day started
    if (currentDay != lastRolledDay && isShopOpen && currentHour == 8 && currentMinute == 0) {
        RollDailyEventAndChallenge(currentDay, outNotification, outNoticeColor);
    }

    // Process ongoing event timer
    if (activeEvent.type != RandomEventType::NONE) {
        if (!activeEvent.isDaily) {
            activeEvent.durationRemaining -= deltaTime;
            if (activeEvent.durationRemaining <= 0.0f) {
                std::string summary;
                Color col;
                EndCurrentEvent(summary, col);
                outNotification = summary;
                outNoticeColor = col;
            }
        }
    } else if (isShopOpen && currentHour == 14 && currentMinute == 0) {
        // Mid-day random flash rush chance (15% chance if no event active)
        if (GetRandomValue(1, 100) <= 25) {
            TriggerEvent(RandomEventType::CUSTOMER_RUSH, 180.0f); // 3 minutes flash rush
            outNotification = "FLASH EVENT: " + activeEvent.name + "!";
            outNoticeColor = activeEvent.bannerColor;
        }
    }
}

void RandomEventManager::RecordCustomerServed() {
    if (dailyChallenge.type == ChallengeType::SERVE_CUSTOMERS && !dailyChallenge.isCompleted) {
        dailyChallenge.currentValue++;
        if (dailyChallenge.currentValue >= dailyChallenge.targetValue) {
            dailyChallenge.isCompleted = true;
        }
    }
}

void RandomEventManager::RecordRevenueEarned(int amount) {
    if (dailyChallenge.type == ChallengeType::EARN_REVENUE && !dailyChallenge.isCompleted) {
        dailyChallenge.currentValue += amount;
        if (dailyChallenge.currentValue >= dailyChallenge.targetValue) {
            dailyChallenge.isCompleted = true;
        }
    }
}

void RandomEventManager::RecordRestock(int count) {
    if (dailyChallenge.type == ChallengeType::RESTOCK_PRODUCTS && !dailyChallenge.isCompleted) {
        dailyChallenge.currentValue += count;
        if (dailyChallenge.currentValue >= dailyChallenge.targetValue) {
            dailyChallenge.isCompleted = true;
        }
    }
}

void RandomEventManager::RecordProductSold(ProductType type, int count) {
    if (dailyChallenge.type == ChallengeType::SELL_SPECIFIC_ITEM && !dailyChallenge.isCompleted) {
        if (dailyChallenge.targetProduct == type) {
            dailyChallenge.currentValue += count;
            if (dailyChallenge.currentValue >= dailyChallenge.targetValue) {
                dailyChallenge.isCompleted = true;
            }
        }
    }
}

void RandomEventManager::CheckCleanlinessChallenge(int currentCleanliness) {
    if (dailyChallenge.type == ChallengeType::KEEP_STORE_CLEAN && !dailyChallenge.isCompleted) {
        dailyChallenge.currentValue = currentCleanliness;
        if (currentCleanliness >= dailyChallenge.targetValue) {
            dailyChallenge.isCompleted = true;
        }
    }
}

void RandomEventManager::LoadEventState(int eventId, float duration, int targetProd, int chId, int chTarget, int chCurrent, int chProd, int chMoney, int chRep, bool chCompleted, bool chClaimed) {
    if (eventId > 0) {
        TriggerEvent((RandomEventType)eventId, duration, (ProductType)targetProd);
    } else {
        activeEvent.type = RandomEventType::NONE;
    }

    dailyChallenge.type = (ChallengeType)chId;
    dailyChallenge.targetValue = chTarget;
    dailyChallenge.currentValue = chCurrent;
    dailyChallenge.targetProduct = (ProductType)chProd;
    dailyChallenge.moneyReward = chMoney;
    dailyChallenge.reputationReward = chRep;
    dailyChallenge.isCompleted = chCompleted;
    dailyChallenge.rewardClaimed = chClaimed;
}

void RandomEventManager::RenderUI(int screenWidth, int screenHeight, int currentDay) {
    if (!menuOpen) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

    int modalW = 720;
    int modalH = 500;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 42, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 241, 196, 15, 255 });

    DrawText("RANDOM EVENTS & DAILY CHALLENGES (STAGE 18)", modalX + 30, modalY + 18, 20, { 255, 215, 0, 255 });
    DrawText("Kejadian toko dinamis, kondisi supplier, tren produk, dan tantangan harian", modalX + 30, modalY + 42, 12, { 180, 195, 210, 255 });

    // 1. ACTIVE EVENT CARD
    int box1Y = modalY + 70;
    DrawRectangle(modalX + 30, box1Y, modalW - 60, 110, { 30, 36, 48, 240 });
    DrawRectangleLines(modalX + 30, box1Y, modalW - 60, 110, (activeEvent.type != RandomEventType::NONE) ? activeEvent.bannerColor : Color{ 70, 80, 95, 255 });

    if (activeEvent.type != RandomEventType::NONE) {
        DrawRectangle(modalX + 45, box1Y + 14, 8, 82, activeEvent.bannerColor);
        DrawText(("EVENT AKTIF: " + activeEvent.name).c_str(), modalX + 65, box1Y + 14, 16, activeEvent.bannerColor);
        DrawText(activeEvent.description.c_str(), modalX + 65, box1Y + 38, 13, RAYWHITE);
        DrawText(("Dampak: " + activeEvent.effectDescription).c_str(), modalX + 65, box1Y + 60, 12, { 100, 220, 255, 255 });

        std::string durStr = activeEvent.isDaily ? "Durasi: Sepanjang Hari Ini (08:00 - 21:00)" : "Sisa Waktu: " + std::to_string((int)activeEvent.durationRemaining) + " detik";
        DrawText(durStr.c_str(), modalX + 65, box1Y + 82, 11, { 255, 220, 120, 255 });
    } else {
        DrawText("TIDAK ADA EVENT KHUSUS HARI INI", modalX + 50, box1Y + 30, 15, { 180, 190, 205, 255 });
        DrawText("Operasional toko berjalan normal. Event acak dapat terjadi di hari berikutnya!", modalX + 50, box1Y + 58, 13, { 140, 150, 165, 255 });
    }

    // 2. DAILY CHALLENGE CARD
    int box2Y = modalY + 195;
    DrawRectangle(modalX + 30, box2Y, modalW - 60, 130, { 30, 36, 48, 240 });
    DrawRectangleLines(modalX + 30, box2Y, modalW - 60, 130, dailyChallenge.isCompleted ? Color{ 46, 204, 113, 255 } : Color{ 52, 152, 219, 255 });

    if (dailyChallenge.type != ChallengeType::NONE) {
        std::string chStatus = dailyChallenge.isCompleted ? " [SELESAI / COMPLETED]" : " [BERJALAN]";
        Color statusCol = dailyChallenge.isCompleted ? Color{ 46, 204, 113, 255 } : Color{ 52, 152, 219, 255 };

        DrawText(("TANTANGAN HARIAN: " + dailyChallenge.title + chStatus).c_str(), modalX + 50, box2Y + 14, 15, statusCol);
        DrawText(dailyChallenge.description.c_str(), modalX + 50, box2Y + 38, 13, RAYWHITE);

        // Progress bar
        float progRatio = (dailyChallenge.targetValue > 0) ? std::min(1.0f, (float)dailyChallenge.currentValue / (float)dailyChallenge.targetValue) : 0.0f;
        int barW = modalW - 100;
        int barH = 14;
        DrawRectangle(modalX + 50, box2Y + 62, barW, barH, { 20, 25, 35, 255 });
        DrawRectangle(modalX + 50, box2Y + 62, (int)(barW * progRatio), barH, dailyChallenge.isCompleted ? Color{ 46, 204, 113, 255 } : Color{ 52, 152, 219, 255 });
        DrawRectangleLines(modalX + 50, box2Y + 62, barW, barH, RAYWHITE);

        std::string progStr = "Progress: " + std::to_string(dailyChallenge.currentValue) + " / " + std::to_string(dailyChallenge.targetValue) +
                              " (" + std::to_string((int)(progRatio * 100.0f)) + "%)";
        DrawText(progStr.c_str(), modalX + 50, box2Y + 82, 12, { 180, 220, 245, 255 });

        std::string rewardStr = "Hadiah Selesai: +Rp" + std::to_string(dailyChallenge.moneyReward) + " & +" + std::to_string(dailyChallenge.reputationReward) + " Reputasi";
        DrawText(rewardStr.c_str(), modalX + 50, box2Y + 102, 13, { 255, 215, 0, 255 });
    } else {
        DrawText("Belum ada tantangan harian aktif.", modalX + 50, box2Y + 45, 14, { 180, 190, 205, 255 });
    }

    // 3. RECENT EVENT HISTORY
    int box3Y = modalY + 340;
    DrawRectangle(modalX + 30, box3Y, modalW - 60, 105, { 20, 25, 32, 240 });
    DrawRectangleLines(modalX + 30, box3Y, modalW - 60, 105, { 60, 75, 90, 255 });

    DrawText("RIWAYAT EVENT TERAKHIR (EVENT HISTORY)", modalX + 45, box3Y + 10, 12, { 255, 215, 0, 255 });

    if (history.empty()) {
        DrawText("Belum ada riwayat event toko sebelumnya.", modalX + 45, box3Y + 45, 12, { 150, 160, 175, 255 });
    } else {
        int lineY = box3Y + 32;
        for (size_t i = 0; i < history.size() && i < 3; ++i) {
            std::string hText = "Hari " + std::to_string(history[i].day) + ": " + history[i].eventName + " (" + history[i].resultSummary + ")";
            DrawText(hText.c_str(), modalX + 45, lineY, 11, history[i].color);
            lineY += 22;
        }
    }

    // Footer Controls
    DrawText("[L / ESC] Tutup Menu Event & Tantangan", modalX + 45, modalY + 465, 12, { 255, 220, 120, 255 });
}
