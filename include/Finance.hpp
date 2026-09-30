#pragma once
#include <string>
#include <vector>

struct FinancialTransaction {
    int id;
    std::string type;       // "PENDAPATAN" atau "PENGELUARAN"
    std::string description;
    int amount;
    float time;
};

class Finance {
public:
    Finance();
    explicit Finance(int initialBalance);
    ~Finance() = default;

    void Init(int initialBalance = 100000);

    // Saldo
    int GetCurrentBalance() const { return currentBalance; }
    
    // Total Revenue & Expenses
    int GetTotalRevenue() const { return totalRevenue; }
    int GetTotalExpenses() const { return totalExpenses; }

    // Keuntungan / Kerugian: Profit = totalRevenue - totalExpenses
    int GetTotalProfit() const { return totalRevenue - totalExpenses; }
    bool IsProfit() const { return GetTotalProfit() >= 0; }

    // Pencatatan Transaksi
    bool RecordExpense(int amount, const std::string& description);
    void RecordRevenue(int amount, const std::string& description);

    // UI State Keuangan
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    // Riwayat transaksi terbaru
    const std::vector<FinancialTransaction>& GetRecentTransactions() const { return recentTransactions; }

private:
    int currentBalance;
    int totalRevenue;
    int totalExpenses;
    
    bool menuOpen;
    int nextTransactionId;
    std::vector<FinancialTransaction> recentTransactions;
};
