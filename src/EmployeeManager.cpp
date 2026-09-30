#include "EmployeeManager.hpp"
#include "Finance.hpp"
#include "DailyStats.hpp"
#include "Shop.hpp"
#include "ShopUpgrade.hpp"
#include "Customer.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include "raymath.h"

EmployeeManager& EmployeeManager::Instance() {
    static EmployeeManager instance;
    return instance;
}

EmployeeManager::EmployeeManager()
    : storeCleanliness(95.0f),
      dirtSpawnTimer(10.0f),
      taskDecisionInterval(0.3f),
      menuOpen(false),
      currentTab(EmployeeMenuTab::MY_EMPLOYEES),
      selectedIndex(0),
      confirmingFire(false),
      nextEmployeeId(1)
{
    Init();
}

void EmployeeManager::Init() {
    activeEmployees.clear();
    candidatePool.clear();
    menuOpen = false;
    currentTab = EmployeeMenuTab::MY_EMPLOYEES;
    selectedIndex = 0;
    confirmingFire = false;
    nextEmployeeId = 1;
    storeCleanliness = 95.0f;
    dirtSpawnTimer = 8.0f;
    taskDecisionInterval = 0.3f;

    InitCleanlinessSpots();
    GenerateDefaultCandidates();
}

void EmployeeManager::InitCleanlinessSpots() {
    dirtySpots.clear();
    // Default dirty spots located across store aisles & entrance
    dirtySpots.push_back({ { 0.0f, 0.02f, 4.0f }, 0.4f, false });
    dirtySpots.push_back({ { -2.5f, 0.02f, -1.0f }, 0.6f, false });
    dirtySpots.push_back({ { 2.5f, 0.02f, -2.0f }, 0.5f, false });
    dirtySpots.push_back({ { 0.0f, 0.02f, -6.0f }, 0.7f, false });
    dirtySpots.push_back({ { -2.5f, 0.02f, -7.0f }, 0.3f, false });
    dirtySpots.push_back({ { 2.5f, 0.02f, 3.0f }, 0.5f, false });
}

void EmployeeManager::GenerateDefaultCandidates() {
    candidatePool.clear();

    // 1. Andi - Cashier
    Employee e1;
    e1.id = nextEmployeeId++;
    e1.name = "Andi";
    e1.role = EmployeeRole::CASHIER;
    e1.salary = 50000;
    e1.hiringCost = 100000;
    e1.level = 2;
    e1.experience = 25;
    e1.skill = 75;
    e1.morale = 85;
    e1.productivity = 80;
    e1.status = EmployeeStatus::AVAILABLE;
    e1.isHired = false;
    e1.position = { -1.8f, 0.0f, 1.2f };
    e1.homePosition = { -1.8f, 0.0f, 1.2f };
    e1.homeRotationY = 0.0f;
    e1.rotationY = 0.0f;
    e1.skinColor = { 245, 210, 180, 255 };
    e1.shirtColor = { 41, 128, 185, 255 }; // Cashier blue uniform
    e1.pantsColor = { 44, 62, 80, 255 };
    e1.accessoryColor = { 241, 196, 15, 255 }; // Gold cashier badge
    e1.heightScale = 1.0f;
    e1.idleTimer = 0.0f;
    e1.taskState = EmployeeTaskState::IDLE;
    e1.taskTimer = 0.0f;
    e1.stuckTimer = 0.0f;
    e1.carriedProduct = ProductType::NONE;
    e1.carriedQuantity = 0;
    e1.targetRackIndex = -1;
    e1.cleanSpotIndex = -1;
    e1.speechTimer = 0.0f;
    candidatePool.push_back(e1);

    // 2. Budi - Stocker
    Employee e2;
    e2.id = nextEmployeeId++;
    e2.name = "Budi";
    e2.role = EmployeeRole::STOCKER;
    e2.salary = 40000;
    e2.hiringCost = 80000;
    e2.level = 1;
    e2.experience = 10;
    e2.skill = 65;
    e2.morale = 80;
    e2.productivity = 75;
    e2.status = EmployeeStatus::AVAILABLE;
    e2.isHired = false;
    e2.position = { 5.2f, 0.0f, -8.0f };
    e2.homePosition = { 5.2f, 0.0f, -8.0f };
    e2.homeRotationY = -PI / 2.0f;
    e2.rotationY = -PI / 2.0f;
    e2.skinColor = { 235, 195, 160, 255 };
    e2.shirtColor = { 230, 126, 34, 255 }; // Stocker orange uniform
    e2.pantsColor = { 50, 50, 50, 255 };
    e2.accessoryColor = { 192, 57, 43, 255 };
    e2.heightScale = 1.04f;
    e2.idleTimer = 1.2f;
    e2.taskState = EmployeeTaskState::IDLE;
    e2.taskTimer = 0.0f;
    e2.stuckTimer = 0.0f;
    e2.carriedProduct = ProductType::NONE;
    e2.carriedQuantity = 0;
    e2.targetRackIndex = -1;
    e2.cleanSpotIndex = -1;
    e2.speechTimer = 0.0f;
    candidatePool.push_back(e2);

    // 3. Citra - Cleaner
    Employee e3;
    e3.id = nextEmployeeId++;
    e3.name = "Citra";
    e3.role = EmployeeRole::CLEANER;
    e3.salary = 35000;
    e3.hiringCost = 70000;
    e3.level = 1;
    e3.experience = 0;
    e3.skill = 70;
    e3.morale = 90;
    e3.productivity = 78;
    e3.status = EmployeeStatus::AVAILABLE;
    e3.isHired = false;
    e3.position = { 2.5f, 0.0f, 7.5f };
    e3.homePosition = { 2.5f, 0.0f, 7.5f };
    e3.homeRotationY = PI;
    e3.rotationY = PI;
    e3.skinColor = { 255, 220, 190, 255 };
    e3.shirtColor = { 39, 174, 96, 255 }; // Cleaner green apron
    e3.pantsColor = { 45, 52, 54, 255 };
    e3.accessoryColor = { 46, 204, 113, 255 };
    e3.heightScale = 0.96f;
    e3.idleTimer = 2.4f;
    e3.taskState = EmployeeTaskState::IDLE;
    e3.taskTimer = 0.0f;
    e3.stuckTimer = 0.0f;
    e3.carriedProduct = ProductType::NONE;
    e3.carriedQuantity = 0;
    e3.targetRackIndex = -1;
    e3.cleanSpotIndex = -1;
    e3.speechTimer = 0.0f;
    candidatePool.push_back(e3);

    // 4. Doni - Cashier Pro
    Employee e4;
    e4.id = nextEmployeeId++;
    e4.name = "Doni";
    e4.role = EmployeeRole::CASHIER;
    e4.salary = 60000;
    e4.hiringCost = 125000;
    e4.level = 3;
    e4.experience = 40;
    e4.skill = 88;
    e4.morale = 82;
    e4.productivity = 85;
    e4.status = EmployeeStatus::AVAILABLE;
    e4.isHired = false;
    e4.position = { -2.6f, 0.0f, 1.2f };
    e4.homePosition = { -2.6f, 0.0f, 1.2f };
    e4.homeRotationY = 0.0f;
    e4.rotationY = 0.0f;
    e4.skinColor = { 220, 175, 140, 255 };
    e4.shirtColor = { 52, 73, 94, 255 };
    e4.pantsColor = { 30, 39, 46, 255 };
    e4.accessoryColor = { 243, 156, 18, 255 };
    e4.heightScale = 1.02f;
    e4.idleTimer = 0.7f;
    e4.taskState = EmployeeTaskState::IDLE;
    e4.taskTimer = 0.0f;
    e4.stuckTimer = 0.0f;
    e4.carriedProduct = ProductType::NONE;
    e4.carriedQuantity = 0;
    e4.targetRackIndex = -1;
    e4.cleanSpotIndex = -1;
    e4.speechTimer = 0.0f;
    candidatePool.push_back(e4);

    // 5. Eka - Stocker
    Employee e5;
    e5.id = nextEmployeeId++;
    e5.name = "Eka";
    e5.role = EmployeeRole::STOCKER;
    e5.salary = 45000;
    e5.hiringCost = 90000;
    e5.level = 2;
    e5.experience = 20;
    e5.skill = 72;
    e5.morale = 78;
    e5.productivity = 80;
    e5.status = EmployeeStatus::AVAILABLE;
    e5.isHired = false;
    e5.position = { 5.2f, 0.0f, -9.2f };
    e5.homePosition = { 5.2f, 0.0f, -9.2f };
    e5.homeRotationY = -PI / 2.0f;
    e5.rotationY = -PI / 2.0f;
    e5.skinColor = { 240, 205, 175, 255 };
    e5.shirtColor = { 211, 84, 0, 255 };
    e5.pantsColor = { 45, 52, 54, 255 };
    e5.accessoryColor = { 241, 196, 15, 255 };
    e5.heightScale = 0.98f;
    e5.idleTimer = 1.8f;
    e5.taskState = EmployeeTaskState::IDLE;
    e5.taskTimer = 0.0f;
    e5.stuckTimer = 0.0f;
    e5.carriedProduct = ProductType::NONE;
    e5.carriedQuantity = 0;
    e5.targetRackIndex = -1;
    e5.cleanSpotIndex = -1;
    e5.speechTimer = 0.0f;
    candidatePool.push_back(e5);
}

