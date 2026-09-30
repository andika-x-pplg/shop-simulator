#include "Finance.hpp"

Finance::Finance()
    : currentBalance(100000),
      totalRevenue(0),
      totalExpenses(0),
      menuOpen(false),
      nextTransactionId(1)
{
}

Finance::Finance(int initialBalance)
    : currentBalance(initialBalance),
      totalRevenue(0),
      totalExpenses(0),
      menuOpen(false),
      nextTransactionId(1)
{
}

void Finance::Init(int initialBalance) {
    currentBalance = initialBalance;
    totalRevenue = 0;
    totalExpenses = 0;
    menuOpen = false;
    nextTransactionId = 1;
    recentTransactions.clear();
}

bool Finance::RecordExpense(int amount, const std::string& description) {
    if (amount <= 0) return false;
    if (currentBalance < amount) return false;

    currentBalance -= amount;
    totalExpenses += amount;

    FinancialTransaction tx;
    tx.id = nextTransactionId++;
    tx.type = "PENGELUARAN";
    tx.description = description;
    tx.amount = amount;
    tx.time = 0.0f;

    recentTransactions.insert(recentTransactions.begin(), tx);
    if (recentTransactions.size() > 10) {
        recentTransactions.pop_back();
    }
    return true;
}

void Finance::RecordRevenue(int amount, const std::string& description) {
    if (amount <= 0) return;

    currentBalance += amount;
    totalRevenue += amount;

    FinancialTransaction tx;
    tx.id = nextTransactionId++;
    tx.type = "PENDAPATAN";
    tx.description = description;
    tx.amount = amount;
    tx.time = 0.0f;

    recentTransactions.insert(recentTransactions.begin(), tx);
    if (recentTransactions.size() > 10) {
        recentTransactions.pop_back();
    }
}
