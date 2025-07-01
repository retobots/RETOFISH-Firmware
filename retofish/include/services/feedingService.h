// // 	Quản lý toàn bộ tiến trình cho ăn: xử lý yêu cầu, kiểm tra pin, điều khiển motor, cập nhật trạng thái.
// #pragma once
// #include <Arduino.h>

// class FeedingService
// {
// public:
//     static FeedingService &getInstance();

//     void setup();  // Gọi trong setup()
//     void loop();   // Gọi trong loop()

// private:
//     FeedingService() = default;

//     bool _feeding = false;
//     bool _warnSpam = false;
//     bool _screenOn = false;

//     unsigned long _feedingStartTime = 0;
//     unsigned long _lastManualFeedTime = 0;
//     unsigned long _lastAutoFeedTime = 0;
//     unsigned long _warnStartTime = 0;
//     unsigned long _screenOnTime = 0;

//     void handleButton();         // xử lý nút bấm
//     void handleAutoFeeding();   // kiểm tra thời gian cho ăn tự động
//     void updateStatus();        // cập nhật LED và màn hình
//     void updateDisplayAndLed();  

// };
#pragma once
#include <Arduino.h>
#include <RTClib.h>

class FeedingService {
public:
    static FeedingService& getInstance();

    void setup();
    void loop();

private:
    FeedingService() = default;

    void handleButton();              // Xử lý nút nhấn (Click / DoubleClick)
    void handleAutoFeeding();        // Kiểm tra và cho ăn tự động khi đến giờ
    void updateDisplayAndLed();      // Hiển thị trạng thái và pin
    void checkScreenTimeout();       // Tự tắt màn hình sau 15s
    void checkFeedingTimeout();      // Tắt trạng thái Feeding sau 5s
    void checkWarningTimeout();      // Xóa cảnh báo spam sau 5s

    bool _screenOn = false;
    unsigned long _screenOnTime = 0;

    bool _feeding = false;
    unsigned long _feedingStartTime = 0;

    bool _warnSpam = false;
    unsigned long _warnStartTime = 0;

    unsigned long _lastManualFeedTime = 0;
    unsigned long _lastAutoFeedTime = 0;
};
