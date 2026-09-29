#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>

class Shop; // Forward declaration

enum class CustomerState {
    ENTERING,          // Walking from outside spawn to shop entrance door
    SELECTING_PRODUCT, // In shop, finding an available rack with stock > 0
    WALKING_TO_SHELF,  // Walking through aisles to chosen rack
    AT_SHELF,          // Standing in front of the rack, browsing/choosing
    TAKING_PRODUCT,    // Picking up the product from rack (stock -= 1)
    LEAVING,           // Walking back to the door from inside while carrying product
    EXITING,           // Walking from door to outside despawn point
    DESPAWNED          // Ready to be removed from memory
};

class Customer {
public:
    Customer();
    Customer(int id, const std::string& name, Vector3 spawnPos, Color bodyColor, Color shirtColor);
    ~Customer() = default;

    void SetTargetRack(Vector3 rackInteractionPos, int rackId, ProductType type);
    void Update(float deltaTime, Shop& shop);
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

    // Carried product inventory (Max 1 product)
    ProductType heldProduct;

    // Visual attributes
    Color bodyColor;  // Skin/head tone
    Color shirtColor; // Shirt color
    Color pantsColor; // Pants color
    float bobbingTimer; // Animation timer for walking step bob

    void MoveTowards(Vector3 target, float deltaTime);
    void BuildEntryWaypoints();
    void BuildExitWaypoints();
    bool SelectAvailableRack(Shop& shop);
    void RenderHeldProduct();
};
