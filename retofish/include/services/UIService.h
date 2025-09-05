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
        SetHour,
        SetMinute,
        SetDuration,
        SetEnabled,
        ConfirmSave,
        NewPage,
        SetSave,
        SetMinute_1,
        SetHour_1 // mới
    };
    void setup();
    void loop();
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

    void renderNumber(int x, int y, int value, int size, uint16_t color);

    SettingPageState _settingPageState = SettingPageState::SelectSlot;

    Screen _screen = Screen::Home;

    bool _startSettingPage = true;
    int _selectedSlot = 0;
    int _hour = 0;
    int _minute = 0;
    int _duration = 4;
    int _confirmIndex = 0; // 0: Yes, 1: No
    bool _enabled = true;
    bool _lastChargingState = false;
    int _time1 = 0;
    int _time2 = 0;
    int _time3 = 0;
    uint32_t _screenOnTime = 0; // Thời gian màn hình được bật
};
