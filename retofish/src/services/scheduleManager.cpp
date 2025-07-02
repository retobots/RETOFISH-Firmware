// #include "services/ScheduleManager.h"

// constexpr FeedTime ScheduleManager::schedule[];
// constexpr int ScheduleManager::scheduleCount;

// ScheduleManager& ScheduleManager::getInstance() {
//     static ScheduleManager instance;
//     return instance;
// }

// void ScheduleManager::setup() {
//     // Nếu có cấu hình qua Blynk hoặc EEPROM thì để đây
// }

// bool ScheduleManager::isTimeToFeed(const DateTime& now) {
//     for (int i = 0; i < scheduleCount; ++i) {
//         if (now.hour() == schedule[i].hour &&
//             now.minute() == schedule[i].minute &&
//             now.second() < 5) {
//                 Serial.printf("------- Đến giờ ăn ------");
//             return true;
//         }
//     }
//     return false;
// }

// const FeedTime* ScheduleManager::getNextFeedTime(const DateTime& now) {
//     for (int i = 0; i < scheduleCount; ++i) {
//         if (now.hour() < schedule[i].hour ||
//             (now.hour() == schedule[i].hour && now.minute() < schedule[i].minute)) {
//             return &schedule[i];
//         }
//     }
//     return &schedule[0];  // hôm nay hết lịch → lấy mốc đầu tiên ngày mai
// }

// void ScheduleManager::updateSlot(int index, int hour, int minute, int duration) {
//     if (index < 0 || index >= 3) return;
//     _slots[index].hour = hour;
//     _slots[index].minute = minute;
//     _slots[index].duration = duration;

//     Serial.printf("📦 Cập nhật slot %d → %02d:%02d – %ds\n", index + 1, hour, minute, duration);
// }

#include "services/ScheduleManager.h"

ScheduleManager& ScheduleManager::getInstance() {
    static ScheduleManager instance;
    return instance;
}

void ScheduleManager::setup() {
    // Nếu có logic tải dữ liệu từ EEPROM hoặc Blynk thì thêm vào đây
}

bool ScheduleManager::isTimeToFeed(const DateTime& now) {
    for (int i = 0; i < 3; ++i) {
        if (now.hour() == _slots[i].hour &&
            now.minute() == _slots[i].minute &&
            now.second() < 5) {
            Serial.println("------- Đến giờ ăn ------");
            return true;
        }
    }
    return false;
}

const FeedTime* ScheduleManager::getNextFeedTime(const DateTime& now) {
    for (int i = 0; i < 3; ++i) {
        if (now.hour() < _slots[i].hour ||
            (now.hour() == _slots[i].hour && now.minute() < _slots[i].minute)) {
            return &_slots[i];
        }
    }
    return &_slots[0];  // hôm nay hết lịch → lấy mốc đầu tiên ngày mai
}

void ScheduleManager::updateSlot(int index, int hour, int minute, int duration) {
    if (index < 0 || index >= 3) return;
    _slots[index].hour = hour;
    _slots[index].minute = minute;
    _slots[index].duration = duration;

    Serial.printf("📦 Cập nhật slot %d → %02d:%02d – %ds\n", index + 1, hour, minute, duration);
}
