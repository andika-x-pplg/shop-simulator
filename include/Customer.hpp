#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>

class Shop; // Forward declaration

enum class CustomerState {
    ENTERING,            // Walking from outside spawn to shop entrance door
    SELECTING_PRODUCT,   // In shop, finding an available rack with stock > 0
    WALKING_TO_SHELF,    // Walking through aisles to chosen rack
    AT_SHELF,            // Standing in front of the rack, browsing/choosing
    TAKING_PRODUCT,      // Picking up the product from rack (stock -= 1)
    GOING_TO_CASHIER,    // Walking from shelf to cashier queue area
    WAITING_FOR_CASHIER, // Standing in queue line waiting for turn
    PAYING,              // At counter processing transaction (money paid)
    LEAVING,             // Walking back to the door from cashier/inside
    EXITING,             // Walking from door to outside despawn point
    DESPAWNED            // Ready to be removed from memory
};

class Customer {
public:
    Customer();
    Customer(int id, const std::string& name, Vector3 spawnPos, Color bodyColor, Color shirtColor);
    ~Customer() = default;

    void SetTargetRack(Vector3 rackInteractionPos, int rackId, ProductType type);
    void Update(float deltaTime, Shop& shop, int queueIndex, bool& outDidPay, int& outPaidAmount, std::string& outPaidProduct);
    void Render();

    CustomerState GetState() const { return state; }
    std::string GetStateString() const;
    std::string GetName() const { return name; }
    int GetId() const { return id; }
    Vector3 GetPosition() const { return position; }
    bool IsDespawned() const { return state == CustomerState::DESPAWNED; }

    ProductType GetHeldProduct() const { return heldProduct; }
    std::string GetHeldProductName() const;
    bool IsHoldingProduct() const { return heldProduct != ProductType::NONE; }

    bool HasPaid() const { return hasPaid; }
    bool IsInCashierQueue() const {
        return state == CustomerState::GOING_TO_CASHIER ||
               state == CustomerState::WAITING_FOR_CASHIER ||
               state == CustomerState::PAYING;
    }

    // Tahap 8: Customer Satisfaction & Rating
    int GetSatisfaction() const { return satisfaction; }
    bool HasGivenRating() const { return hasGivenRating; }
    void MarkRatingGiven() { hasGivenRating = true; }
    float GetTotalQueueWaitTime() const { return totalQueueWaitTime; }
    bool DidSuccessfullyBuy() const { return hasPaid && heldProduct != ProductType::NONE; }

private:
    int id;
    std::string name;
    Vector3 position;
    Vector3 velocity;
    float rotationY; // Facing direction in radians

    float moveSpeed;
    CustomerState state;

    // Waypoints for navigating into/out of shop
    std::vector<Vector3> waypoints;
    size_t currentWaypointIndex;

    // Shelf target & browsing timer
    Vector3 targetRackPos;
    int targetRackId;
    ProductType targetProductType;
    float waitTimer;
    float maxWaitDuration;

    // Payment timer & flag (ensures transaction triggers exactly once)
    float payTimer;
    bool hasPaid;

    // Carried product inventory (Max 1 product)
    ProductType heldProduct;

    // Tahap 8: Satisfaction tracking & single-trigger penalty flags
    int satisfaction;              // 0 - 100 (Awal: 100)
    float totalQueueWaitTime;      // Waktu menunggu di antrean kasir (detik)
    bool penaltyWait5Applied;      // Penalti tunggu 5-10s (-5)
    bool penaltyWait10Applied;     // Penalti tunggu 10-20s (-10)
    bool penaltyWait20Applied;     // Penalti tunggu >20s (-20)
    bool penaltyNoStockApplied;    // Penalti produk habis (-20)
    bool penaltySwitchRackApplied; // Penalti produk habis lalu pindah rak (-10)
    bool bonusProductAcquired;     // Bonus berhasil dapat produk (+10)
    bool bonusPaymentSuccess;      // Bonus berhasil bayar kasir (+10)
    bool hasGivenRating;           // Memastikan rating hanya dicatat tepat 1 kali

    // Visual attributes
    Color bodyColor;  // Skin/head tone
    Color shirtColor; // Shirt color
    Color pantsColor; // Pants color
    float bobbingTimer; // Animation timer for walking step bob

    void MoveTowards(Vector3 target, float deltaTime);
    void BuildEntryWaypoints();
    void BuildExitWaypoints(Vector3 startPos);
    void BuildPathToCashier(Vector3 queueSlot);
    bool SelectAvailableRack(Shop& shop);
    void RenderHeldProduct();
};
