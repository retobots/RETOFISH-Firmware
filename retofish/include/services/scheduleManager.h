
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
    const FeedTime* getSlot(int index) const;


private:
    ScheduleManager() = default;

    // ✅ Mảng lưu runtime slot có thể thay đổi từ menu cài đặt
    FeedTime _slots[3] = {
        {7, 0, 10},
        {11, 30, 10},
        {18, 0, 10}
    };
};
