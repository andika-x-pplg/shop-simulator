#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>
#include "raylib.h"

enum class RandomEventType {
    NONE = 0,
    BUSY_DAY,            // High customer foot traffic (spawn rate +40%)
    QUIET_DAY,           // Low foot traffic (spawn rate -30%)
    POPULAR_PRODUCT,     // A random product is trending (customers prioritize buying it)
    SUPPLIER_DISCOUNT,   // Supplier wholesale discount (-25% buy price)
    SUPPLIER_PRICE_HIKE, // Supplier wholesale price surge (+20% buy price)
    SUPPLIER_DELAY,      // Logistics delay (shipments take 50% longer)
    CUSTOMER_RUSH,       // Flash rush hour of eager customers
    MESSY_DAY,           // High footprint litter (more dirty spots for cleaner)
    GREAT_SALES_DAY,     // High customer satisfaction & spending willingness
    EMPLOYEE_BONUS_DAY   // Staff working in high morale (+15% speed/productivity)
};

enum class ChallengeType {
    NONE = 0,
    SERVE_CUSTOMERS,     // Serve X customers today
    EARN_REVENUE,        // Earn Rp X in revenue today
    RESTOCK_PRODUCTS,    // Restock X items to shelves
    SELL_SPECIFIC_ITEM,  // Sell X units of trending item
    KEEP_STORE_CLEAN     // Finish day with cleanliness >= 85%
};

struct ActiveEvent {
    RandomEventType type;
    std::string name;
    std::string description;
    std::string effectDescription;
    ProductType affectedProduct;
    float durationRemaining;  // Remaining duration in seconds
    float totalDuration;      // Initial duration
    bool isDaily;             // Lasts full day until 21:00
    Color bannerColor;
};

struct DailyChallenge {
    ChallengeType type;
    std::string title;
    std::string description;
    int targetValue;
    int currentValue;
    ProductType targetProduct;
    int moneyReward;
    int reputationReward;
    bool isCompleted;
    bool isFailed;
    bool rewardClaimed;
};

struct EventHistoryEntry {
    int day;
    std::string eventName;
    std::string resultSummary;
    Color color;
};

class RandomEventManager {
public:
    static RandomEventManager& Instance();

    void Init();
    void Update(float deltaTime, int currentDay, int currentHour, int currentMinute, bool isShopOpen, std::string& outNotification, Color& outNoticeColor);

    // Event Lifecycle
    void RollDailyEventAndChallenge(int currentDay, std::string& outNotification, Color& outNoticeColor);
    void TriggerEvent(RandomEventType type, float durationSeconds, ProductType targetProd = ProductType::NONE);
    void EndCurrentEvent(std::string& outSummary, Color& outColor);

    // Challenge Progression
    void RecordCustomerServed();
    void RecordRevenueEarned(int amount);
    void RecordRestock(int count);
    void RecordProductSold(ProductType type, int count);
    void CheckCleanlinessChallenge(int currentCleanliness);

    // Event Query Modifiers
    bool HasActiveEvent() const { return activeEvent.type != RandomEventType::NONE; }
    RandomEventType GetActiveEventType() const { return activeEvent.type; }
    const ActiveEvent& GetActiveEvent() const { return activeEvent; }
    
    float GetCustomerSpawnRateMultiplier() const;
    float GetSupplierBuyPriceMultiplier(ProductType type) const;
    float GetSupplierDeliveryTimeMultiplier() const;
    float GetEmployeeProductivityMultiplier() const;
    ProductType GetPopularProduct() const { return (activeEvent.type == RandomEventType::POPULAR_PRODUCT) ? activeEvent.affectedProduct : ProductType::NONE; }
    bool IsMessyDay() const { return activeEvent.type == RandomEventType::MESSY_DAY; }

    // Challenge Queries
    bool HasDailyChallenge() const { return dailyChallenge.type != ChallengeType::NONE; }
    const DailyChallenge& GetDailyChallenge() const { return dailyChallenge; }
    DailyChallenge& GetDailyChallenge() { return dailyChallenge; }

    // History & Cooldowns
    const std::vector<EventHistoryEntry>& GetHistory() const { return history; }

    // UI Menu Modal (L key)
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }
    void RenderUI(int screenWidth, int screenHeight, int currentDay);

    // Save / Load Support
    int GetActiveEventId() const { return (int)activeEvent.type; }
    float GetActiveEventDuration() const { return activeEvent.durationRemaining; }
    int GetActiveEventProduct() const { return (int)activeEvent.affectedProduct; }

    int GetChallengeId() const { return (int)dailyChallenge.type; }
    int GetChallengeTarget() const { return dailyChallenge.targetValue; }
    int GetChallengeCurrent() const { return dailyChallenge.currentValue; }
    int GetChallengeProduct() const { return (int)dailyChallenge.targetProduct; }
    int GetChallengeMoneyReward() const { return dailyChallenge.moneyReward; }
    int GetChallengeRepReward() const { return dailyChallenge.reputationReward; }
    bool IsChallengeCompleted() const { return dailyChallenge.isCompleted; }
    bool IsChallengeRewardClaimed() const { return dailyChallenge.rewardClaimed; }

    void LoadEventState(int eventId, float duration, int targetProd, int chId, int chTarget, int chCurrent, int chProd, int chMoney, int chRep, bool chCompleted, bool chClaimed);

private:
    RandomEventManager();
    ~RandomEventManager() = default;

    ActiveEvent activeEvent;
    DailyChallenge dailyChallenge;
    std::vector<EventHistoryEntry> history;

    bool menuOpen;
    int lastRolledDay;
    int eventCooldownTimer; // Days before certain major events can repeat
    RandomEventType lastMajorEvent;

    void SetupChallenge(int currentDay);
    void ClaimChallengeReward(std::string& outNotice, Color& outNoticeColor);
};
