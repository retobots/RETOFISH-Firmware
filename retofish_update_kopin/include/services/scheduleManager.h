
#pragma once
#include <Arduino.h>
#include <RTClib.h>

#define TIME1_ADDR 0  // Địa chỉ EEPROM lưu Time 1
#define TIME2_ADDR 4  // Địa chỉ EEPROM lưu Time 2
#define TIME3_ADDR 8  // Địa chỉ EEPROM lưu Time 3
struct FeedTime {
    int hour;
    int minute;
    int duration;     // số vòng quay motor
    bool enabled = true;  // bật/tắt slot
    FeedTime(int h = 7, int m = 0, int d = 10, bool e = true)
        : hour(h), minute(m), duration(d), enabled(e) {}
};

class ScheduleManager {
public:
    static ScheduleManager& getInstance();

    void setup();

    // Kiểm tra có đến giờ cho ăn không (slot hợp lệ và enabled)
    bool isTimeToFeed(const DateTime& now);

    // Trả về slot tiếp theo (dù có enabled hay không – dùng cho hiển thị)
    const FeedTime* getNextFeedTime(const DateTime& now);

    // Truy cập slot theo chỉ số
    const FeedTime* getSlot(int index) const;

    // Cập nhật 1 slot (giờ, phút, duration)
    // void updateSlot(int index, int hour, int minute, int duration);
    void updateSlot(int index, int hour, int minute, int duration, bool enabled);  // ✅ SỬA THÊM enabled


    // Bật/tắt trạng thái enable của slot → trả về trạng thái sau khi đổi
    bool toggleSlotEnabled(int index);
    // kiểm tra trùng lập 
    bool isTimeUsedByOtherSlot(int currentIndex, int hour, int minute);

    void saveToEEPROM();
    void loadFromEEPROM();

        // Thêm các biến lưu trữ Time 1, Time 2, Time 3 từ EEPROM
    int _time1 = 0;
    int _time2 = 0;
    int _time3 = 0;



private:
    ScheduleManager() = default;

    // 3 khung giờ mặc định: 7h00, 11h30, 18h00 – quay 10 vòng – mặc định bật
    FeedTime _slots[3] = {
        {7, 0, 10, true},
        {11, 30, 10, true},
        {18, 0, 10, true}

    
    };
};