int EmployeeManager::GetMaxEmployeeCapacity(int shopUpgradeLevel) const {
    if (shopUpgradeLevel <= 1) return 2;
    if (shopUpgradeLevel == 2) return 4;
    return 6;
}

int EmployeeManager::GetTotalDailySalary() const {
    int total = 0;
    for (const auto& emp : activeEmployees) {
        if (emp.isHired && emp.status == EmployeeStatus::WORKING) {
            total += emp.salary;
        }
    }
    return total;
}

void EmployeeManager::SwitchTab() {
    if (currentTab == EmployeeMenuTab::MY_EMPLOYEES) {
        currentTab = EmployeeMenuTab::HIRE_CANDIDATES;
    } else {
        currentTab = EmployeeMenuTab::MY_EMPLOYEES;
    }
    selectedIndex = 0;
    confirmingFire = false;
}

void EmployeeManager::NextItem() {
    confirmingFire = false;
    int count = (currentTab == EmployeeMenuTab::MY_EMPLOYEES) ? (int)activeEmployees.size() : (int)candidatePool.size();
    if (count > 0) {
        selectedIndex = (selectedIndex + 1) % count;
    }
}

void EmployeeManager::PreviousItem() {
    confirmingFire = false;
    int count = (currentTab == EmployeeMenuTab::MY_EMPLOYEES) ? (int)activeEmployees.size() : (int)candidatePool.size();
    if (count > 0) {
        selectedIndex = (selectedIndex - 1 + count) % count;
    }
}

bool EmployeeManager::HireEmployee(int candidateIndex, Finance& finance, DailyStats& dailyStats, int maxCapacity, std::string& outFeedback) {
    if (candidateIndex < 0 || candidateIndex >= (int)candidatePool.size()) {
        outFeedback = "Pilihan kandidat tidak valid.";
        return false;
    }

    if ((int)activeEmployees.size() >= maxCapacity) {
        outFeedback = "Kapasitas karyawan toko penuh (" + std::to_string(activeEmployees.size()) + "/" + std::to_string(maxCapacity) + ")! Upgrade toko untuk menambah kapasitas.";
        return false;
    }

    Employee cand = candidatePool[candidateIndex];

    if (finance.GetCurrentBalance() < cand.hiringCost) {
        outFeedback = "Saldo tidak cukup untuk biaya rekrutmen Rp" + std::to_string(cand.hiringCost) + "!";
        return false;
    }

    // Pay hiring cost
    if (!finance.RecordExpense(cand.hiringCost, "Rekrutmen Karyawan: " + cand.name + " (" + cand.GetRoleString() + ")")) {
        outFeedback = "Gagal memproses pembayaran biaya rekrutmen.";
        return false;
    }

    dailyStats.RecordExpense(cand.hiringCost);

    // Add to active employees
    cand.isHired = true;
    cand.status = EmployeeStatus::WORKING;
    cand.taskState = EmployeeTaskState::IDLE;
    cand.taskTimer = 0.0f;
    activeEmployees.push_back(cand);

    // Remove from candidate pool
    candidatePool.erase(candidatePool.begin() + candidateIndex);

    if (selectedIndex >= (int)candidatePool.size() && !candidatePool.empty()) {
        selectedIndex = (int)candidatePool.size() - 1;
    }

    outFeedback = "Berhasil merekrut " + cand.name + " sebagai " + cand.GetRoleString() + "! (-Rp" + std::to_string(cand.hiringCost) + ")";
    return true;
}

