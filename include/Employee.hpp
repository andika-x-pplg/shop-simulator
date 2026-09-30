#pragma once
#include "Common.hpp"
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

    // Visual appearance (Stage 16)
    Vector3 position;
    float rotationY;
    Color skinColor;
    Color shirtColor;
    Color pantsColor;
    Color accessoryColor;
    float heightScale;
    float idleTimer;

    std::string GetIdString() const;
    std::string GetRoleString() const;
    std::string GetStatusString() const;
    std::string GetSkillName() const;
};
