#include "DailyStats.hpp"
#include <string>

DailyStats::DailyStats()
    : dailyRevenue(0), dailyExpenses(0), dailyCustomers(0), dailyRatingPoints(0) {}

void DailyStats::Init() {
    ResetDaily();
}

void DailyStats::RecordRevenue(int amount) {
    dailyRevenue += amount;
}

void DailyStats::RecordExpense(int amount) {
    dailyExpenses += amount;
}

void DailyStats::IncrementCustomerServed() {
    dailyCustomers++;
}

void DailyStats::RecordRating(const std::string& name, int stars, int satisfaction, const std::string& feedback) {
    dailyRatings.push_back({ name, stars, satisfaction, feedback });
    dailyRatingPoints += stars;
}

void DailyStats::ResetDaily() {
    dailyRevenue = 0;
    dailyExpenses = 0;
    dailyCustomers = 0;
    dailyRatings.clear();
    dailyRatingPoints = 0;
}

float DailyStats::GetDailyAverageRating() const {
    if (dailyRatings.empty()) return 0.0f;
    return (float)dailyRatingPoints / (float)dailyRatings.size();
}

void DailyStats::RenderSummaryModal(int screenWidth, int screenHeight, int dayNumber, int currentBalance) const {
    // Dim background
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 190 });

    int modalW = 600;
    int modalH = 500;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    DrawRectangle(modalX, modalY, modalW, modalH, { 22, 28, 38, 250 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, { 241, 196, 15, 255 });

    // Header
    DrawText("================================", modalX + 40, modalY + 20, 16, { 100, 150, 180, 255 });
    DrawText("DAILY SUMMARY (RINGKASAN HARIAN)", modalX + 40, modalY + 40, 20, { 255, 215, 0, 255 });
    std::string dayStr = "Laporan Operasional Hari ke-" + std::to_string(dayNumber) + " (Toko Ditutup - 21:00)";
    DrawText(dayStr.c_str(), modalX + 40, modalY + 68, 14, { 180, 200, 220, 255 });
    DrawText("================================", modalX + 40, modalY + 86, 16, { 100, 150, 180, 255 });

    // Cards
    int cardW = 245;
    int cardH = 65;

    // Daily Revenue Card
    int c1X = modalX + 40;
    int c1Y = modalY + 115;
    DrawRectangle(c1X, c1Y, cardW, cardH, { 30, 38, 50, 240 });
    DrawRectangleLines(c1X, c1Y, cardW, cardH, { 52, 152, 219, 255 });
    DrawText("Daily Revenue (Pendapatan):", c1X + 12, c1Y + 10, 12, { 180, 200, 220, 255 });
    std::string revText = "Rp" + std::to_string(dailyRevenue);
    DrawText(revText.c_str(), c1X + 12, c1Y + 30, 18, { 50, 255, 120, 255 });

    // Daily Expenses Card
    int c2X = modalX + 315;
    int c2Y = modalY + 115;
    DrawRectangle(c2X, c2Y, cardW, cardH, { 30, 38, 50, 240 });
    DrawRectangleLines(c2X, c2Y, cardW, cardH, { 231, 76, 60, 255 });
    DrawText("Daily Expenses (Pengeluaran):", c2X + 12, c2Y + 10, 12, { 180, 200, 220, 255 });
    std::string expText = "Rp" + std::to_string(dailyExpenses);
    DrawText(expText.c_str(), c2X + 12, c2Y + 30, 18, { 255, 100, 100, 255 });

    // Daily Profit Card
    int c3X = modalX + 40;
    int c3Y = modalY + 190;
    int profit = GetDailyProfit();
    DrawRectangle(c3X, c3Y, cardW, cardH, { 30, 38, 50, 240 });
    Color pCol = (profit >= 0) ? Color{ 46, 204, 113, 255 } : Color{ 231, 76, 60, 255 };
    DrawRectangleLines(c3X, c3Y, cardW, cardH, pCol);
    DrawText("Daily Profit (Keuntungan Bersih):", c3X + 12, c3Y + 10, 12, { 180, 200, 220, 255 });
    std::string pStr = (profit >= 0) ? ("+Rp" + std::to_string(profit)) : ("-Rp" + std::to_string(-profit));
    DrawText(pStr.c_str(), c3X + 12, c3Y + 30, 18, pCol);

    // Customer & Rating Card
    int c4X = modalX + 315;
    int c4Y = modalY + 190;
    DrawRectangle(c4X, c4Y, cardW, cardH, { 30, 38, 50, 240 });
    DrawRectangleLines(c4X, c4Y, cardW, cardH, { 241, 196, 15, 255 });
    DrawText("Customer & Rata-rata Rating:", c4X + 12, c4Y + 10, 12, { 180, 200, 220, 255 });
    std::string custRatingText = std::to_string(dailyCustomers) + " Orang | " + 
                                (dailyRatings.empty() ? "-" : TextFormat("%.1f / 5.0", GetDailyAverageRating()));
    DrawText(custRatingText.c_str(), c4X + 12, c4Y + 30, 16, { 255, 215, 0, 255 });

    // Current Bank Balance
    int balBoxY = modalY + 268;
    DrawRectangle(modalX + 40, balBoxY, modalW - 80, 42, { 18, 22, 28, 240 });
    DrawRectangleLines(modalX + 40, balBoxY, modalW - 80, 42, { 80, 90, 100, 255 });
    std::string balText = "Saldo Akhir Toko: Rp" + std::to_string(currentBalance);
    DrawText(balText.c_str(), modalX + 55, balBoxY + 12, 16, { 50, 255, 120, 255 });

    // Recent Customer Feedbacks Today
    int fBoxY = modalY + 320;
    DrawText("Ulasan Customer Hari Ini:", modalX + 40, fBoxY, 13, { 241, 196, 15, 255 });
    DrawRectangle(modalX + 40, fBoxY + 18, modalW - 80, 85, { 18, 22, 28, 240 });
    DrawRectangleLines(modalX + 40, fBoxY + 18, modalW - 80, 85, { 60, 70, 80, 255 });

    if (dailyRatings.empty()) {
        DrawText("Tidak ada ulasan customer hari ini.", modalX + 55, fBoxY + 45, 13, { 140, 150, 160, 255 });
    } else {
        int rLineY = fBoxY + 26;
        for (int i = (int)dailyRatings.size() - 1; i >= 0 && i >= (int)dailyRatings.size() - 2; --i) {
            const auto& r = dailyRatings[i];
            std::string line = r.customerName + " (" + std::to_string(r.stars) + "/5): \"" + r.feedback + "\"";
            Color lineCol = (r.stars >= 4) ? Color{ 255, 215, 0, 255 } : Color{ 230, 126, 34, 255 };
            DrawText(line.c_str(), modalX + 55, rLineY, 12, lineCol);
            rLineY += 26;
        }
    }

    // Next Day Prompt
    int promptY = modalY + 438;
    DrawRectangle(modalX + 40, promptY, modalW - 80, 44, { 35, 75, 45, 250 });
    DrawRectangleLines(modalX + 40, promptY, modalW - 80, 44, { 50, 255, 120, 255 });
    DrawText(">> Tekan [ENTER] / [SPASI] untuk Memulai Hari Berikutnya <<", modalX + 52, promptY + 13, 14, RAYWHITE);
}
