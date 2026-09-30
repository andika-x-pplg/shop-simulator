#include "EmployeeManager.hpp"
#include "Finance.hpp"
#include "DailyStats.hpp"
#include "Shop.hpp"
#include "ShopUpgrade.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>

EmployeeManager& EmployeeManager::Instance() {
    static EmployeeManager instance;
    return instance;
}

EmployeeManager::EmployeeManager()
    : menuOpen(false),
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

    GenerateDefaultCandidates();
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
    e1.position = { -2.0f, 0.0f, 6.0f };
    e1.rotationY = 0.0f;
    e1.skinColor = { 245, 210, 180, 255 };
    e1.shirtColor = { 41, 128, 185, 255 }; // Cashier blue uniform
    e1.pantsColor = { 44, 62, 80, 255 };
    e1.accessoryColor = { 241, 196, 15, 255 }; // Gold cashier badge
    e1.heightScale = 1.0f;
    e1.idleTimer = 0.0f;
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
    e2.position = { 0.0f, 0.0f, 6.0f };
    e2.rotationY = 0.0f;
    e2.skinColor = { 235, 195, 160, 255 };
    e2.shirtColor = { 230, 126, 34, 255 }; // Stocker orange uniform
    e2.pantsColor = { 50, 50, 50, 255 };
    e2.accessoryColor = { 192, 57, 43, 255 };
    e2.heightScale = 1.04f;
    e2.idleTimer = 1.2f;
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
    e3.position = { 2.0f, 0.0f, 6.0f };
    e3.rotationY = 0.0f;
    e3.skinColor = { 255, 220, 190, 255 };
    e3.shirtColor = { 39, 174, 96, 255 }; // Cleaner green apron
    e3.pantsColor = { 45, 52, 54, 255 };
    e3.accessoryColor = { 46, 204, 113, 255 };
    e3.heightScale = 0.96f;
    e3.idleTimer = 2.4f;
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
    e4.position = { -2.0f, 0.0f, 6.0f };
    e4.rotationY = 0.0f;
    e4.skinColor = { 220, 175, 140, 255 };
    e4.shirtColor = { 52, 73, 94, 255 };
    e4.pantsColor = { 30, 39, 46, 255 };
    e4.accessoryColor = { 243, 156, 18, 255 };
    e4.heightScale = 1.02f;
    e4.idleTimer = 0.7f;
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
    e5.position = { 0.0f, 0.0f, 6.0f };
    e5.rotationY = 0.0f;
    e5.skinColor = { 240, 205, 175, 255 };
    e5.shirtColor = { 211, 84, 0, 255 };
    e5.pantsColor = { 45, 52, 54, 255 };
    e5.accessoryColor = { 241, 196, 15, 255 };
    e5.heightScale = 0.98f;
    e5.idleTimer = 1.8f;
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
    // Workplace placeholder positions inside the store
    // Cashier assistant standing near counter
    // Stocker standing near Storage pallets
    // Cleaner standing near front lobby
    int cashierCount = 0;
    int stockerCount = 0;
    int cleanerCount = 0;

    for (auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        if (emp.role == EmployeeRole::CASHIER) {
            emp.position = { -1.8f - (float)cashierCount * 0.8f, 0.0f, 1.2f };
            emp.rotationY = 0.0f; // Face entrance/lobby
            cashierCount++;
        } else if (emp.role == EmployeeRole::STOCKER) {
            emp.position = { 5.2f, 0.0f, -8.0f - (float)stockerCount * 1.2f };
            emp.rotationY = -PI / 2.0f; // Face storage shelves
            stockerCount++;
        } else if (emp.role == EmployeeRole::CLEANER) {
            emp.position = { 2.5f + (float)cleanerCount * 1.0f, 0.0f, 7.5f };
            emp.rotationY = PI; // Face lobby
            cleanerCount++;
        }
    }
}

void EmployeeManager::Update(float deltaTime, Shop& shop) {
    AssignWorkplacePositions(shop);

    for (auto& emp : activeEmployees) {
        emp.idleTimer += deltaTime;
    }
}

