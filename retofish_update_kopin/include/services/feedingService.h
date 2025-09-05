#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "hal/Button.h"
#include "services/UIService.h"


class FeedingService {
public:
    static FeedingService& getInstance();

    void setup();
    void loop();
    bool isFeeding();
    void setFeeding(bool f);
    void feeding(float level, bool disableAfterFeeding);

private:
    FeedingService() = default;

    void renderSettingPage();  // 👉 hiển thị giao diện cài đặt
    void renderNumber(int x, int y, int value, int size, uint16_t color); 

    void handleButton();              // Xử lý nút nhấn (Click / DoubleClick)

    void handleAutoFeeding();         // Kiểm tra và cho ăn tự động khi đến giờ

    

    bool _feeding = false;
    unsigned long _feedingDuration = 0;   // 🕒 thời gian motor chạy (ms), lấy từ StepperMotor

    unsigned long _warnStartTime = 0;

    unsigned long _lastAutoFeedTime = 0;
   
    static constexpr unsigned long FEED_COOLDOWN_MS = 30000; // 30s
    
   
    uint32_t    _lastRawHold = 0;         // độ dài giữ nút ở vòng lặp trước (ms)

        // === Cấu hình bằng encoder === 
    


};
