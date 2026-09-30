#pragma once
#include "Employee.hpp"
#include <vector>
#include <string>
#include <set>

class Finance;
class DailyStats;
class Shop;
class ShopUpgrade;
class Customer;
class PriceManager;
class Reputation;

enum class EmployeeMenuTab {
    MY_EMPLOYEES = 0,
    HIRE_CANDIDATES
};

struct DirtySpot {
    Vector3 position;
    float dirtLevel; // 0.0 to 1.0
    bool isReserved;
};

class EmployeeManager {
public:
    static EmployeeManager& Instance();

    void Init();
    void Update(float deltaTime, Shop& shop, std::vector<Customer>& customers, bool isShopOpen, std::string& outNotification, Color& outNoticeColor);
    void Render3D();
    void RenderSpeechBubbles2D(Camera3D camera, int screenWidth, int screenHeight);

    // Queries
    const std::vector<Employee>& GetActiveEmployees() const { return activeEmployees; }
    std::vector<Employee>& GetActiveEmployees() { return activeEmployees; }
    const std::vector<Employee>& GetCandidates() const { return candidatePool; }
    
    int GetActiveEmployeeCount() const { return (int)activeEmployees.size(); }
    int GetMaxEmployeeCapacity(int shopUpgradeLevel) const; // Level 1: 2, Level 2: 4, Level 3: 6
    int GetTotalDailySalary() const;

    // Actions
    bool HireEmployee(int candidateIndex, Finance& finance, DailyStats& dailyStats, int maxCapacity, std::string& outFeedback);
    bool FireEmployee(int employeeIndex, std::string& outFeedback);

    // End-of-day Salary payment
    bool ProcessDailySalaries(Finance& finance, DailyStats& dailyStats, std::string& outSummary);

    // Store Cleanliness System (Stage 17)
    int GetCleanliness() const { return (int)storeCleanliness; }
    void SetCleanliness(float val) { storeCleanliness = std::max(0.0f, std::min(100.0f, val)); }
    const std::vector<DirtySpot>& GetDirtySpots() const { return dirtySpots; }

    // UI Menu Navigation
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    EmployeeMenuTab GetActiveTab() const { return currentTab; }
    void SetActiveTab(EmployeeMenuTab tab) { currentTab = tab; selectedIndex = 0; }
    void SwitchTab();

    int GetSelectedIndex() const { return selectedIndex; }
    void NextItem();
    void PreviousItem();

    bool IsConfirmingFire() const { return confirmingFire; }
    void SetConfirmingFire(bool confirming) { confirmingFire = confirming; }

    // Save / Load helpers
    void ClearAllEmployees();
    void AddEmployeeDirect(const Employee& emp);
    void SetCandidatesDirect(const std::vector<Employee>& candidates);
    void RefreshCandidates();

    // Render Menu UI Modal
    void RenderUI(int screenWidth, int screenHeight, int curBalance, int shopUpgradeLevel);

    // Proximity inspection test
    const Employee* GetNearbyEmployee(Vector3 playerEyePos, Vector3 playerLookDir, float maxDist = 3.5f) const;

private:
    EmployeeManager();
    ~EmployeeManager() = default;

    std::vector<Employee> activeEmployees;
    std::vector<Employee> candidatePool;

    // Cleanliness & dirty spots in store
    float storeCleanliness;
    std::vector<DirtySpot> dirtySpots;
    float dirtSpawnTimer;

    // Task check timers (Performance optimization: scan intervals)
    float taskDecisionInterval;

    bool menuOpen;
    EmployeeMenuTab currentTab;
    int selectedIndex;
    bool confirmingFire;

    int nextEmployeeId;

    void AssignWorkplacePositions(Shop& shop);
    void GenerateDefaultCandidates();
    void InitCleanlinessSpots();

    // Stage 17 Automation Subsystems
    void UpdateCashierEmployee(Employee& emp, float deltaTime, Shop& shop, std::vector<Customer>& customers, bool isShopOpen, std::string& outNotice, Color& outNoticeColor);
    void UpdateStockerEmployee(Employee& emp, float deltaTime, Shop& shop, bool isShopOpen, std::set<int>& reservedRacks, std::string& outNotice, Color& outNoticeColor);
    void UpdateCleanerEmployee(Employee& emp, float deltaTime, Shop& shop, bool isShopOpen, std::string& outNotice, Color& outNoticeColor);

    // Navigation & Collision avoidance helper
    void MoveEmployeeTowards(Employee& emp, Vector3 targetPos, float speed, float deltaTime);
    void BuildPath(Employee& emp, Vector3 targetPos);
};

