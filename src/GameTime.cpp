#include "GameTime.hpp"
#include <cstdio>

GameTime::GameTime()
    : currentDay(1), currentHour(8), currentMinute(0), secondAccumulator(0.0f),
      isShopOpen(true), showDailySummary(false),
      warned1Hour(false), warnedClosingSoon(false), notifiedClosed(false) {}

void GameTime::Init() {
    currentDay = 1;
    currentHour = 8;
    currentMinute = 0;
    secondAccumulator = 0.0f;
    isShopOpen = true;
    showDailySummary = false;
    warned1Hour = false;
    warnedClosingSoon = false;
    notifiedClosed = false;
}

void GameTime::Update(float deltaTime, std::string& outNotification, Color& outNoticeColor) {
    // If daily summary modal is active or past 21:00, freeze time progression until player starts next day
    if (showDailySummary) {
        return;
    }

    // 1 second real time = 1 minute game time
    secondAccumulator += deltaTime;
    while (secondAccumulator >= 1.0f) {
        secondAccumulator -= 1.0f;
        currentMinute++;

        if (currentMinute >= 60) {
            currentMinute = 0;
            currentHour++;
        }

        // Warnings Check
        if (currentHour == 20 && currentMinute == 0 && !warned1Hour) {
            warned1Hour = true;
            outNotification = "1 jam lagi toko tutup (21:00).";
            outNoticeColor = { 230, 126, 34, 235 }; // Orange
        }

        if (currentHour == 20 && currentMinute == 30 && !warnedClosingSoon) {
            warnedClosingSoon = true;
            outNotification = "Toko akan segera tutup!";
            outNoticeColor = { 231, 76, 60, 235 }; // Red
        }

        // Closing Time (21:00)
        if (currentHour >= 21 && !notifiedClosed) {
            currentHour = 21;
            currentMinute = 0;
            isShopOpen = false;
            notifiedClosed = true;
            showDailySummary = true; // Trigger daily summary modal
            outNotification = "Toko ditutup.";
            outNoticeColor = { 180, 40, 40, 235 };
        }
    }
}

std::string GameTime::GetFormattedTime() const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", currentHour, currentMinute);
    return std::string(buffer);
}

std::string GameTime::GetDayString() const {
    return "Day " + std::to_string(currentDay);
}

TimeOfDay GameTime::GetTimeOfDay() const {
    if (currentHour < 12) {
        return TimeOfDay::MORNING;
    } else if (currentHour < 18) {
        return TimeOfDay::DAY;
    } else {
        return TimeOfDay::EVENING;
    }
}

Color GameTime::GetSkyColor() const {
    TimeOfDay tod = GetTimeOfDay();
    switch (tod) {
        case TimeOfDay::MORNING:
            return { 135, 206, 235, 255 }; // Sky Blue (08:00 - 11:59)
        case TimeOfDay::DAY:
            return { 100, 180, 240, 255 }; // Bright Clear Day (12:00 - 17:59)
        case TimeOfDay::EVENING:
        default:
            return { 50, 60, 90, 255 };   // Dark Twilight / Night (18:00 - 21:00)
    }
}

void GameTime::StartNextDay(std::string& outNotification, Color& outNoticeColor) {
    currentDay++;
    currentHour = 8;
    currentMinute = 0;
    secondAccumulator = 0.0f;
    isShopOpen = true;
    showDailySummary = false;
    warned1Hour = false;
    warnedClosingSoon = false;
    notifiedClosed = false;

    outNotification = "Toko dibuka! (Hari " + std::to_string(currentDay) + " - 08:00)";
    outNoticeColor = { 46, 204, 113, 235 }; // Green
}
