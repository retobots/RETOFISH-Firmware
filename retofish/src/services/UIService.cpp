#include "services/UIService.h"
#include "hal/RTC.h"
#include "hal/StatusLed.h"
#include "hal/TftDisplay.h"
#include "services/FeedingService.h"
#include "services/ScheduleManager.h"
#include "hal/Button.h"
#include "hal/config.h"

// ---------- Singleton ----------
UIService &UIService::getInstance()
{
    static UIService instance;
    return instance;
}

void UIService::setup()
{
    _screen = Screen::Home;
    _screenOnTime = millis();

    // State mặc định cho Setup
    _settingPageState = SettingPageState::NewPage;
    _startSettingPage = true;
    _selectedSlot = 0;
    _hour = 7;
    _minute = 0;
    _duration = 1;
    _enabled = true;
    _confirmIndex = 0;

    _errorSince = 0;
    _errorMsg[0] = '\0';
}

// ---------- Public API ----------
void UIService::setScreen(Screen s) { _screen = s; }
UIService::Screen UIService::getScreen() { return _screen; }
void UIService::setSettingPageState(SettingPageState state) { _settingPageState = state; }
void UIService::setScreenOnTime(uint32_t t) { _screenOnTime = t; }

// ---------- Frame update ----------
void UIService::updateScreen(Button::Event evt)
{
    if (_screen == Screen::Home)
    {
        updateHomePage();
    }
    else if (_screen == Screen::Setting)
    {
        updateSettingPage(evt);
    }
    checkScreenTimeout();
}

// ---------- Home ----------
void UIService::updateHomePage()
{
    if (_screen != Screen::Home)
        return;

    auto &tft = TftDisplay::getInstance();
    auto &led = StatusLed::getInstance();
    const bool isFeeding = FeedingService::getInstance().isFeeding();

    const char *statusStr = nullptr;
    if (isFeeding)
    {
        led.updateLed(StatusLed::State::Feeding);
        statusStr = "Feeding";
    }
    else
    {
        led.updateLed(StatusLed::State::Idle);
        statusStr = "Standby";
    }

    DateTime now = RTC::getInstance().now();
    const FeedTime *next = ScheduleManager::getInstance().getNextFeedTime(now);

    if (next)
    {
        int hour12 = next->hour % 12;
        if (hour12 == 0)
            hour12 = 12;
        const char *ampm = (next->hour < 12) ? "AM" : "PM";
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, next->minute, ampm);
        tft.showFullStatus(statusStr, timeStr);
    }
    else
    {
        tft.showFullStatus(statusStr, "No setting");
    }
}

