#pragma once
#include <string>
#include <vector>

struct RatingRecord {
    int customerId;
    std::string customerName;
    int stars;              // 1 - 5 bintang
    int satisfaction;       // 0 - 100
    std::string feedback;   // Feedback pesan teks
    std::string productName;// Produk yang dibeli atau ""
    bool didBuy;
};

class Reputation {
public:
    Reputation();
    ~Reputation() = default;

    void Init(int initialReputation = 50);

    // Reputasi (0 - 100)
    int GetReputation() const { return reputation; }
    void AddReputation(int points) {
        reputation = (reputation + points > 100) ? 100 : ((reputation + points < 0) ? 0 : (reputation + points));
    }
    
    // Rating Toko
    int GetTotalRatings() const { return totalRatings; }
    int GetTotalRatingPoints() const { return totalRatingPoints; }
    float GetAverageRating() const;
    bool HasRatings() const { return totalRatings > 0; }
    void LoadReputationData(int rep, int ratings, int ratingPoints) {
        reputation = rep;
        totalRatings = ratings;
        totalRatingPoints = ratingPoints;
    }

    // Catat Rating baru dari customer (dijamin dipanggil tepat 1 kali per customer)
    void RecordRating(int customerId, const std::string& customerName, int satisfaction, const std::string& productName, bool didBuy, int& outStars, std::string& outFeedback);

    // Riwayat rating customer
    const std::vector<RatingRecord>& GetRecentRatings() const { return recentRatings; }

    // Menu Reputasi UI State (Tombol R)
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    // Helper konversi satisfaction -> bintang (1 - 5)
    static int SatisfactionToStars(int satisfaction);

    // Helper konversi satisfaction -> feedback string
    static std::string SatisfactionToFeedback(int satisfaction);

private:
    int reputation;          // 0 - 100 (awal: 50)
    int totalRatings;        // Jumlah customer yang memberi rating
    int totalRatingPoints;   // Total poin bintang yang dikumpulkan
    bool menuOpen;

    std::vector<RatingRecord> recentRatings;
};
