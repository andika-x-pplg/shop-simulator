#pragma once
#include "Common.hpp"
#include <string>
#include <vector>

enum class CustomerState {
    ENTERING,    // Walking from outside spawn to shop entrance door
    WALKING_TO_SHELF, // Walking through aisles to a targeted rack
    AT_SHELF,    // Standing in front of the rack, browsing/waiting
    LEAVING,     // Walking back to the door from inside
    EXITING,     // Walking from door to outside despawn point
    DESPAWNED    // Ready to be removed
};

class Customer {
public:
    Customer();
    Customer(int id, const std::string& name, Vector3 spawnPos, Color bodyColor, Color shirtColor);
    ~Customer() = default;

    void SetTargetRack(Vector3 rackInteractionPos, int rackId);
    void Update(float deltaTime);
    void Render();

    CustomerState GetState() const { return state; }
    std::string GetStateString() const;
    std::string GetName() const { return name; }
    int GetId() const { return id; }
    Vector3 GetPosition() const { return position; }
    bool IsDespawned() const { return state == CustomerState::DESPAWNED; }

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
    float waitTimer;
    float maxWaitDuration;

    // Visual attributes
    Color bodyColor;  // Skin/head tone
    Color shirtColor; // Shirt color
    Color pantsColor; // Pants color
    float bobbingTimer; // Animation timer for walking step bob

    void MoveTowards(Vector3 target, float deltaTime);
    void BuildEntryWaypoints();
    void BuildExitWaypoints();
};