// ---------- Setting flow ----------
void UIService::renderSettingPage()
{
    if (_screen != Screen::Setting)
    {
        return;
    }
    auto &tft = TftDisplay::getInstance();
    tft.clear();

    switch (_settingPageState)
    {
    case SettingPageState::NewPage:
    {
        printTitle(tft, "SETUP MENU");
        const char *options[] = {"Feeding Time", "System Time", "Back"};
        for (int i = 0; i < 3; ++i)
        {
            tft.setCursor(20, 50 + i * 30);
            tft.setTextSize(2);
            tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);
            tft.print(options[i]);
        }
        break;
    }

    case SettingPageState::SelectSlot:
    {
        printTitle(tft, "FEEDING SLOT");
        const char *labels[4] = {"Feeding 1/3", "Feeding 2/3", "Feeding 3/3", "BACK"};
        for (int i = 0; i < 4; ++i)
        {
            tft.setCursor(20, 50 + i * 30);
            tft.setTextSize(2);
            tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);
            tft.print(labels[i]);

            if (i < 3)
            {
                const FeedTime *ft = ScheduleManager::getInstance().getSlot(i);
                char timeStr[8] = "--:--";
                bool enabled = false;
                if (ft)
                {
                    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", ft->hour, ft->minute);
                    enabled = ft->enabled;
                }
                tft.setCursor(160, 50 + i * 30);
                tft.print(timeStr);
                tft.setCursor(240, 50 + i * 30);
                tft.setTextColor(enabled ? UiCfg::COLOR_OK : UiCfg::COLOR_ERR);
                tft.print(enabled ? "[V]" : "[X]");
                tft.setTextColor(ST77XX_WHITE);
            }
        }
        break;
    }

    case SettingPageState::SetSystemHour:
    {
        printTitle(tft, "SET SYSTEM HOUR");
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_VALUE);
        tft.setCursor(20, 70);
        tft.print("Hour: ");
        renderNumber(tft, 160, 70, _hour, 4, UiCfg::COLOR_VALUE);
        printHint(tft);
        break;
    }

    case SettingPageState::SetSystemMinute:
    {
        printTitle(tft, "SET SYSTEM MINUTE");
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_VALUE);
        tft.setCursor(20, 70);
        tft.print("Minute: ");
        renderNumber(tft, 200, 70, _minute, 4, UiCfg::COLOR_VALUE);
        printHint(tft);
        break;
    }

    case SettingPageState::SetSave:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(50, 20);
        tft.print("Save this setting?");
        const char *opts[2] = {"YES", "NO"};
        for (int i = 0; i < 2; ++i)
        {
            tft.setCursor(60 + i * 160, 100);
            tft.setTextSize(3);
            tft.setTextColor(i == _confirmIndex ? UiCfg::COLOR_OK : ST77XX_WHITE);
            tft.print(opts[i]);
        }
        break;
    }

    case SettingPageState::SetFeedingHour:
    {
        char label[32];
        snprintf(label, sizeof(label), "SET HOUR %d/3", _selectedSlot + 1);
        printTitle(tft, label, 20, 20);
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_VALUE);
        tft.setCursor(20, 70);
        tft.print("Hour: ");
        renderNumber(tft, 160, 70, _hour, 4, UiCfg::COLOR_VALUE);
        printHint(tft);
        break;
    }

    case SettingPageState::SetFeedingMinute:
    {
        char label[32];
        snprintf(label, sizeof(label), "SET MINUTE %d/3", _selectedSlot + 1);
        printTitle(tft, label, 20, 20);
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_VALUE);
        tft.setCursor(20, 70);
        tft.print("Minute: ");
        renderNumber(tft, 210, 70, _minute, 4, UiCfg::COLOR_VALUE);
        printHint(tft);
        break;
    }

    case SettingPageState::SetDuration:
    {
        char label[32];
        snprintf(label, sizeof(label), "SET ROUNDS %d/3", _selectedSlot + 1);
        printTitle(tft, label, 20, 20);
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_VALUE);
        tft.setCursor(20, 70);
        tft.print("Duration: ");
        renderNumber(tft, 240, 70, _duration, 4, UiCfg::COLOR_VALUE);
        printHint(tft);
        break;
    }

    case SettingPageState::SetEnabled:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(60, 20);
        tft.print("Enable this slot?");
        const char *opts[2] = {"YES", "NO"};
        for (int i = 0; i < 2; ++i)
        {
            tft.setCursor(60 + i * 160, 100);
            tft.setTextSize(3);
            tft.setTextColor(i == _confirmIndex ? UiCfg::COLOR_OK : ST77XX_WHITE);
            tft.print(opts[i]);
        }
        printHint(tft);
        break;
    }

    case SettingPageState::ConfirmSave:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(80, 20);
        tft.print("Save changes ?");
        const char *opts[2] = {"YES", "NO"};
        for (int i = 0; i < 2; ++i)
        {
            tft.setCursor(60 + i * 160, 100);
            tft.setTextSize(3);
            tft.setTextColor(i == _confirmIndex ? UiCfg::COLOR_OK : ST77XX_WHITE);
            tft.print(opts[i]);
        }
        break;
    }

    case SettingPageState::Error:
    {
        tft.setTextSize(4);
        tft.setTextColor(UiCfg::COLOR_ERR);
        tft.setCursor(90, 30);
        tft.print("ERROR");
        tft.setTextSize(2);
        tft.setTextColor(UiCfg::COLOR_ERR);
        tft.setCursor(10, 70);
        tft.print(_errorMsg);
        // Không delay ở đây; updateSettingPage sẽ tự thoát sau 3s
        break;
    }

    default:
        break;
    }
}

