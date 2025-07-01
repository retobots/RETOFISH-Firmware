#pragma once
#include <Arduino.h>
#include <RTClib.h>

struct FeedTime {
    uint8_t hour;
    uint8_t minute;
};

class ScheduleManager {
public:
    static ScheduleManager& getInstance();

    void setup();                                 // Cài đặt ban đầu (nếu cần)
    bool isTimeToFeed(const DateTime& now);       // Kiểm tra có đến giờ ăn chưa
    const FeedTime* getNextFeedTime(const DateTime& now); // Trả về thời gian ăn kế tiếp

private:
    ScheduleManager() = default;

    static constexpr FeedTime schedule[] = {
        {16, 55},
        {16, 57},
        {17, 14}
    };
    static constexpr int scheduleCount = sizeof(schedule) / sizeof(schedule[0]);
};
