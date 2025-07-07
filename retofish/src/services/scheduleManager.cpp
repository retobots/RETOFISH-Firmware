
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
        const FeedTime& slot = _slots[i];
        if (!slot.enabled) continue;  // Bỏ qua nếu tắt

        if (now.hour() < slot.hour ||
            (now.hour() == slot.hour && now.minute() < slot.minute)) {
            return &slot;
        }
    }

    // Nếu hôm nay không còn slot nào → tìm slot đầu tiên đã bật cho ngày mai
    for (int i = 0; i < 3; ++i) {
        if (_slots[i].enabled) {
            return &_slots[i];
        }
    }

    // Nếu không có slot nào bật → không có giờ kế tiếp
    return nullptr;
    
}

// void ScheduleManager::updateSlot(int index, int hour, int minute, int duration) {
//     if (index < 0 || index >= 3) return;
//     _slots[index].hour = hour;
//     _slots[index].minute = minute;
//     _slots[index].duration = duration;

//     Serial.printf("📦 Cập nhật slot %d → %02d:%02d – %ds\n", index + 1, hour, minute, duration);
// }
void ScheduleManager::updateSlot(int index, int hour, int minute, int duration, bool enabled) {
    if (index < 0 || index >= 3) return;
    _slots[index].hour = hour;
    _slots[index].minute = minute;
    _slots[index].duration = duration;
    _slots[index].enabled = enabled;  // ✅ lưu trạng thái bật/tắt slot

    Serial.printf("📦 Slot %d → %02d:%02d – %ds [%s]\n",
        index + 1, hour, minute, duration, enabled ? "ENABLED" : "DISABLED");
}


const FeedTime* ScheduleManager::getSlot(int index) const {
    if (index < 0 || index >= 3) return nullptr;
    return &_slots[index];
}

bool ScheduleManager::toggleSlotEnabled(int index) {
    if (index < 0 || index >= 3) return false;
    _slots[index].enabled = !_slots[index].enabled;
    return _slots[index].enabled;
}
