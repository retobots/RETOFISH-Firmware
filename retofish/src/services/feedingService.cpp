
#include "services/FeedingService.h"
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"
#include "hal/StatusLed.h"
#include "services/ScheduleManager.h"

FeedingService& FeedingService::getInstance() {
    static FeedingService instance;
    return instance;
}

void FeedingService::setup() {
    _screenOn = true;
    _screenOnTime = millis();
    _lastManualFeedTime = millis() - 30000;
    _lastAutoFeedTime = 0;
}

void FeedingService::loop() {
    Button& btn = Button::getInstance();
    btn.update();
    auto evt = btn.getEvent();
    int delta = btn.getRotationDelta();

    if (_inSettingMode) {
        handleSetting(delta, evt);
        return;
    }

    handleButton(evt);
    handleAutoFeeding();
    checkScreenTimeout();
    checkFeedingTimeout();
    checkWarningTimeout();
    updateDisplayAndLed();
    delay(20);
}
/// moi ne


void FeedingService::handleSetting(int delta, Button::Event evt) {
    switch (_settingPage) {
        case SettingPage::SelectSlot:
            if (delta != 0) {
                _selectedSlot = constrain(_selectedSlot + delta, 0, 3); // 0-2: slot, 3: back
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                if (_selectedSlot == 3) {
                    _inSettingMode = false;  //
                    _screenOnTime = millis(); //
                    auto& display = TftDisplay::getInstance();
                    display.clear();
                    // ✅ XÓA CACHE để bắt buộc vẽ lại mọi thứ
                    display.resetLastStatus();  // bạn sẽ thêm hàm này ở bước dưới

    updateDisplayAndLed();

                } else {
                    const FeedTime* ft = ScheduleManager::getInstance().getSlot(_selectedSlot);
                    if (ft) {
                        _hour = ft->hour;
                        _minute = ft->minute;
                        _duration = ft->duration;
                    } else {
                        _hour = 7;
                        _minute = 0;
                        _duration = 10;
                    }
                    _settingPage = SettingPage::SetHour;
                    renderSettingPage();
                }
            }    // them if 
            //     if (evt == Button::Event::DoubleClick && _selectedSlot < 3) {
            //         bool newState = ScheduleManager::getInstance().toggleSlotEnabled(_selectedSlot);
            //         Serial.printf("🔁 Slot %d → %s\n", _selectedSlot + 1, newState ? "ENABLED ✔️" : "DISABLED ❌");
            //         renderSettingPage();
            // }
            break;

        case SettingPage::SetHour:
            if (delta != 0) {
                _hour = (_hour + delta + 24) % 24;
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetMinute;
                renderSettingPage();
            }
            break;

        case SettingPage::SetMinute:
            if (delta != 0) {
                _minute = (_minute + delta + 60) % 60;
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetDuration;
                renderSettingPage();
            }
            break;

        case SettingPage::SetDuration:
            if (delta != 0) {
                _duration = constrain(_duration + delta, 4, 10);  // 4-10 số vòng đc chọn
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetEnabled;
                renderSettingPage();
            }
            break;

        case SettingPage::SetEnabled:
            if (delta != 0) {
                _confirmIndex = (_confirmIndex + delta + 2) % 2;  // ✅ Xoay chọn YES/NO
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _enabled = (_confirmIndex == 0);  // ✅ YES → bật, NO → tắt
                _settingPage = SettingPage::ConfirmSave;
                renderSettingPage();
            }
            break;

        case SettingPage::ConfirmSave:
            if (delta != 0) {
                _confirmIndex = (_confirmIndex + delta + 2) % 2;
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                if (_confirmIndex == 0) {
                    // Serial.printf("✅ Saved: Slot %d = %02d:%02d for %ds\n", _selectedSlot + 1, _hour, _minute, _duration);
                    // ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration);
                    
                     Serial.printf("✅ Saved: Slot %d = %02d:%02d for %ds (%s)\n",
                    _selectedSlot + 1, _hour, _minute, _duration, _enabled ? "ENABLED" : "DISABLED");

    
                    ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration, _enabled);
                } else {
                    Serial.printf("❌ Cancel Save\n");
                }
                _settingPage = SettingPage::SelectSlot;
                renderSettingPage();
            }
            break;
    }
}

void FeedingService::handleButton(Button::Event evt) {
    unsigned long now = millis();

    if (evt == Button::Event::HoldLong) {
        Serial.println("Vao che do setting");
        _inSettingMode = true;
        _settingPage = SettingPage::SelectSlot;
        _screenOn = true;
        _screenOnTime = now;
        _selectedSlot = 0;
        _hour = 7;
        _minute = 0;
        _duration = 10;
        _confirmIndex = 0;
        renderSettingPage();
        
        return;
    }

    if (evt == Button::Event::Click) {
        if (!_screenOn) {
            _screenOn = true;
            _screenOnTime = now;
            _warnSpam = false;
            Serial.println("Screen ON");
        }
    }

    if (evt == Button::Event::DoubleClick) {
    if (!_screenOn) {
        _screenOn = true;
        _screenOnTime = now;
        _warnSpam = false;
        Serial.println("Screen ON");
    } else {
        if (now - _lastManualFeedTime > 30000) {
            _feeding = true;
            _warnSpam = false;
            Serial.println("Feeding START → 4 vòng");
            updateDisplayAndLed();
            StepperMotor::getInstance().feedForRounds(4);  // ✅ QUAY 4 VÒNG  định

            _feedingStartTime = now;
            _lastManualFeedTime = now;
            
            updateDisplayAndLed();
        } else {
            _warnSpam = true;
            _warnStartTime = now;
            Serial.println("Feed ignored → PLEASE WAIT");
            updateDisplayAndLed();
        }
    }
}

}

