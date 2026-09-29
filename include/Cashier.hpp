#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>

struct Transaction {
    int customerId;
    std::string customerName;
    ProductType product;
    int amount;
    float timestamp;
};

class Cashier {
public:
    Cashier();
    Cashier(Vector3 position, Vector3 size);
    ~Cashier() = default;

    void Init();
    void Update(float deltaTime);
    void Render();

    // Physical bounds
    Vector3 GetPosition() const { return position; }
    Vector3 GetSize() const { return size; }
    AABB GetCollider() const;

    // Queue slots in front of the counter
    // slot 0: Active payment position right at the counter
    // slot 1: 1st waiting position in line
    // slot 2: 2nd waiting position in line
    Vector3 GetQueuePosition(int queueIndex) const;

    // Transaction processing
    bool ProcessPayment(int customerId, const std::string& customerName, ProductType product, int& outAmount);

    // Register display & queue count helper
    int GetQueueCount() const { return currentQueueCount; }
    void SetQueueCount(int count) { currentQueueCount = count; }

    // Cashier NPC position
    Vector3 GetNpcPosition() const;

private:
    Vector3 position;
    Vector3 size;
    Color counterColor;
    Color counterTopColor;
    Color registerColor;
    Color registerScreenColor;

    // Cashier NPC visual properties
    Color npcSkinColor;
    Color npcUniformColor;
    Color npcCapColor;
    Color npcApronColor;
    float npcIdleTimer;

    int currentQueueCount;

    void RenderCashierNpc();
};
