#include "Reputation.hpp"
#include <algorithm>

Reputation::Reputation()
    : reputation(50),
      totalRatings(0),
      totalRatingPoints(0),
      menuOpen(false)
{
}

void Reputation::Init(int initialReputation) {
    reputation = std::clamp(initialReputation, 0, 100);
    totalRatings = 0;
    totalRatingPoints = 0;
    menuOpen = false;
    recentRatings.clear();
}

float Reputation::GetAverageRating() const {
    if (totalRatings == 0) {
        return 0.0f;
    }
    return static_cast<float>(totalRatingPoints) / static_cast<float>(totalRatings);
}

int Reputation::SatisfactionToStars(int satisfaction) {
    int clamped = std::clamp(satisfaction, 0, 100);
    if (clamped >= 90) return 5;
    if (clamped >= 75) return 4;
    if (clamped >= 55) return 3;
    if (clamped >= 35) return 2;
    return 1;
}

std::string Reputation::SatisfactionToFeedback(int satisfaction) {
    int clamped = std::clamp(satisfaction, 0, 100);
    if (clamped >= 90) return "Customer sangat puas!";
    if (clamped >= 75) return "Customer puas.";
    if (clamped >= 55) return "Customer cukup puas.";
    if (clamped >= 35) return "Customer kurang puas.";
    return "Customer kecewa.";
}

void Reputation::RecordRating(int customerId, const std::string& customerName, int satisfaction, const std::string& productName, bool didBuy, int& outStars, std::string& outFeedback) {
    int clampedSat = std::clamp(satisfaction, 0, 100);
    int stars = SatisfactionToStars(clampedSat);
    std::string feedback = SatisfactionToFeedback(clampedSat);

    totalRatings++;
    totalRatingPoints += stars;

    // Perubahan reputasi bertahap:
    // 5 bintang: +3 reputasi
    // 4 bintang: +1 reputasi
    // 3 bintang:  0
    // 2 bintang: -2 reputasi
    // 1 bintang: -3 reputasi
    int deltaRep = 0;
    if (stars == 5) deltaRep = 3;
    else if (stars == 4) deltaRep = 1;
    else if (stars == 3) deltaRep = 0;
    else if (stars == 2) deltaRep = -2;
    else if (stars == 1) deltaRep = -3;

    reputation = std::clamp(reputation + deltaRep, 0, 100);

    RatingRecord record;
    record.customerId = customerId;
    record.customerName = customerName;
    record.stars = stars;
    record.satisfaction = clampedSat;
    record.feedback = feedback;
    record.productName = productName;
    record.didBuy = didBuy;

    recentRatings.insert(recentRatings.begin(), record);
    if (recentRatings.size() > 10) {
        recentRatings.pop_back();
    }

    outStars = stars;
    outFeedback = feedback;
}
