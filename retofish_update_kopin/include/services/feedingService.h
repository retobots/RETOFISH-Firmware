#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "hal/Button.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


class FeedingService {
public:
    static FeedingService& getInstance();

    void setup();
    void loop();

private:
    FeedingService() = default;

    void renderSettingPage();  // 👉 hiển thị giao diện cài đặt
    void renderNumber(int x, int y, int value, int size, uint16_t color); 

    void handleButton();              // Xử lý nút nhấn (Click / DoubleClick)
    void handleButton(Button::Event evt);
    void handleSetting(int delta, Button::Event evt); 
    void handleAutoFeeding();         // Kiểm tra và cho ăn tự động khi đến giờ
    void updateDisplayAndLed();       // Hiển thị trạng thái và pin
    void checkScreenTimeout();        // Tự tắt màn hình sau 15s
    void checkFeedingTimeout();       // Tắt trạng thái Feeding sau đúng thời gian motor chạy
    void checkWarningTimeout();       // Xóa cảnh báo spam sau 5s

    bool _screenOn = false;
    unsigned long _screenOnTime = 0;

    bool _feeding = false;
    unsigned long _feedingStartTime = 0;
    unsigned long _feedingDuration = 0;   // 🕒 thời gian motor chạy (ms), lấy từ StepperMotor

    bool _warnSpam = false;
    unsigned long _warnStartTime = 0;

    unsigned long _lastManualFeedTime = 0;
    unsigned long _lastAutoFeedTime = 0;

    // FeedingService.h (hoặc ở đầu file .cpp nếu bạn để private ở đó)
    unsigned long _lastAnyFeedTime = 0;   // mốc lần cho ăn gần nhất (tự động / thủ công)
   
    static constexpr unsigned long FEED_COOLDOWN_MS = 30000; // 30s
    
    bool        _holdFeeding = false;     // đang “giữ để cho ăn”
    uint32_t    _lastRawHold = 0;         // độ dài giữ nút ở vòng lặp trước (ms)

        // === Cấu hình bằng encoder === 
    enum class SettingPage {
        SelectSlot,
        SetHour,
        SetMinute,
        SetDuration,
        SetEnabled,      
        ConfirmSave,
        NewPage,
        SetSave,
        SetMinute_1,
        SetHour_1             // mới
    };

    bool _inSettingMode = false;
    SettingPage _settingPage = SettingPage::SelectSlot;
    int _selectedSlot = 0;
    int _hour = 0;
    int _minute = 0;
    int _duration = 4;
    int _confirmIndex = 0;  // 0: Yes, 1: No
    bool _enabled = true;
    bool _lastChargingState = false;
    bool _inStandbyMode = true;  
    int _time1 = 0;
    int _time2 = 0;
    int _time3 = 0;

};
