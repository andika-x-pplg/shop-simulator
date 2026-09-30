#pragma once
#include <string>
#include <vector>
#include "raylib.h"

struct DailyRatingEntry {
    std::string customerName;
    int stars;
    int satisfaction;
    std::string feedback;
};

class DailyStats {
public:
    DailyStats();

    void Init();

    // Event hooks (called when transactions/ratings happen)
    void RecordRevenue(int amount);
    void RecordExpense(int amount);
    void IncrementCustomerServed();
    void RecordRating(const std::string& name, int stars, int satisfaction, const std::string& feedback);

    // Reset daily counters when starting next day
    void ResetDaily();

    // Getters
    int GetDailyRevenue() const { return dailyRevenue; }
    int GetDailyExpenses() const { return dailyExpenses; }
    int GetDailyProfit() const { return dailyRevenue - dailyExpenses; }
    int GetDailyCustomers() const { return dailyCustomers; }
    int GetDailyRatingsCount() const { return (int)dailyRatings.size(); }
    float GetDailyAverageRating() const;
    const std::vector<DailyRatingEntry>& GetDailyRatings() const { return dailyRatings; }

    // Render Daily Summary Modal
    void RenderSummaryModal(int screenWidth, int screenHeight, int dayNumber, int currentBalance) const;

private:
    int dailyRevenue;
    int dailyExpenses;
    int dailyCustomers;
    std::vector<DailyRatingEntry> dailyRatings;
    int dailyRatingPoints;
};