bool EmployeeManager::FireEmployee(int employeeIndex, std::string& outFeedback) {
    if (employeeIndex < 0 || employeeIndex >= (int)activeEmployees.size()) {
        outFeedback = "Pilihan karyawan tidak valid.";
        return false;
    }

    Employee fired = activeEmployees[employeeIndex];

    // Release any reservations if cleaner was holding dirty spot
    if (fired.cleanSpotIndex >= 0 && fired.cleanSpotIndex < (int)dirtySpots.size()) {
        dirtySpots[fired.cleanSpotIndex].isReserved = false;
    }

    activeEmployees.erase(activeEmployees.begin() + employeeIndex);

    if (selectedIndex >= (int)activeEmployees.size() && !activeEmployees.empty()) {
        selectedIndex = (int)activeEmployees.size() - 1;
    }
    confirmingFire = false;

    outFeedback = "Karyawan " + fired.name + " (" + fired.GetRoleString() + ") telah diberhentikan (Fired).";
    return true;
}

bool EmployeeManager::ProcessDailySalaries(Finance& finance, DailyStats& dailyStats, std::string& outSummary) {
    int totalSalary = GetTotalDailySalary();
    if (totalSalary <= 0 || activeEmployees.empty()) {
        outSummary = "Tidak ada beban gaji karyawan hari ini.";
        return false;
    }

    finance.RecordExpense(totalSalary, "Gaji Harian " + std::to_string(activeEmployees.size()) + " Karyawan");
    dailyStats.RecordExpense(totalSalary);

    outSummary = "Gaji harian " + std::to_string(activeEmployees.size()) + " karyawan dibayar (-Rp" + std::to_string(totalSalary) + ").";
    return true;
}

void EmployeeManager::ClearAllEmployees() {
    activeEmployees.clear();
}

void EmployeeManager::AddEmployeeDirect(const Employee& emp) {
    activeEmployees.push_back(emp);
}

void EmployeeManager::SetCandidatesDirect(const std::vector<Employee>& candidates) {
    candidatePool = candidates;
}

void EmployeeManager::RefreshCandidates() {
    if (candidatePool.empty()) {
        GenerateDefaultCandidates();
    }
}

void EmployeeManager::AssignWorkplacePositions(Shop& shop) {
    int cashierCount = 0;
    int stockerCount = 0;
    int cleanerCount = 0;

    for (auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        if (emp.role == EmployeeRole::CASHIER) {
            emp.homePosition = { -1.8f - (float)cashierCount * 0.8f, 0.0f, 1.2f };
            emp.homeRotationY = 0.0f;
            cashierCount++;
        } else if (emp.role == EmployeeRole::STOCKER) {
            emp.homePosition = { 5.2f, 0.0f, -8.0f - (float)stockerCount * 1.2f };
            emp.homeRotationY = -PI / 2.0f;
            stockerCount++;
        } else if (emp.role == EmployeeRole::CLEANER) {
            emp.homePosition = { 2.5f + (float)cleanerCount * 1.0f, 0.0f, 7.5f };
            emp.homeRotationY = PI;
            cleanerCount++;
        }
    }
}

void EmployeeManager::BuildPath(Employee& emp, Vector3 targetPos) {
    emp.waypoints.clear();
    emp.currentWaypointIndex = 0;

    // Direct path with mid aisle waypoint if crossing center
    Vector3 start = emp.position;
    if (fabsf(start.x - targetPos.x) > 2.0f || fabsf(start.z - targetPos.z) > 4.0f) {
        // Use central corridor waypoint (X = 0.0f) to smoothly navigate shop aisles
        Vector3 mid1 = { 0.0f, 0.0f, start.z };
        Vector3 mid2 = { 0.0f, 0.0f, targetPos.z };
        emp.waypoints.push_back(mid1);
        emp.waypoints.push_back(mid2);
    }
    emp.waypoints.push_back(targetPos);
}

void EmployeeManager::MoveEmployeeTowards(Employee& emp, Vector3 targetPos, float speed, float deltaTime) {
    Vector3 dir = Vector3Subtract(targetPos, emp.position);
    dir.y = 0.0f;
    float dist = Vector3Length(dir);

    if (dist > 0.05f) {
        dir = Vector3Normalize(dir);
        emp.position.x += dir.x * speed * deltaTime;
        emp.position.z += dir.z * speed * deltaTime;
        emp.rotationY = atan2f(dir.x, dir.z);
    }
}

// -------------------------------------------------------------
// STAGE 17: CASHIER AUTOMATION
// -------------------------------------------------------------
void EmployeeManager::UpdateCashierEmployee(Employee& emp, float deltaTime, Shop& shop, std::vector<Customer>& customers, bool isShopOpen, std::string& outNotice, Color& outNoticeColor) {
    // If shop is closed, cashier stands idle at counter
    if (!isShopOpen) {
        if (Vector3Distance(emp.position, emp.homePosition) > 0.3f) {
            MoveEmployeeTowards(emp, emp.homePosition, 2.0f, deltaTime);
        } else {
            emp.position = emp.homePosition;
            emp.rotationY = emp.homeRotationY;
            emp.taskState = EmployeeTaskState::IDLE;
        }
        return;
    }

    // Check if there is a customer waiting in the queue
    bool hasQueue = (shop.GetCashier().GetQueueCount() > 0);
    Customer* payingCust = nullptr;
    for (auto& c : customers) {
        if (c.GetState() == CustomerState::PAYING) {
            payingCust = &c;
            break;
        }
    }

    if (payingCust != nullptr) {
        emp.taskState = EmployeeTaskState::CASHIER_SERVING;
        emp.rotationY = 0.0f; // Face customer

        emp.taskTimer += deltaTime;
        // Skill bonus: Higher cashier skill reduces processing wait time
        float skillFactor = (float)emp.skill / 100.0f; // 0.4 to 1.0
        
        if (emp.taskTimer >= 1.0f) {
            emp.taskTimer = 0.0f;
            if (emp.speechTimer <= 0.0f && (emp.id + (int)GetTime()) % 4 == 0) {
                const std::vector<std::string> cashierChat = {
                    "Totalnya saya hitung ya.",
                    "Terima kasih sudah berbelanja!",
                    "Pembayaran sedang diproses.",
                    "Silakan ditunggu sebentar.",
                    "Semoga harimu menyenangkan!"
                };
                emp.Say(cashierChat[emp.id % cashierChat.size()], 2.5f, Color{ 235, 245, 255, 245 }, Color{ 20, 60, 120, 255 });
            }

            // Award XP to Cashier
            std::string lvlMsg;
            if (emp.AddExperience(15, lvlMsg)) {
                outNotice = lvlMsg;
                outNoticeColor = { 46, 204, 113, 235 };
            }
        }
    } else if (hasQueue) {
        emp.taskState = EmployeeTaskState::CASHIER_WAITING;
        emp.rotationY = 0.0f;
        if (Vector3Distance(emp.position, emp.homePosition) > 0.2f) {
            MoveEmployeeTowards(emp, emp.homePosition, 2.2f, deltaTime);
        }
    } else {
        emp.taskState = EmployeeTaskState::IDLE;
        if (Vector3Distance(emp.position, emp.homePosition) > 0.2f) {
            MoveEmployeeTowards(emp, emp.homePosition, 2.0f, deltaTime);
        } else {
            emp.rotationY = emp.homeRotationY;
        }
    }
}

