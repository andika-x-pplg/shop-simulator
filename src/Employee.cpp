#include "Employee.hpp"
#include <cstdio>
#include <algorithm>

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

std::string Employee::GetCurrentTaskString() const {
    switch (taskState) {
        case EmployeeTaskState::IDLE:
            return "Siaga / Idle di Posisi";
        case EmployeeTaskState::CASHIER_WAITING:
            return "Menunggu Pembeli di Kasir";
        case EmployeeTaskState::CASHIER_SERVING:
            return "Melayani Checkout Pembeli";
        case EmployeeTaskState::CASHIER_RETURNING:
            return "Kembali ke Meja Kasir";
        case EmployeeTaskState::STOCKER_HEADING_TO_STORAGE:
            return "Menuju Gudang Mengambil Barang";
        case EmployeeTaskState::STOCKER_PICKING_PRODUCT:
            return "Mengambil Stok " + GetProductInfo(carriedProduct).name;
        case EmployeeTaskState::STOCKER_HEADING_TO_SHELF:
            return "Membawa " + GetProductInfo(carriedProduct).name + " ke Rak";
        case EmployeeTaskState::STOCKER_RESTOCKING:
            return "Mengisi Rak " + GetProductInfo(carriedProduct).name;
        case EmployeeTaskState::STOCKER_RETURNING:
            return "Kembali ke Area Gudang";
        case EmployeeTaskState::CLEANER_SEARCHING:
            return "Memeriksa Kebersihan Toko";
        case EmployeeTaskState::CLEANER_HEADING_TO_SPOT:
            return "Menuju Area Kotor";
        case EmployeeTaskState::CLEANER_CLEANING:
            return "Sedang Membersihkan Lantai";
        case EmployeeTaskState::CLEANER_RETURNING:
            return "Kembali ke Posisi Standby";
        default:
            return "Bekerja";
    }
}

bool Employee::AddExperience(int amount, std::string& outLevelUpMsg) {
    if (amount <= 0 || level >= 5) return false;

    experience += amount;
    int requiredXp = level * 100;

    if (experience >= requiredXp) {
        experience -= requiredXp;
        level++;
        // Small balanced stat progression (Stage 17)
        skill = std::min(100, skill + 4);
        productivity = std::min(100, productivity + 3);
        morale = std::min(100, morale + 5);

        outLevelUpMsg = name + " (" + GetRoleString() + ") Naik Level -> Level " + std::to_string(level) + "! (Skill: " + std::to_string(skill) + ")";
        Say("Aku naik level! Skill meningkat.", 3.0f, Color{ 255, 245, 200, 245 }, Color{ 20, 30, 40, 255 });
        return true;
    }
    return false;
}

void Employee::Say(const std::string& text, float duration, Color bubbleColor, Color textColor) {
    speechText = text;
    speechTimer = duration;
    speechBubbleColor = bubbleColor;
    speechTextColor = textColor;
}

