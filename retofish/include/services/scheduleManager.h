// #pragma once
// #include <Arduino.h>
// #include <RTClib.h>

// struct FeedTime {
//     uint8_t hour;
//     uint8_t minute;
// };

// class ScheduleManager {
// public:
//     static ScheduleManager& getInstance();

//     void setup();                                 // Cài đặt ban đầu (nếu cần)
//     bool isTimeToFeed(const DateTime& now);       // Kiểm tra có đến giờ ăn chưa
//     const FeedTime* getNextFeedTime(const DateTime& now); // Trả về thời gian ăn kế tiếp
    

// private:
//     ScheduleManager() = default;

//     static constexpr FeedTime schedule[] = {
//         {16, 55},
//         {16, 57},
//         {17, 14}
//     };
//     static constexpr int scheduleCount = sizeof(schedule) / sizeof(schedule[0]);
// };

#pragma once
#include <Arduino.h>
#include <RTClib.h>

struct FeedTime {
    int hour;
    int minute;
    int duration;
};

class ScheduleManager {
public:
    static ScheduleManager& getInstance();

    void setup();
    bool isTimeToFeed(const DateTime& now);
    const FeedTime* getNextFeedTime(const DateTime& now);
    void updateSlot(int index, int hour, int minute, int duration);

private:
    ScheduleManager() = default;

    // ✅ Mảng lưu runtime slot có thể thay đổi từ menu cài đặt
    FeedTime _slots[3] = {
        {7, 0, 10},
        {11, 30, 10},
        {18, 0, 10}
    };
};