void UIService::updateSettingPage(Button::Event evt)
{
    if (_screen != Screen::Setting)
        return;

    auto &tft = TftDisplay::getInstance();
    const int delta = Button::getInstance().getRotationDelta();

    // lần đầu vào Setting
    if (_settingPageState == SettingPageState::NewPage && _startSettingPage)
    {
        tft.setScreen(true);
        _screenOnTime = millis();
        _selectedSlot = 0;
        _hour = 7;
        _minute = 0;
        _duration = 1;
        _confirmIndex = 0;
        _startSettingPage = false;
        renderSettingPage();
        return;
    }

    // Nếu đang ở trang lỗi → tự thoát sau 3s
    if (_settingPageState == SettingPageState::Error)
    {
        if (millis() - _errorSince >= UiCfg::ERROR_SHOW_MS)
        {
            _settingPageState = _returnStateAfterError; // quay về trang trước khi báo lỗi
            renderSettingPage();
        }
        return;
    }

    switch (_settingPageState)
    {
    case SettingPageState::NewPage:
    {
        if (delta)
        {
            _selectedSlot = constrain(_selectedSlot + delta, 0, 2);
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_selectedSlot == 0)
            { // Feeding Time
                _settingPageState = SettingPageState::SelectSlot;
            }
            else if (_selectedSlot == 1)
            { // System Time
                DateTime now = RTC::getInstance().now();
                _hour = now.hour();
                _minute = now.minute();
                _settingPageState = SettingPageState::SetSystemHour;
            }
            else
            { // Back
                tft.clear();
                tft.resetLastStatus();
                _startSettingPage = true;
                _screen = Screen::Home;
                setScreenOnTime(millis());
            }
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetSystemHour:
    {
        if (delta)
        {
            _hour = (_hour + delta + 24) % 24;
            tft.fillRect(160, 70, 100, 30, ST77XX_BLACK);
            renderNumber(tft, 160, 70, _hour, 4, UiCfg::COLOR_VALUE);
        }
        if (evt == Button::Event::Click)
        {
            _settingPageState = SettingPageState::SetSystemMinute;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetSystemMinute:
    {
        if (delta)
        {
            _minute = (_minute + delta + 60) % 60;
            tft.fillRect(200, 70, 150, 30, ST77XX_BLACK);
            renderNumber(tft, 200, 70, _minute, 4, UiCfg::COLOR_VALUE);
        }
        if (evt == Button::Event::Click)
        {
            _settingPageState = SettingPageState::SetSave;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetSave:
    {
        if (delta)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2;
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_confirmIndex == 0)
            {
                // giữ nguyên Y/M/D, thay H/M
                DateTime cur = RTC::getInstance().now();
                RTC::getInstance().setTime(cur.year(), cur.month(), cur.day(), _hour, _minute, 0);
                Serial.printf("Time set to: %02d:%02d\n", _hour, _minute);
            }
            _settingPageState = SettingPageState::NewPage;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SelectSlot:
    {
        if (delta)
        {
            _selectedSlot = constrain(_selectedSlot + delta, 0, 3); // 0..2 slot, 3 back
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_selectedSlot == 3)
            {
                _settingPageState = SettingPageState::NewPage;
            }
            else
            {
                const FeedTime *ft = ScheduleManager::getInstance().getSlot(_selectedSlot);
                if (ft)
                {
                    _hour = ft->hour;
                    _minute = ft->minute;
                    _duration = ft->duration;
                    _enabled = ft->enabled;
                }
                else
                {
                    _hour = 7;
                    _minute = 0;
                    _duration = 1;
                    _enabled = true;
                }
                _settingPageState = SettingPageState::SetFeedingHour;
            }
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetFeedingHour:
    {
        if (delta)
        {
            _hour = (_hour + delta + 24) % 24;
            tft.fillRect(160, 70, 100, 30, ST77XX_BLACK);
            renderNumber(tft, 160, 70, _hour, 4, UiCfg::COLOR_VALUE);
        }
        if (evt == Button::Event::Click)
        {
            _settingPageState = SettingPageState::SetFeedingMinute;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetFeedingMinute:
    {
        if (delta)
        {
            _minute = (_minute + delta + 60) % 60;
            tft.fillRect(210, 70, 100, 30, ST77XX_BLACK);
            renderNumber(tft, 210, 70, _minute, 4, UiCfg::COLOR_VALUE);
        }
        if (evt == Button::Event::Click)
        {
            _settingPageState = SettingPageState::SetDuration;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetDuration:
    {
        if (delta)
        {
            _duration = constrain(_duration + delta, 1, 10);
            tft.fillRect(240, 70, 100, 30, ST77XX_BLACK);
            renderNumber(tft, 240, 70, _duration, 4, UiCfg::COLOR_VALUE);
        }
        if (evt == Button::Event::Click)
        {
            _settingPageState = SettingPageState::SetEnabled;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::SetEnabled:
    {
        if (delta)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2;
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            _enabled = (_confirmIndex == 0);
            _settingPageState = SettingPageState::ConfirmSave;
            renderSettingPage();
        }
        break;
    }

    case SettingPageState::ConfirmSave:
    {
        if (delta)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2;
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_confirmIndex == 0)
            {
                // Validate khoảng cách ≥ 10 phút với các slot khác đang enabled
                const int newMin = _hour * 60 + _minute;
                for (int i = 0; i < 3; ++i)
                {
                    if (i == _selectedSlot)
                        continue;
                    const FeedTime *other = ScheduleManager::getInstance().getSlot(i);
                    if (other && other->enabled)
                    {
                        const int otherMin = other->hour * 60 + other->minute;
                        if (!timeApartAtLeast10(newMin, otherMin))
                        {
                            snprintf(_errorMsg, sizeof(_errorMsg), "Time must be >=10 min apart.");
                            _errorSince = millis();
                            _returnStateAfterError = SettingPageState::SetFeedingHour; // quay lại chỉnh giờ
                            _settingPageState = SettingPageState::Error;
                            renderSettingPage();
                            return;
                        }
                    }
                }

                // Save
                ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration, _enabled);
                ScheduleManager::getInstance().saveToEEPROM();
                Serial.printf("Saved: Slot %d = %02d:%02d for %ds (%s)\n",
                              _selectedSlot + 1, _hour, _minute, _duration, _enabled ? "EN" : "DIS");
            }
            else
            {
                Serial.println("Cancel Save");
            }
            _settingPageState = SettingPageState::SelectSlot;
            renderSettingPage();
        }
        break;
    }

    default:
        break;
    }
}

// ---------- Screen timeout ----------
void UIService::checkScreenTimeout()
{
    // Không tắt khi đang Feeding hoặc ở trang Setting (F04/F05)
    if (FeedingService::getInstance().isFeeding() || _screen == Screen::Setting)
        return;

    auto &tft = TftDisplay::getInstance();
    if (_screen == Screen::Home && tft.isScreenON() && (millis() - _screenOnTime > UiCfg::SCREEN_TIMEOUT_MS))
    {
        tft.turnOffScreen(); // để TftDisplay tự lo backlight nếu có
    }
}