// -------------------------------------------------------------
// STAGE 17: STOCKER AUTOMATION
// -------------------------------------------------------------
void EmployeeManager::UpdateStockerEmployee(Employee& emp, float deltaTime, Shop& shop, bool isShopOpen, std::set<int>& reservedRacks, std::string& outNotice, Color& outNoticeColor) {
    if (!isShopOpen) {
        // Return to warehouse station when shop closes
        if (Vector3Distance(emp.position, emp.homePosition) > 0.3f) {
            MoveEmployeeTowards(emp, emp.homePosition, 2.2f, deltaTime);
        } else {
            emp.position = emp.homePosition;
            emp.rotationY = emp.homeRotationY;
            emp.taskState = EmployeeTaskState::IDLE;
            emp.carriedProduct = ProductType::NONE;
            emp.carriedQuantity = 0;
            emp.targetRackIndex = -1;
        }
        return;
    }

    float stockerSpeed = 2.4f + ((float)emp.skill / 100.0f) * 0.8f; // 2.4 to 3.2 m/s

    // State Machine
    switch (emp.taskState) {
        case EmployeeTaskState::IDLE: {
            // Search for rack with lowest stock where storage has available supply
            int bestRackIdx = -1;
            int lowestStock = 999;
            ProductType bestType = ProductType::NONE;

            auto& racks = shop.GetRacks();
            for (size_t i = 0; i < racks.size(); ++i) {
                if (reservedRacks.find((int)i) != reservedRacks.end()) continue; // Skip reserved rack

                int curStock = racks[i].GetStock();
                int maxStock = racks[i].GetMaxStock();
                ProductType pType = racks[i].GetProductType();

                if (curStock < maxStock && shop.GetStorage().GetStock(pType) > 0) {
                    // Priority: Empty rack first, then lowest percentage
                    if (curStock < lowestStock) {
                        lowestStock = curStock;
                        bestRackIdx = (int)i;
                        bestType = pType;
                    }
                }
            }

            if (bestRackIdx >= 0) {
                emp.targetRackIndex = bestRackIdx;
                emp.carriedProduct = bestType;
                reservedRacks.insert(bestRackIdx);
                emp.taskState = EmployeeTaskState::STOCKER_HEADING_TO_STORAGE;
                emp.taskTimer = 0.0f;

                // Move towards Storage pallet location
                Vector3 storageSpot = { 4.5f, 0.0f, -8.0f };
                BuildPath(emp, storageSpot);

                emp.Say("Mengecek rak " + GetProductInfo(bestType).name + ", ambil stok di gudang.", 2.2f);
            } else {
                // No restock needed -> Stand idle at warehouse
                if (Vector3Distance(emp.position, emp.homePosition) > 0.3f) {
                    MoveEmployeeTowards(emp, emp.homePosition, 2.0f, deltaTime);
                } else {
                    emp.rotationY = emp.homeRotationY;
                }
            }
            break;
        }

        case EmployeeTaskState::STOCKER_HEADING_TO_STORAGE: {
            if (emp.currentWaypointIndex < emp.waypoints.size()) {
                Vector3 targetWp = emp.waypoints[emp.currentWaypointIndex];
                MoveEmployeeTowards(emp, targetWp, stockerSpeed, deltaTime);
                if (Vector3Distance(emp.position, targetWp) < 0.35f) {
                    emp.currentWaypointIndex++;
                }
            } else {
                // Arrived at storage -> Pick product
                emp.taskState = EmployeeTaskState::STOCKER_PICKING_PRODUCT;
                emp.taskTimer = 0.0f;
            }
            break;
        }

        case EmployeeTaskState::STOCKER_PICKING_PRODUCT: {
            emp.taskTimer += deltaTime;
            float pickDuration = 1.2f - ((float)emp.skill / 200.0f); // 0.7s to 1.2s
            if (emp.taskTimer >= pickDuration) {
                // Take 1 to 2 units from storage
                int available = shop.GetStorage().GetStock(emp.carriedProduct);
                if (available > 0) {
                    int takeAmt = std::min(available, 2);
                    for (int k = 0; k < takeAmt; ++k) {
                        shop.GetStorage().TakeStock(emp.carriedProduct);
                    }
                    emp.carriedQuantity = takeAmt;
                    emp.taskState = EmployeeTaskState::STOCKER_HEADING_TO_SHELF;

                    // Build path to rack
                    Vector3 rackFront = shop.GetRackFrontPosition(emp.targetRackIndex);
                    BuildPath(emp, rackFront);
                    emp.Say("Mengambil " + std::to_string(takeAmt) + "x " + GetProductInfo(emp.carriedProduct).name + " untuk di-restock.", 2.2f);
                } else {
                    // Storage empty -> cancel
                    emp.taskState = EmployeeTaskState::IDLE;
                    emp.targetRackIndex = -1;
                    emp.carriedProduct = ProductType::NONE;
                }
            }
            break;
        }

        case EmployeeTaskState::STOCKER_HEADING_TO_SHELF: {
            if (emp.currentWaypointIndex < emp.waypoints.size()) {
                Vector3 targetWp = emp.waypoints[emp.currentWaypointIndex];
                MoveEmployeeTowards(emp, targetWp, stockerSpeed, deltaTime);
                if (Vector3Distance(emp.position, targetWp) < 0.35f) {
                    emp.currentWaypointIndex++;
                }
            } else {
                // Arrived in front of rack -> Start placing
                emp.taskState = EmployeeTaskState::STOCKER_RESTOCKING;
                emp.taskTimer = 0.0f;
            }
            break;
        }

        case EmployeeTaskState::STOCKER_RESTOCKING: {
            emp.taskTimer += deltaTime;
            float placeDuration = 1.0f - ((float)emp.skill / 250.0f); // 0.6s to 1.0s
            if (emp.taskTimer >= placeDuration) {
                auto& racks = shop.GetRacks();
                if (emp.targetRackIndex >= 0 && emp.targetRackIndex < (int)racks.size()) {
                    for (int k = 0; k < emp.carriedQuantity; ++k) {
                        racks[emp.targetRackIndex].PlaceProduct(emp.carriedProduct);
                    }
                    emp.Say("Rak " + GetProductInfo(emp.carriedProduct).name + " berhasil di-restock!", 2.2f, Color{ 235, 255, 240, 245 }, Color{ 20, 100, 40, 255 });

                    // Award XP
                    std::string lvlMsg;
                    if (emp.AddExperience(20, lvlMsg)) {
                        outNotice = lvlMsg;
                        outNoticeColor = { 46, 204, 113, 235 };
                    }
                }

                emp.carriedQuantity = 0;
                emp.carriedProduct = ProductType::NONE;
                emp.targetRackIndex = -1;
                emp.taskState = EmployeeTaskState::STOCKER_RETURNING;
                BuildPath(emp, emp.homePosition);
            }
            break;
        }

        case EmployeeTaskState::STOCKER_RETURNING: {
            if (emp.currentWaypointIndex < emp.waypoints.size()) {
                Vector3 targetWp = emp.waypoints[emp.currentWaypointIndex];
                MoveEmployeeTowards(emp, targetWp, stockerSpeed, deltaTime);
                if (Vector3Distance(emp.position, targetWp) < 0.35f) {
                    emp.currentWaypointIndex++;
                }
            } else {
                emp.taskState = EmployeeTaskState::IDLE;
                emp.rotationY = emp.homeRotationY;
            }
            break;
        }

        default:
            emp.taskState = EmployeeTaskState::IDLE;
            break;
    }
}

