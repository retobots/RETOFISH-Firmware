

#include "services/ScheduleManager.h"
#include <EEPROM.h>


ScheduleManager& ScheduleManager::getInstance() {
    static ScheduleManager instance;
    return instance;
}

void ScheduleManager::setup() {
    // Nếu có logic tải dữ liệu từ EEPROM hoặc Blynk thì thêm vào đây
    EEPROM.begin(128);         // ✅ Khởi động EEPROM với dung lượng 128 byte
    loadFromEEPROM();    
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

bool ScheduleManager::isTimeUsedByOtherSlot(int currentIndex, int hour, int minute) {
    for (int i = 0; i < 3; ++i) {
        if (i == currentIndex) continue;
        if (_slots[i].enabled && _slots[i].hour == hour && _slots[i].minute == minute) {
            return true;
        }
    }
    return false;
}

void ScheduleManager::saveToEEPROM() {
    for (int i = 0; i < 3; ++i) {
        EEPROM.put(i * sizeof(FeedTime), _slots[i]);
    }
    EEPROM.commit();  // Ghi vào flash
}

void ScheduleManager::loadFromEEPROM() {
    for (int i = 0; i < 3; ++i) {
        EEPROM.get(i * sizeof(FeedTime), _slots[i]);

        // Kiểm tra hợp lệ, nếu không thì đặt mặc định
        if (_slots[i].hour >= 24 || _slots[i].minute >= 60 || _slots[i].duration < 1 || _slots[i].duration > 10) {
            _slots[i] = FeedTime(7 + i * 2, 0, 1, true);  // VD: 7h, 9h, 11h
        }
    }
}
