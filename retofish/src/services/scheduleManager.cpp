#include "services/ScheduleManager.h"

constexpr FeedTime ScheduleManager::schedule[];
constexpr int ScheduleManager::scheduleCount;

ScheduleManager& ScheduleManager::getInstance() {
    static ScheduleManager instance;
    return instance;
}

void ScheduleManager::setup() {
    // Nếu có cấu hình qua Blynk hoặc EEPROM thì để đây
}

bool ScheduleManager::isTimeToFeed(const DateTime& now) {
    for (int i = 0; i < scheduleCount; ++i) {
        if (now.hour() == schedule[i].hour &&
            now.minute() == schedule[i].minute &&
            now.second() < 5) {
                Serial.printf("------- Đến giờ ăn ------");
            return true;
        }
    }
    return false;
}

const FeedTime* ScheduleManager::getNextFeedTime(const DateTime& now) {
    for (int i = 0; i < scheduleCount; ++i) {
        if (now.hour() < schedule[i].hour ||
            (now.hour() == schedule[i].hour && now.minute() < schedule[i].minute)) {
            return &schedule[i];
        }
    }
    return &schedule[0];  // hôm nay hết lịch → lấy mốc đầu tiên ngày mai
}
