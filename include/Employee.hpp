#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <string>
#include <vector>
#include "raylib.h"

enum class EmployeeRole {
    CASHIER = 0,
    STOCKER,
    CLEANER
};

enum class EmployeeStatus {
    AVAILABLE = 0, // In candidate pool
    WORKING,       // Hired and on duty
    OFF,           // Off-shift / rested
    FIRED          // Terminated
};

enum class EmployeeTaskState {
    IDLE = 0,
    // Cashier Tasks
    CASHIER_WAITING,
    CASHIER_SERVING,
    CASHIER_RETURNING,
    // Stocker Tasks
    STOCKER_HEADING_TO_STORAGE,
    STOCKER_PICKING_PRODUCT,
    STOCKER_HEADING_TO_SHELF,
    STOCKER_RESTOCKING,
    STOCKER_RETURNING,
    // Cleaner Tasks
    CLEANER_SEARCHING,
    CLEANER_HEADING_TO_SPOT,
    CLEANER_CLEANING,
    CLEANER_RETURNING
};

struct Employee {
    int id;                     // e.g. 1 -> "EMP-001"
    std::string name;           // e.g. "Andi"
    EmployeeRole role;          // CASHIER, STOCKER, CLEANER
    int salary;                 // e.g. 50000 / day
    int hiringCost;             // e.g. 100000 one-time
    int level;                  // 1 - 5
    int experience;             // 0 - 100
    int skill;                  // 0 - 100 (e.g. 75)
    int morale;                 // 0 - 100 (e.g. 85)
    int productivity;           // 0 - 100 (e.g. 80)
    EmployeeStatus status;      // AVAILABLE, WORKING, OFF, FIRED
    bool isHired;               // true if active employee

    // Visual appearance & Animation (Stage 16 & 17)
    Vector3 position;
    Vector3 velocity;
    float rotationY;
    Color skinColor;
    Color shirtColor;
    Color pantsColor;
    Color accessoryColor;
    float heightScale;
    float idleTimer;

    // Task & Automation State (Stage 17)
    EmployeeTaskState taskState;
    Vector3 homePosition;
    float homeRotationY;
    float taskTimer;
    float stuckTimer;
    Vector3 lastStuckCheckPos;

    // Stocker specific
    ProductType carriedProduct;
    int carriedQuantity;
    int targetRackIndex;

    // Cleaner specific
    Vector3 cleanTargetSpot;
    int cleanSpotIndex;

    // Waypoints for smooth movement
    std::vector<Vector3> waypoints;
    size_t currentWaypointIndex;

    // Speech / Dialogue bubble
    std::string speechText;
    float speechTimer;
    Color speechBubbleColor;
    Color speechTextColor;

    std::string GetIdString() const;
    std::string GetRoleString() const;
    std::string GetStatusString() const;
    std::string GetSkillName() const;
    std::string GetCurrentTaskString() const;

    // Experience & Level Up
    bool AddExperience(int amount, std::string& outLevelUpMsg);

    // Dialogue helper
    void Say(const std::string& text, float duration = 2.5f, Color bubbleColor = Color{ 245, 245, 250, 245 }, Color textColor = Color{ 20, 25, 35, 255 });
    bool HasActiveDialogue() const { return speechTimer > 0.0f && !speechText.empty(); }
};

