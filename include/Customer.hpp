#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>
#include <map>

class Shop; // Forward declaration

// Tahap 14: Customer Personality Types
enum class CustomerType {
    NORMAL,           // Standard shopping behavior (1-2 items, normal patience)
    IMPATIENT,        // Low wait tolerance, leaves faster if queue is long, sensitive to waiting time
    PATIENT,          // High patience, willing to queue longer, slower satisfaction decay
    BIG_SHOPPER,      // Buys multiple items (2-4 items), larger shopping list
    PRICE_SENSITIVE   // Inspects store selling prices against baseline, skips or switches if overpriced
};

// Tahap 14: Customer AI State Machine
enum class CustomerState {
    ENTERING,            // Walking from outside spawn to shop entrance door
    BROWSING,            // In lobby deciding which item from shopping list to search next
    SEARCHING_PRODUCT,   // Finding shelf containing target item on list
    WALKING_TO_SHELF,    // Walking through aisles to chosen rack
    AT_SHELF,            // Standing in front of the rack, inspecting product & price
    TAKING_PRODUCT,      // Picking up the product from rack (stock -= 1)
    GOING_TO_CASHIER,    // Walking from shelf to cashier queue area
    WAITING_FOR_CASHIER, // Standing in queue line waiting for turn
    PAYING,              // At counter processing transactions (paying all cart items)
    LEAVING,             // Walking back to the door from cashier/inside
    EXITING,             // Walking from door to outside despawn point
    DESPAWNED            // Ready to be removed from memory
};

struct ShoppingItem {
    ProductType product;
    int desiredQuantity;
    int acquiredQuantity;
};

class Customer {
public:
    Customer();
    Customer(int id, const std::string& name, CustomerType type, Vector3 spawnPos, Color bodyColor, Color shirtColor);
    ~Customer() = default;

    void Update(float deltaTime, Shop& shop, int queueIndex, bool& outDidPay, int& outPaidAmount, std::string& outPaidProductSummary, int playerReputation = 50);
    void Render();

    CustomerState GetState() const { return state; }
    CustomerType GetCustomerType() const { return type; }
    std::string GetCustomerTypeString() const;
    std::string GetStateString() const;
    std::string GetName() const { return name; }
    int GetId() const { return id; }
    Vector3 GetPosition() const { return position; }
    bool IsDespawned() const { return state == CustomerState::DESPAWNED; }

    // Multi-item cart inspection
    const std::vector<ShoppingItem>& GetShoppingList() const { return shoppingList; }
    const std::vector<ProductType>& GetCarriedItems() const { return carriedItems; }
    int GetTotalCarriedCount() const { return (int)carriedItems.size(); }
    std::string GetCarriedSummaryString() const;
    bool IsHoldingProduct() const { return !carriedItems.empty(); }

    bool HasPaid() const { return hasPaid; }
    bool IsInCashierQueue() const {
        return state == CustomerState::GOING_TO_CASHIER ||
               state == CustomerState::WAITING_FOR_CASHIER ||
               state == CustomerState::PAYING;
    }

    // Tahap 8 & 14: Customer Satisfaction & Rating
    int GetSatisfaction() const { return satisfaction; }
    bool HasGivenRating() const { return hasGivenRating; }
    void MarkRatingGiven() { hasGivenRating = true; }
    float GetTotalQueueWaitTime() const { return totalQueueWaitTime; }
    bool DidSuccessfullyBuy() const { return hasPaid && !carriedItems.empty(); }
    std::string GetFeedbackMessage() const;

    // Speech Bubble & Dialogue System (Tahap 15 expansion)
    void Say(const std::string& text, float duration = 3.0f, Color bubbleColor = Color{ 245, 245, 250, 245 }, Color textColor = Color{ 20, 25, 35, 255 });
    bool HasActiveDialogue() const { return speechBubbleTimer > 0.0f && !speechBubbleText.empty(); }
    const std::string& GetSpeechText() const { return speechBubbleText; }
    float GetSpeechTimer() const { return speechBubbleTimer; }
    Color GetSpeechBubbleColor() const { return speechBubbleColor; }
    Color GetSpeechTextColor() const { return speechTextColor; }
    void RenderSpeechBubble2D(Camera3D camera, int screenWidth, int screenHeight);
    void RenderSpeechBubble();

    // Separation & crowd avoidance
    void ApplySeparation(const std::vector<Customer>& otherCustomers, float deltaTime);

    void GenerateShoppingList(ProductType popularProductPreference = ProductType::NONE);

private:
    int id;
    std::string name;
    CustomerType type;
    Vector3 position;
    Vector3 velocity;
    float rotationY; // Facing direction in radians

    float moveSpeed;
    CustomerState state;

    // Shopping List & Multi-item Cart (Tahap 14)
    std::vector<ShoppingItem> shoppingList;
    size_t currentShoppingItemIndex;
    std::vector<ProductType> carriedItems;

    // Waypoints for navigating into/out of shop
    std::vector<Vector3> waypoints;
    size_t currentWaypointIndex;

    // Shelf target & browsing timer
    Vector3 targetRackPos;
    int targetRackId;
    ProductType currentTargetProduct;
    float waitTimer;
    float maxWaitDuration;

    // Patience tolerance & thresholds (Tahap 14)
    float patienceMultiplier;
    float maxQueuePatience; // Max seconds willing to wait in line before rage-quitting

    // Payment timer & flag (ensures transaction triggers exactly once)
    float payTimer;
    bool hasPaid;

    // Speech Bubble & Dynamic Dialogue System
    std::string speechBubbleText;
    float speechBubbleTimer;
    float speechCooldown;
    Color speechBubbleColor;
    Color speechTextColor;

    // Satisfaction tracking & single-trigger penalty flags (Tahap 8 & 14)
    int satisfaction;              // 0 - 100 (Awal: 100)
    float totalQueueWaitTime;      // Waktu menunggu di antrean kasir (detik)
    bool penaltyWaitShortApplied;  // Penalti antre pendek
    bool penaltyWaitMidApplied;    // Penalti antre sedang
    bool penaltyWaitLongApplied;   // Penalti antre lama
    bool penaltyNoStockApplied;    // Penalti produk habis
    bool penaltyPriceTooHighApplied; // Penalti harga kemahalan (Price sensitive)
    bool bonusProductAcquired;     // Bonus berhasil dapat produk (+10)
    bool bonusPaymentSuccess;      // Bonus berhasil bayar kasir (+10)
    bool hasGivenRating;           // Memastikan rating hanya dicatat tepat 1 kali
    std::string specificFeedback;  // Custom feedback text based on experience

    // Visual attributes & accessories
    Color bodyColor;  // Skin/head tone
    Color shirtColor; // Shirt color
    Color pantsColor; // Pants color
    float heightScale;// Subtle height variation (0.92f - 1.08f)
    float bobbingTimer; // Animation timer for walking step bob

    void MoveTowards(Vector3 target, float deltaTime);
    void BuildEntryWaypoints();
    void BuildExitWaypoints(Vector3 startPos);
    void BuildPathToCashier(Vector3 queueSlot);
    bool SearchTargetRack(Shop& shop);
    void RenderCarriedItems();
};
