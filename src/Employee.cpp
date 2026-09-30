#include "Employee.hpp"
#include <cstdio>

std::string Employee::GetIdString() const {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "EMP-%03d", id);
    return std::string(buf);
}

std::string Employee::GetRoleString() const {
    switch (role) {
        case EmployeeRole::CASHIER: return "Cashier";
        case EmployeeRole::STOCKER: return "Stocker";
        case EmployeeRole::CLEANER: return "Cleaner";
        default: return "Employee";
    }
}

std::string Employee::GetStatusString() const {
    switch (status) {
        case EmployeeStatus::AVAILABLE: return "Tersedia (Candidate)";
        case EmployeeStatus::WORKING:   return "Bekerja (Working)";
        case EmployeeStatus::OFF:       return "Istirahat (Off)";
        case EmployeeStatus::FIRED:     return "Diberhentikan (Fired)";
        default: return "Active";
    }
}

std::string Employee::GetSkillName() const {
    switch (role) {
        case EmployeeRole::CASHIER: return "Cashier Speed & Accuracy";
        case EmployeeRole::STOCKER: return "Stocking Efficiency";
        case EmployeeRole::CLEANER: return "Cleaning & Store Care";
        default: return "Skill";
    }
}
