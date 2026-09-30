#pragma once
#include <string>
#include "raylib.h"

enum class TimeOfDay {
    MORNING,    // 08:00 - 11:59
    DAY,        // 12:00 - 17:59
    EVENING     // 18:00 - 20:59
};

class GameTime {
public:
    GameTime();

    void Init();
    void Update(float deltaTime, std::string& outNotification, Color& outNoticeColor);

    // State queries
    int GetCurrentDay() const { return currentDay; }
    int GetCurrentHour() const { return currentHour; }
    int GetCurrentMinute() const { return currentMinute; }
    std::string GetFormattedTime() const;
    std::string GetDayString() const;

    bool IsShopOpen() const { return isShopOpen; }
    bool IsShopClosed() const { return !isShopOpen; }
    bool IsDaySummaryOpen() const { return showDailySummary; }
    void SetDaySummaryOpen(bool open) { showDailySummary = open; }

    TimeOfDay GetTimeOfDay() const;
    Color GetSkyColor() const;

    // Day lifecycle
    void StartNextDay(std::string& outNotification, Color& outNoticeColor);

private:
    int currentDay;
    int currentHour;
    int currentMinute;
    float secondAccumulator; // Accumulates deltaTime for 1 real second = 1 game minute

    bool isShopOpen;
    bool showDailySummary;

    // Warning flags to trigger exactly once per day
    bool warned1Hour;       // 20:00 warning
    bool warnedClosingSoon; // 20:30 warning
    bool notifiedClosed;    // 21:00 closing notice
};