// -------------------------------------------------------------
// STAGE 17: CLEANER AUTOMATION
// -------------------------------------------------------------
void EmployeeManager::UpdateCleanerEmployee(Employee& emp, float deltaTime, Shop& shop, bool isShopOpen, std::string& outNotice, Color& outNoticeColor) {
    if (!isShopOpen) {
        if (Vector3Distance(emp.position, emp.homePosition) > 0.3f) {
            MoveEmployeeTowards(emp, emp.homePosition, 2.0f, deltaTime);
        } else {
            emp.position = emp.homePosition;
            emp.rotationY = emp.homeRotationY;
            emp.taskState = EmployeeTaskState::IDLE;
            if (emp.cleanSpotIndex >= 0 && emp.cleanSpotIndex < (int)dirtySpots.size()) {
                dirtySpots[emp.cleanSpotIndex].isReserved = false;
                emp.cleanSpotIndex = -1;
            }
        }
        return;
    }

    float cleanerSpeed = 2.2f + ((float)emp.skill / 100.0f) * 0.6f; // 2.2 to 2.8 m/s

    switch (emp.taskState) {
        case EmployeeTaskState::IDLE: {
            // Find most dirty unreserved spot
            int bestSpot = -1;
            float maxDirt = 0.15f;
            for (size_t i = 0; i < dirtySpots.size(); ++i) {
                if (!dirtySpots[i].isReserved && dirtySpots[i].dirtLevel > maxDirt) {
                    maxDirt = dirtySpots[i].dirtLevel;
                    bestSpot = (int)i;
                }
            }

            if (bestSpot >= 0) {
                emp.cleanSpotIndex = bestSpot;
                dirtySpots[bestSpot].isReserved = true;
                emp.cleanTargetSpot = dirtySpots[bestSpot].position;
                emp.taskState = EmployeeTaskState::CLEANER_HEADING_TO_SPOT;
                BuildPath(emp, emp.cleanTargetSpot);
                emp.Say("Ada lantai kotor, saya bersihkan sekarang.", 2.2f);
            } else {
                // No dirty spots -> Stand idle near lobby
                if (Vector3Distance(emp.position, emp.homePosition) > 0.3f) {
                    MoveEmployeeTowards(emp, emp.homePosition, 2.0f, deltaTime);
                } else {
                    emp.rotationY = emp.homeRotationY;
                }
            }
            break;
        }

        case EmployeeTaskState::CLEANER_HEADING_TO_SPOT: {
            if (emp.currentWaypointIndex < emp.waypoints.size()) {
                Vector3 targetWp = emp.waypoints[emp.currentWaypointIndex];
                MoveEmployeeTowards(emp, targetWp, cleanerSpeed, deltaTime);
                if (Vector3Distance(emp.position, targetWp) < 0.35f) {
                    emp.currentWaypointIndex++;
                }
            } else {
                emp.taskState = EmployeeTaskState::CLEANER_CLEANING;
                emp.taskTimer = 0.0f;
            }
            break;
        }

        case EmployeeTaskState::CLEANER_CLEANING: {
            emp.taskTimer += deltaTime;
            float cleanDuration = 2.0f - ((float)emp.skill / 200.0f); // 1.5s to 2.0s

            if (emp.taskTimer >= cleanDuration) {
                if (emp.cleanSpotIndex >= 0 && emp.cleanSpotIndex < (int)dirtySpots.size()) {
                    dirtySpots[emp.cleanSpotIndex].dirtLevel = 0.0f;
                    dirtySpots[emp.cleanSpotIndex].isReserved = false;
                }

                // Increase store cleanliness meter
                storeCleanliness = std::min(100.0f, storeCleanliness + 6.0f);
                emp.Say("Lantai sudah bersih dan rapi!", 2.2f, Color{ 235, 255, 245, 245 }, Color{ 20, 120, 50, 255 });

                // Award XP
                std::string lvlMsg;
                if (emp.AddExperience(15, lvlMsg)) {
                    outNotice = lvlMsg;
                    outNoticeColor = { 46, 204, 113, 235 };
                }

                emp.cleanSpotIndex = -1;
                emp.taskState = EmployeeTaskState::CLEANER_RETURNING;
                BuildPath(emp, emp.homePosition);
            }
            break;
        }

        case EmployeeTaskState::CLEANER_RETURNING: {
            if (emp.currentWaypointIndex < emp.waypoints.size()) {
                Vector3 targetWp = emp.waypoints[emp.currentWaypointIndex];
                MoveEmployeeTowards(emp, targetWp, cleanerSpeed, deltaTime);
                if (Vector3Distance(emp.position, targetWp) < 0.35f) {
                    emp.currentWaypointIndex++;
                }
            } else {
                emp.taskState = EmployeeTaskState::IDLE;
                emp.rotationY = emp.homeRotationY;
            }
            break;
        }

        default:
            emp.taskState = EmployeeTaskState::IDLE;
            break;
    }
}

