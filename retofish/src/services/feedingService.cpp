
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
    handleButton();
    handleAutoFeeding();
    checkScreenTimeout();
    checkFeedingTimeout();
    checkWarningTimeout();
    updateDisplayAndLed();
    delay(20);
}

void FeedingService::handleButton() {
    Button::getInstance().update();
    auto evt = Button::getInstance().getEvent();
    unsigned long now = millis();

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
                _feedingStartTime = now;
                _lastManualFeedTime = now;
                _warnSpam = false;
                Serial.println("Feeding START");
                updateDisplayAndLed();
                StepperMotor::getInstance().feedOnce();

            } else {
                _warnSpam = true;
                _warnStartTime = now;
                Serial.println("Feed ignored → PLEASE WAIT");
                updateDisplayAndLed();
            }
        }
    }
}


void FeedingService::handleAutoFeeding() {
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();
    Serial.printf("RTC Time: %02d:%02d:%02d\n", nowRtc.hour(), nowRtc.minute(), nowRtc.second());

    if (ScheduleManager::getInstance().isTimeToFeed(nowRtc)) {

        _feeding = true;
        _feedingStartTime = now;
        _lastAutoFeedTime = now;

        Serial.println("[Auto] Feeding START → cho ăn theo lịch");

        // ✅ Bật màn hình nếu đang tắt
        _screenOn = true;
        _screenOnTime = now;

        updateDisplayAndLed();  // ✅ cập nhật màn hình và LED
        StepperMotor::getInstance().feedOnce();
    }
}

void FeedingService::updateDisplayAndLed() {
    float v = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();

    auto& led = StatusLed::getInstance();
    const char* statusStr;

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
        DateTime nowRtc = RTC::getInstance().now();
        const FeedTime* nextFeed = ScheduleManager::getInstance().getNextFeedTime(nowRtc);

        int hour12 = nextFeed->hour % 12;
        if (hour12 == 0) hour12 = 12;
        const char* ampm = nextFeed->hour < 12 ? "AM" : "PM";

        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, nextFeed->minute, ampm);

        TftDisplay::getInstance().showFullStatus(v, level, statusStr, timeStr);
    }
}

void FeedingService::checkScreenTimeout() {
    if (_screenOn && (millis() - _screenOnTime > 15000)) {
        _screenOn = false;
        Serial.println("Screen OFF");
        TftDisplay::getInstance().turnOff();
    }
}

void FeedingService::checkFeedingTimeout() {
    if (_feeding && (millis() - _feedingStartTime > 5000)) {
        _feeding = false;
        Serial.println("Feeding STOP");
    }
}

void FeedingService::checkWarningTimeout() {
    if (_warnSpam && (millis() - _warnStartTime > 5000)) {
        _warnSpam = false;
        Serial.println("Warning END");
    }
}
