// Định kỳ gửi trạng thái lên cloud, cập nhật LED & OLED theo trạng thái hệ thống.
#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "hal/TftDisplay.h"
#include "hal/RTC.h"
#include "hal/StatusLed.h"
#include "services/feedingService.h"
#include "services/scheduleManager.h"
#include "hal/Button.h"

class UIService
{
public:
    static UIService &getInstance();

    enum class Screen
    {
        Home,
        Setting,
    };
    enum class SettingPageState
    {
        SelectSlot,
        SetFeedingHour,
        SetFeedingMinute,
        SetDuration,
        SetEnabled,
        ConfirmSave,
        NewPage,
        SetSave,
        SetSystemMinute,
        SetSystemHour,
        Error
    };
    void setup();
    void updateHomePage();
    Screen getScreen();
    void setScreen(Screen s);
    void setScreenOnTime(uint32_t t);
    void checkScreenTimeout();
    void renderSettingPage();
    void updateSettingPage(Button::Event evt);
    void updateScreen(Button::Event evt);
    void setSettingPageState(SettingPageState state);

private:
    UIService() = default;

    inline void printTitle(TftDisplay &tft, const char *title, int x = 20, int y = 10)
    {
        tft.setTextSize(2);
        tft.setTextColor(UiCfg::COLOR_TITLE);
        tft.setCursor(x, y);
        tft.print(title);
    }

    inline void printHint(TftDisplay &tft, int x = 20, int y = 130)
    {
        tft.setTextSize(2);
        tft.setTextColor(UiCfg::COLOR_HINT);
        tft.setCursor(x, y);
        tft.print("Rotate to adjust,");
        tft.setCursor(x, y + 20);
        tft.print("Click to confirm");
    }

    inline void renderNumber(TftDisplay &tft, int x, int y, int value, int size, uint16_t color)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d", value);
        tft.setTextSize(size);
        tft.setTextColor(color);
        tft.setCursor(x, y);
        tft.print(buf);
    }

    inline bool timeApartAtLeast10(int m1, int m2)
    {
        return (abs(m1 - m2) >= 10);
    }

    SettingPageState _settingPageState = SettingPageState::SelectSlot;

    Screen _screen = Screen::Home;

    bool _startSettingPage = true;
    int _selectedSlot = 0;
    int _hour = 0;
    int _minute = 0;
    int _duration = 4;
    int _confirmIndex = 0; // 0: Yes, 1: No
    bool _enabled = true;
    int _time1 = 0;
    int _time2 = 0;
    int _time3 = 0;
    uint32_t _screenOnTime = 0;              // Thời gian màn hình được bật
    uint32_t _errorSince;                    // thời điểm bắt đầu hiển thị lỗi
    char _errorMsg[64];                      // nội dung thông báo lỗi
    SettingPageState _returnStateAfterError; // quay lại state nào sau khi hết lỗi
};