void EmployeeManager::Update(float deltaTime, Shop& shop, std::vector<Customer>& customers, bool isShopOpen, std::string& outNotification, Color& outNoticeColor) {
    AssignWorkplacePositions(shop);

    // Natural cleanliness decay over time when shop is open (foot traffic)
    if (isShopOpen) {
        dirtSpawnTimer -= deltaTime;
        if (dirtSpawnTimer <= 0.0f) {
            dirtSpawnTimer = 12.0f;
            // Slightly degrade cleanliness
            storeCleanliness = std::max(20.0f, storeCleanliness - 1.5f);

            // Randomly increase dirt on one spot
            int spotIdx = GetRandomValue(0, (int)dirtySpots.size() - 1);
            if (!dirtySpots[spotIdx].isReserved) {
                dirtySpots[spotIdx].dirtLevel = std::min(1.0f, dirtySpots[spotIdx].dirtLevel + 0.4f);
            }
        }
    }

    std::set<int> reservedRacks;
    for (auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        emp.idleTimer += deltaTime;
        if (emp.speechTimer > 0.0f) {
            emp.speechTimer -= deltaTime;
        }

        // Stuck detection safeguard
        if (Vector3Distance(emp.position, emp.lastStuckCheckPos) < 0.05f && emp.taskState != EmployeeTaskState::IDLE) {
            emp.stuckTimer += deltaTime;
            if (emp.stuckTimer > 6.0f) {
                // Reset stuck task
                emp.taskState = EmployeeTaskState::IDLE;
                emp.position = emp.homePosition;
                emp.stuckTimer = 0.0f;
                emp.targetRackIndex = -1;
                emp.carriedQuantity = 0;
            }
        } else {
            emp.stuckTimer = 0.0f;
            emp.lastStuckCheckPos = emp.position;
        }

        // Execute role specific AI state machine
        if (emp.role == EmployeeRole::CASHIER) {
            UpdateCashierEmployee(emp, deltaTime, shop, customers, isShopOpen, outNotification, outNoticeColor);
        } else if (emp.role == EmployeeRole::STOCKER) {
            UpdateStockerEmployee(emp, deltaTime, shop, isShopOpen, reservedRacks, outNotification, outNoticeColor);
        } else if (emp.role == EmployeeRole::CLEANER) {
            UpdateCleanerEmployee(emp, deltaTime, shop, isShopOpen, outNotification, outNoticeColor);
        }
    }
}

void EmployeeManager::Render3D() {
    // 1. Render floor dirty spots (Stains / Dust markers)
    for (const auto& spot : dirtySpots) {
        if (spot.dirtLevel > 0.1f) {
            Color stainColor = { 100, 80, 60, (unsigned char)(spot.dirtLevel * 180.0f) };
            DrawCircle3D(spot.position, 0.45f * spot.dirtLevel + 0.2f, { 1.0f, 0.0f, 0.0f }, 90.0f, stainColor);
        }
    }

    // 2. Render 3D Employee NPC Models
    for (const auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        float bobOffset = sinf(emp.idleTimer * 3.0f) * 0.02f;
        Vector3 basePos = { emp.position.x, emp.position.y + bobOffset, emp.position.z };

        // 1. Legs (Walk swing animation if moving)
        float legHeight = 0.65f * emp.heightScale;
        float legSwing = (emp.taskState != EmployeeTaskState::IDLE) ? sinf(emp.idleTimer * 8.0f) * 0.08f : 0.0f;

        Vector3 leftLegPos = { basePos.x - 0.14f, basePos.y + legHeight / 2.0f, basePos.z + legSwing };
        Vector3 rightLegPos = { basePos.x + 0.14f, basePos.y + legHeight / 2.0f, basePos.z - legSwing };
        DrawCube(leftLegPos, 0.18f, legHeight, 0.22f, emp.pantsColor);
        DrawCubeWires(leftLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });
        DrawCube(rightLegPos, 0.18f, legHeight, 0.22f, emp.pantsColor);
        DrawCubeWires(rightLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });

        // 2. Body / Uniform
        float bodyHeight = 0.75f * emp.heightScale;
        Vector3 torsoPos = { basePos.x, basePos.y + legHeight + bodyHeight / 2.0f, basePos.z };
        DrawCube(torsoPos, 0.55f, bodyHeight, 0.35f, emp.shirtColor);
        DrawCubeWires(torsoPos, 0.55f, bodyHeight, 0.35f, { 30, 30, 35, 255 });

        // 3. Head
        Vector3 headPos = { basePos.x, basePos.y + legHeight + bodyHeight + 0.22f, basePos.z };
        DrawSphere(headPos, 0.22f, emp.skinColor);
        DrawSphereWires(headPos, 0.22f, 8, 8, { 180, 140, 120, 255 });

        // 4. Uniform Cap
        Vector3 capPos = { basePos.x, headPos.y + 0.14f, basePos.z };
        DrawCube(capPos, 0.38f, 0.12f, 0.38f, emp.accessoryColor);

        // 5. Carried Item or Cleaning Tool Visual Placeholder (Stage 17)
        if (emp.role == EmployeeRole::STOCKER && emp.carriedProduct != ProductType::NONE && emp.carriedQuantity > 0) {
            Vector3 itemPos = { basePos.x + 0.25f, basePos.y + 0.9f, basePos.z + 0.25f };
            ProductInfo pInfo = GetProductInfo(emp.carriedProduct);
            DrawCube(itemPos, 0.28f, 0.28f, 0.28f, pInfo.primaryColor);
            DrawCubeWires(itemPos, 0.28f, 0.28f, 0.28f, RAYWHITE);
        } else if (emp.role == EmployeeRole::CLEANER) {
            // Cleaner holds mop handle
            Vector3 mopPos = { basePos.x + 0.3f, basePos.y + 0.75f, basePos.z + 0.2f };
            DrawCylinder(mopPos, 0.03f, 0.03f, 1.1f, 6, { 180, 140, 100, 255 });
            Vector3 mopHead = { mopPos.x, mopPos.y - 0.5f, mopPos.z };
            DrawCube(mopHead, 0.22f, 0.12f, 0.15f, { 220, 220, 230, 255 });
        }

        // 6. Overhead Role Marker & Level Badge
        Vector3 markerPos = { basePos.x, headPos.y + 0.55f, basePos.z };
        Color markerCol = (emp.role == EmployeeRole::CASHIER) ? Color{ 41, 128, 185, 255 } :
                          (emp.role == EmployeeRole::STOCKER) ? Color{ 230, 126, 34, 255 } : Color{ 39, 174, 96, 255 };
        DrawCube(markerPos, 0.14f, 0.14f, 0.14f, markerCol);
        DrawCubeWires(markerPos, 0.14f, 0.14f, 0.14f, RAYWHITE);
    }
}