void FeedingService::updateDisplayAndLed() {
    float voltage = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();

    const char* statusStr;
    auto& led = StatusLed::getInstance();

    if (_warnSpam) {
        statusStr = "Wait 30s";
    } else if (_feeding) {
        led.setStatus(StatusLed::State::Feeding);
        statusStr = "Feeding";
    } else if (level < 15) {
        led.setStatus(StatusLed::State::LowBattery);
        statusStr = "Low Battery";
    } else {
        led.setStatus(StatusLed::State::Idle);
        statusStr = "Standby";
    }

    led.update();

    if (_screenOn) {

        DateTime now = RTC::getInstance().now();
        const FeedTime* next = ScheduleManager::getInstance().getNextFeedTime(now);

        if (next) {
            int hour12 = next->hour % 12;
            if (hour12 == 0) hour12 = 12;
            const char* ampm = next->hour < 12 ? "AM" : "PM";

            char timeStr[16];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, next->minute, ampm);

            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, timeStr);
        } else {
            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, "No setting");
        }
    }
}

void FeedingService::handleAutoFeeding() {         /////// auto 
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();

    if (ScheduleManager::getInstance().isTimeToFeed(nowRtc)) {
        _feeding = true;
        Serial.println("[Auto] Feeding START");
        _screenOn = true;
        _screenOnTime = now;
        updateDisplayAndLed();
        StepperMotor::getInstance().feedForRounds(_duration); 
        _feedingStartTime = now;
        _lastAutoFeedTime = now;
        

    }
}

void FeedingService::checkScreenTimeout() {
    if (_inSettingMode) return;
    if (_screenOn && (millis() - _screenOnTime > 15000)) {
        _screenOn = false;
        TftDisplay::getInstance().turnOff();
    }
}

void FeedingService::checkFeedingTimeout() {
    if (_feeding && (millis() - _feedingStartTime > _feedingDuration)) {
        _feeding = false;
        updateDisplayAndLed();
    }
}

void FeedingService::checkWarningTimeout() {
    if (_warnSpam && (millis() - _warnStartTime > 5000)) {
        _warnSpam = false;
    }
}


void FeedingService::renderSettingPage() {
    auto& tft = TftDisplay::getInstance();
    tft.clear();

    switch (_settingPage) {

        case SettingPage::SelectSlot: {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(20, 10);
            tft.print("Select feeding time");

            const char* labels[4] = { "Timer 1/3", "Timer 2/3", "Timer 3/3", "BACK" };

            for (int i = 0; i < 4; i++) {
                tft.setCursor(40, 50 + i * 30);
                tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);
                tft.print(labels[i]);

                // Nếu là slot 0–2 → hiển thị trạng thái enable
                if (i < 3) {
                    const FeedTime* ft = ScheduleManager::getInstance().getSlot(i);
                    tft.setCursor(220, 50 + i * 30);
                    tft.setTextColor(ft && ft->enabled ? ST77XX_GREEN : ST77XX_RED);
                    tft.print(ft && ft->enabled ? "[V]" : "[X]");
                }
            }
            break;
        }

        case SettingPage::SetHour: {


            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char hourStr[16];
            snprintf(hourStr, sizeof(hourStr), "Hour: %02d", _hour);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(hourStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }

        case SettingPage::SetMinute: {

            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char minStr[16];
            snprintf(minStr, sizeof(minStr), "Minute: %02d", _minute);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(minStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }

        case SettingPage::SetDuration: {


            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char durationStr[32];
            snprintf(durationStr, sizeof(durationStr), "Duration: %d ", _duration);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(durationStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }
        case SettingPage::SetEnabled: {
                tft.setTextSize(2);
                tft.setTextColor(ST77XX_WHITE);
                tft.setCursor(60, 20);
                tft.print("Enable this slot?");  // ✅ Tiêu đề

                const char* options[2] = { "YES", "NO" };
                for (int i = 0; i < 2; i++) {
                    tft.setCursor(60 + i * 160, 100);  // ✅ Giãn cách xa nhau
                    tft.setTextSize(3);
                    tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
                    tft.print(options[i]);
                }

                tft.setTextSize(2);
                tft.setCursor(20, 130);
                tft.setTextColor(ST77XX_WHITE);
                tft.print("Rotate to adjust,");

                tft.setTextSize(2);
                tft.setCursor(20, 150);
                tft.setTextColor(ST77XX_WHITE);
                tft.print("Click to confirm");
                break;
            }

        case SettingPage::ConfirmSave: {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(60, 20);
            tft.print("Save this setting?");

            const char* options[2] = { "YES", "NO" };
            for (int i = 0; i < 2; i++) {
                tft.setCursor(60 + i * 160, 100);
                tft.setTextSize(3);
                tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
                tft.print(options[i]);
            }
            break;
        }

        default:
            break;
    }
}
