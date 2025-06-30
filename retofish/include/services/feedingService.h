// 	Quản lý toàn bộ tiến trình cho ăn: xử lý yêu cầu, kiểm tra pin, điều khiển motor, cập nhật trạng thái.
#pragma once
#include <Arduino.h>

class FeedingService
{
public:
    static FeedingService &getInstance();

    void setup();  // Gọi trong setup()
    void loop();   // Gọi trong loop()

private:
    FeedingService() = default;

    bool _feeding = false;
    bool _warnSpam = false;
    bool _screenOn = false;

    unsigned long _feedingStartTime = 0;
    unsigned long _lastManualFeedTime = 0;
    unsigned long _lastAutoFeedTime = 0;
    unsigned long _warnStartTime = 0;
    unsigned long _screenOnTime = 0;

    void handleButton();         // xử lý nút bấm
    void handleAutoFeeding();   // kiểm tra thời gian cho ăn tự động
    void updateStatus();        // cập nhật LED và màn hình
    void updateDisplayAndLed();  

};