void EmployeeManager::RenderSpeechBubbles2D(Camera3D camera, int screenWidth, int screenHeight) {
    for (const auto& emp : activeEmployees) {
        if (!emp.isHired || !emp.HasActiveDialogue()) continue;

        Vector3 headPos = { emp.position.x, emp.position.y + 2.1f, emp.position.z };

        // Do not render if behind camera
        Vector3 toNpc = Vector3Subtract(headPos, camera.position);
        Vector3 forward = Vector3Subtract(camera.target, camera.position);
        if (Vector3DotProduct(toNpc, forward) <= 0.1f) continue;

        Vector2 screenPos = GetWorldToScreen(headPos, camera);
        if (screenPos.x < -100 || screenPos.x > screenWidth + 100 ||
            screenPos.y < -100 || screenPos.y > screenHeight + 100) continue;

        int fontSize = 12;
        int textWidth = MeasureText(emp.speechText.c_str(), fontSize);
        int paddingX = 10;
        int paddingY = 6;
        int boxW = textWidth + paddingX * 2;
        int boxH = fontSize + paddingY * 2;

        int boxX = (int)screenPos.x - boxW / 2;
        int boxY = (int)screenPos.y - boxH - 6;

        // Speech bubble box
        DrawRectangle(boxX, boxY, boxW, boxH, emp.speechBubbleColor);
        DrawRectangleLines(boxX, boxY, boxW, boxH, emp.shirtColor);

        // Little bottom pointer triangle
        DrawTriangle(
            { (float)screenPos.x - 5.0f, (float)boxY + boxH },
            { (float)screenPos.x, (float)screenPos.y - 2.0f },
            { (float)screenPos.x + 5.0f, (float)boxY + boxH },
            emp.speechBubbleColor
        );

        DrawText(emp.speechText.c_str(), boxX + paddingX, boxY + paddingY, fontSize, emp.speechTextColor);
    }
}

const Employee* EmployeeManager::GetNearbyEmployee(Vector3 playerEyePos, Vector3 playerLookDir, float maxDist) const {
    for (const auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        Vector3 empHeadPos = { emp.position.x, emp.position.y + 1.6f, emp.position.z };
        float dist = Vector3Distance(playerEyePos, empHeadPos);
        if (dist <= maxDist) {
            Vector3 toEmp = Vector3Normalize(Vector3Subtract(empHeadPos, playerEyePos));
            float dot = Vector3DotProduct(playerLookDir, toEmp);
            if (dot > 0.8f) {
                return &emp;
            }
        }
    }
    return nullptr;
}