void EmployeeManager::Render3D() {
    for (const auto& emp : activeEmployees) {
        if (!emp.isHired || emp.status != EmployeeStatus::WORKING) continue;

        float bobOffset = sinf(emp.idleTimer * 2.0f) * 0.02f;
        Vector3 basePos = { emp.position.x, emp.position.y + bobOffset, emp.position.z };

        // 1. Legs
        float legHeight = 0.65f * emp.heightScale;
        Vector3 leftLegPos = { basePos.x - 0.14f, basePos.y + legHeight / 2.0f, basePos.z };
        Vector3 rightLegPos = { basePos.x + 0.14f, basePos.y + legHeight / 2.0f, basePos.z };
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

        // 4. Employee Uniform Hat / Cap
        Vector3 capPos = { basePos.x, headPos.y + 0.14f, basePos.z };
        DrawCube(capPos, 0.38f, 0.12f, 0.38f, emp.accessoryColor);

        // 5. Overhead Role Marker (Distinguishes from customers: Cyan Diamond)
        Vector3 markerPos = { basePos.x, headPos.y + 0.55f, basePos.z };
        Color markerCol = (emp.role == EmployeeRole::CASHIER) ? Color{ 41, 128, 185, 255 } :
                          (emp.role == EmployeeRole::STOCKER) ? Color{ 230, 126, 34, 255 } : Color{ 39, 174, 96, 255 };
        DrawCube(markerPos, 0.14f, 0.14f, 0.14f, markerCol);
        DrawCubeWires(markerPos, 0.14f, 0.14f, 0.14f, RAYWHITE);
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
    int modalH = 520;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    int maxCap = GetMaxEmployeeCapacity(shopUpgradeLevel);

    DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 42, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 41, 128, 185, 255 });

    DrawText("MANAJEMEN KARYAWAN TOKO (EMPLOYEE SYSTEM)", modalX + 30, modalY + 18, 20, { 100, 220, 255, 255 });
    DrawText("Kelola staf kasir, stocker, dan cleaner untuk operasional toko", modalX + 30, modalY + 42, 12, { 180, 195, 210, 255 });

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

                DrawRectangle(modalX + 30, listY, modalW - 60, 78, itemBg);
                DrawRectangleLines(modalX + 30, listY, modalW - 60, 78, itemBorder);

                // Role Icon Badge
                DrawRectangle(modalX + 45, listY + 14, 50, 50, emp.shirtColor);
                DrawRectangleLines(modalX + 45, listY + 14, 50, 50, RAYWHITE);
                DrawText(emp.GetRoleString().substr(0, 4).c_str(), modalX + 48, listY + 32, 10, RAYWHITE);

                // Name, Role & Level
                std::string headerLine = "[" + emp.GetIdString() + "] " + emp.name + " (" + emp.GetRoleString() + ") - Lvl " + std::to_string(emp.level) + (isSel ? "  [DIPILIH]" : "");
                DrawText(headerLine.c_str(), modalX + 110, listY + 12, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                // Stats: Skill, Morale, Productivity, Salary
                std::string statLine = "Skill: " + std::to_string(emp.skill) + "/100 | Morale: " + std::to_string(emp.morale) + "/100 | Prod: " + std::to_string(emp.productivity) + "%";
                DrawText(statLine.c_str(), modalX + 110, listY + 32, 12, { 100, 220, 255, 255 });

                std::string salaryLine = "Gaji: Rp" + std::to_string(emp.salary) + "/hari  |  Status: " + emp.GetStatusString();
                DrawText(salaryLine.c_str(), modalX + 110, listY + 52, 12, { 50, 255, 120, 255 });

                // Action hint on selected card
                if (isSel) {
                    if (confirmingFire) {
                        DrawRectangle(modalX + modalW - 225, listY + 15, 180, 48, { 180, 40, 40, 240 });
                        DrawRectangleLines(modalX + modalW - 225, listY + 15, 180, 48, RAYWHITE);
                        DrawText("Yakin Pecat? (Fire)", modalX + modalW - 210, listY + 22, 12, RAYWHITE);
                        DrawText("[Y: Pecat] [N: Batal]", modalX + modalW - 210, listY + 42, 11, { 255, 220, 120, 255 });
                    } else {
                        DrawText("[ENTER / F: Pecat Karyawan]", modalX + modalW - 220, listY + 30, 12, { 255, 100, 100, 255 });
                    }
                }

                listY += 86;
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

                DrawRectangle(modalX + 30, listY, modalW - 60, 78, itemBg);
                DrawRectangleLines(modalX + 30, listY, modalW - 60, 78, itemBorder);

                // Badge
                DrawRectangle(modalX + 45, listY + 14, 50, 50, cand.shirtColor);
                DrawRectangleLines(modalX + 45, listY + 14, 50, 50, RAYWHITE);
                DrawText(cand.GetRoleString().substr(0, 4).c_str(), modalX + 48, listY + 32, 10, RAYWHITE);

                // Info
                std::string headerLine = cand.name + " (" + cand.GetRoleString() + ") - Lvl " + std::to_string(cand.level) + (isSel ? "  [DIPILIH]" : "");
                DrawText(headerLine.c_str(), modalX + 110, listY + 12, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                std::string statLine = "Skill: " + std::to_string(cand.skill) + "/100 | Morale: " + std::to_string(cand.morale) + "/100 | Gaji: Rp" + std::to_string(cand.salary) + "/hari";
                DrawText(statLine.c_str(), modalX + 110, listY + 32, 12, { 180, 220, 245, 255 });

                std::string hireCostLine = "Biaya Rekrutmen: Rp" + std::to_string(cand.hiringCost);
                DrawText(hireCostLine.c_str(), modalX + 110, listY + 52, 12, { 255, 215, 0, 255 });

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

                listY += 86;
            }
        }
    }

    // Summary Card at bottom
    int sumBoxY = modalY + 380;
    DrawRectangle(modalX + 30, sumBoxY, modalW - 60, 85, { 18, 22, 28, 240 });
    DrawRectangleLines(modalX + 30, sumBoxY, modalW - 60, 85, { 60, 75, 90, 255 });

    std::string capInfo = "Kapasitas Toko: " + std::to_string(activeEmployees.size()) + " / " + std::to_string(maxCap) + " Karyawan (Upgrade Toko untuk ekspansi)";
    DrawText(capInfo.c_str(), modalX + 45, sumBoxY + 12, 13, { 100, 220, 255, 255 });

    std::string salarySummary = "Total Beban Gaji Harian: Rp" + std::to_string(GetTotalDailySalary()) + " / hari (Dibayar otomatis setiap akhir hari)";
    DrawText(salarySummary.c_str(), modalX + 45, sumBoxY + 34, 13, { 255, 215, 0, 255 });

    std::string balInfo = "Saldo Toko Saat Ini: Rp" + std::to_string(curBalance);
    DrawText(balInfo.c_str(), modalX + 45, sumBoxY + 56, 13, { 50, 255, 120, 255 });

    // Navigation Footnotes
    DrawText("[Q / E / TAB] Ganti Tab    [W / S / Panah] Pilih    [ENTER] Eksekusi    [K / ESC] Tutup",
             modalX + 45, modalY + 485, 12, { 255, 220, 120, 255 });
}
