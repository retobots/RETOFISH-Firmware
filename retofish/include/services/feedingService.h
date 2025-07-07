#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "hal/Button.h"


class FeedingService {
public:
    static FeedingService& getInstance();

    void setup();
    void loop();

private:
    FeedingService() = default;

    void renderSettingPage();  // 👉 hiển thị giao diện cài đặt

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

        // === Cấu hình bằng encoder === 
    enum class SettingPage {
        SelectSlot,
        SetHour,
        SetMinute,
        SetDuration,
        SetEnabled,      
        ConfirmSave
    };

    bool _inSettingMode = false;
    SettingPage _settingPage = SettingPage::SelectSlot;
    int _selectedSlot = 0;
    int _hour = 0;
    int _minute = 0;
    int _duration = 4;
    int _confirmIndex = 0;  // 0: Yes, 1: No
    bool _enabled = true;

};