void EmployeeManager::RenderUI(int screenWidth, int screenHeight, int curBalance, int shopUpgradeLevel) {
    if (!menuOpen) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

    int modalW = 760;
    int modalH = 530;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    int maxCap = GetMaxEmployeeCapacity(shopUpgradeLevel);

    DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 42, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 41, 128, 185, 255 });

    DrawText("MANAJEMEN KARYAWAN & AUTOMATION (STAGE 17)", modalX + 30, modalY + 18, 20, { 100, 220, 255, 255 });
    DrawText("Sistem otomatisasi staf: Kasir, Pengisi Rak (Stocker), & Kebersihan (Cleaner)", modalX + 30, modalY + 42, 12, { 180, 195, 210, 255 });

    // Tabs: [1. Karyawan Aktif] vs [2. Rekrut Karyawan]
    int tabY = modalY + 68;
    bool isMyTab = (currentTab == EmployeeMenuTab::MY_EMPLOYEES);

    Color tab1Bg = isMyTab ? Color{ 41, 128, 185, 255 } : Color{ 35, 42, 55, 255 };
    Color tab2Bg = !isMyTab ? Color{ 39, 174, 96, 255 } : Color{ 35, 42, 55, 255 };

    DrawRectangle(modalX + 30, tabY, 210, 32, tab1Bg);
    DrawRectangleLines(modalX + 30, tabY, 210, 32, RAYWHITE);
    std::string tab1Title = "1. Karyawan Aktif (" + std::to_string(activeEmployees.size()) + "/" + std::to_string(maxCap) + ")";
    DrawText(tab1Title.c_str(), modalX + 42, tabY + 8, 13, RAYWHITE);

    DrawRectangle(modalX + 250, tabY, 210, 32, tab2Bg);
    DrawRectangleLines(modalX + 250, tabY, 210, 32, RAYWHITE);
    std::string tab2Title = "2. Rekrut / Hire (" + std::to_string(candidatePool.size()) + " Tersedia)";
    DrawText(tab2Title.c_str(), modalX + 262, tabY + 8, 13, RAYWHITE);

    int listY = modalY + 110;

    // TAB 1: ACTIVE EMPLOYEES
    if (isMyTab) {
        if (activeEmployees.empty()) {
            DrawRectangle(modalX + 30, listY, modalW - 60, 180, { 20, 24, 32, 220 });
            DrawRectangleLines(modalX + 30, listY, modalW - 60, 180, { 55, 65, 75, 255 });
            DrawText("Belum ada karyawan yang direkrut.", modalX + 50, listY + 60, 16, { 255, 200, 100, 255 });
            DrawText("Tekan [TAB / Q / E] untuk beralih ke tab 'Rekrut / Hire' dan pekerjakan staf pertamamu!", modalX + 50, listY + 90, 13, { 180, 200, 220, 255 });
        } else {
            int visibleCount = 3;
            int startIdx = std::max(0, std::min((int)activeEmployees.size() - visibleCount, selectedIndex - 1));

            for (int i = startIdx; i < startIdx + visibleCount && i < (int)activeEmployees.size(); ++i) {
                const auto& emp = activeEmployees[i];
                bool isSel = (selectedIndex == i);

                Color itemBg = isSel ? Color{ 35, 65, 95, 240 } : Color{ 30, 36, 45, 200 };
                Color itemBorder = isSel ? Color{ 0, 220, 255, 255 } : Color{ 55, 65, 75, 255 };

                DrawRectangle(modalX + 30, listY, modalW - 60, 80, itemBg);
                DrawRectangleLines(modalX + 30, listY, modalW - 60, 80, itemBorder);

                // Role Icon Badge
                DrawRectangle(modalX + 45, listY + 14, 52, 52, emp.shirtColor);
                DrawRectangleLines(modalX + 45, listY + 14, 52, 52, RAYWHITE);
                DrawText(emp.GetRoleString().substr(0, 4).c_str(), modalX + 48, listY + 32, 10, RAYWHITE);

                // Name, Role, Level & Experience
                std::string headerLine = "[" + emp.GetIdString() + "] " + emp.name + " (" + emp.GetRoleString() + ") - Lvl " + std::to_string(emp.level) +
                                         " [XP: " + std::to_string(emp.experience) + "/" + std::to_string(emp.level * 100) + "]" + (isSel ? "  <-- DIPILIH" : "");
                DrawText(headerLine.c_str(), modalX + 110, listY + 12, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                // Stats: Skill, Morale, Productivity, Current Task
                std::string statLine = "Skill: " + std::to_string(emp.skill) + "/100 | Morale: " + std::to_string(emp.morale) + "/100 | Gaji: Rp" + std::to_string(emp.salary) + "/hari";
                DrawText(statLine.c_str(), modalX + 110, listY + 32, 12, { 100, 220, 255, 255 });

                std::string taskLine = "Task Aktif: " + emp.GetCurrentTaskString();
                DrawText(taskLine.c_str(), modalX + 110, listY + 54, 12, { 50, 255, 120, 255 });

                // Action hint on selected card
                if (isSel) {
                    if (confirmingFire) {
                        DrawRectangle(modalX + modalW - 225, listY + 15, 180, 50, { 180, 40, 40, 240 });
                        DrawRectangleLines(modalX + modalW - 225, listY + 15, 180, 50, RAYWHITE);
                        DrawText("Yakin Pecat? (Fire)", modalX + modalW - 210, listY + 22, 12, RAYWHITE);
                        DrawText("[Y: Pecat] [N: Batal]", modalX + modalW - 210, listY + 42, 11, { 255, 220, 120, 255 });
                    } else {
                        DrawText("[ENTER / F: Pecat]", modalX + modalW - 190, listY + 30, 12, { 255, 100, 100, 255 });
                    }
                }

                listY += 88;
            }
        }
    }
    // TAB 2: HIRING CANDIDATES
    else {
        if (candidatePool.empty()) {
            DrawRectangle(modalX + 30, listY, modalW - 60, 180, { 20, 24, 32, 220 });
            DrawRectangleLines(modalX + 30, listY, modalW - 60, 180, { 55, 65, 75, 255 });
            DrawText("Semua kandidat saat ini telah direkrut.", modalX + 50, listY + 70, 16, { 50, 255, 120, 255 });
        } else {
            int visibleCount = 3;
            int startIdx = std::max(0, std::min((int)candidatePool.size() - visibleCount, selectedIndex - 1));

            for (int i = startIdx; i < startIdx + visibleCount && i < (int)candidatePool.size(); ++i) {
                const auto& cand = candidatePool[i];
                bool isSel = (selectedIndex == i);

                Color itemBg = isSel ? Color{ 30, 75, 55, 240 } : Color{ 30, 36, 45, 200 };
                Color itemBorder = isSel ? Color{ 46, 204, 113, 255 } : Color{ 55, 65, 75, 255 };

                DrawRectangle(modalX + 30, listY, modalW - 60, 80, itemBg);
                DrawRectangleLines(modalX + 30, listY, modalW - 60, 80, itemBorder);

                // Badge
                DrawRectangle(modalX + 45, listY + 14, 52, 52, cand.shirtColor);
                DrawRectangleLines(modalX + 45, listY + 14, 52, 52, RAYWHITE);
                DrawText(cand.GetRoleString().substr(0, 4).c_str(), modalX + 48, listY + 32, 10, RAYWHITE);

                // Info
                std::string headerLine = cand.name + " (" + cand.GetRoleString() + ") - Lvl " + std::to_string(cand.level) + (isSel ? "  [DIPILIH]" : "");
                DrawText(headerLine.c_str(), modalX + 110, listY + 12, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                std::string statLine = "Skill: " + std::to_string(cand.skill) + "/100 | Morale: " + std::to_string(cand.morale) + "/100 | Gaji: Rp" + std::to_string(cand.salary) + "/hari";
                DrawText(statLine.c_str(), modalX + 110, listY + 32, 12, { 180, 220, 245, 255 });

                std::string hireCostLine = "Biaya Rekrutmen: Rp" + std::to_string(cand.hiringCost);
                DrawText(hireCostLine.c_str(), modalX + 110, listY + 54, 12, { 255, 215, 0, 255 });

                if (isSel) {
                    bool canAfford = (curBalance >= cand.hiringCost);
                    bool hasCapacity = ((int)activeEmployees.size() < maxCap);

                    if (!hasCapacity) {
                        DrawText("[Kapasitas Penuh]", modalX + modalW - 190, listY + 30, 12, { 255, 120, 120, 255 });
                    } else if (!canAfford) {
                        DrawText("[Saldo Kurang]", modalX + modalW - 180, listY + 30, 12, { 255, 100, 100, 255 });
                    } else {
                        DrawText("[ENTER / SPACE: Rekrut]", modalX + modalW - 210, listY + 30, 12, { 50, 255, 120, 255 });
                    }
                }

                listY += 88;
            }
        }
    }

    // Summary Card at bottom
    int sumBoxY = modalY + 380;
    DrawRectangle(modalX + 30, sumBoxY, modalW - 60, 95, { 18, 22, 28, 240 });
    DrawRectangleLines(modalX + 30, sumBoxY, modalW - 60, 95, { 60, 75, 90, 255 });

    std::string capInfo = "Kapasitas Toko: " + std::to_string(activeEmployees.size()) + " / " + std::to_string(maxCap) +
                          " Karyawan  |  Kebersihan Toko: " + std::to_string((int)storeCleanliness) + "%";
    DrawText(capInfo.c_str(), modalX + 45, sumBoxY + 12, 13, { 100, 220, 255, 255 });

    std::string salarySummary = "Total Beban Gaji Harian: Rp" + std::to_string(GetTotalDailySalary()) + " / hari (Dibayar otomatis setiap 21:00)";
    DrawText(salarySummary.c_str(), modalX + 45, sumBoxY + 36, 13, { 255, 215, 0, 255 });

    std::string balInfo = "Saldo Toko Saat Ini: Rp" + std::to_string(curBalance);
    DrawText(balInfo.c_str(), modalX + 45, sumBoxY + 60, 13, { 50, 255, 120, 255 });

    // Navigation Footnotes
    DrawText("[Q / E / TAB] Ganti Tab    [W / S / Panah] Pilih    [ENTER] Eksekusi    [K / ESC] Tutup",
             modalX + 45, modalY + 495, 12, { 255, 220, 120, 255 });
}

