#include "services/FeedingService.h"
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"
#include "hal/StatusLed.h"
#include "services/ScheduleManager.h"

FeedingService &FeedingService::getInstance()
{
    static FeedingService instance;
    return instance;
}

void FeedingService::setup()
{
    _screenOn = true;
    _screenOnTime = millis();
    _lastManualFeedTime = millis() - 30000;
}

void FeedingService::loop()
{
    unsigned long now = millis();
    DateTime nowRtc = RTC::getInstance().now();

    handleButton();
    handleAutoFeeding();

    // Tự động tắt màn sau 15 giây
    if (_screenOn && (now - _screenOnTime > 15000)) {
        _screenOn = false;
        Serial.println("[Auto] Screen OFF → đã tắt màn hình sau 15s");
        TftDisplay::getInstance().turnOff();
    }

    // Kết thúc feeding sau 5 giây
    if (_feeding && now - _feedingStartTime > 5000) {
        _feeding = false;
        Serial.println("Feeding STOP");
    }

    // Tắt cảnh báo spam sau 5 giây
    if (_warnSpam && now - _warnStartTime > 5000) {
        _warnSpam = false;
        Serial.println("Warning END");
    }

    updateDisplayAndLed();
}

void FeedingService::handleButton()
{
    Button::getInstance().update();
    auto evt = Button::getInstance().getEvent();
    unsigned long now = millis();

    if (evt == Button::Event::Click || evt == Button::Event::HoldShort)
    {
        if (!_screenOn)
        {
            _screenOn = true;
            _screenOnTime = now;
            _warnSpam = false;

            Serial.println("[Button] Screen ON → bật màn hình");
            updateDisplayAndLed();
            Serial.println("[Display] Đã cập nhật nội dung TFT");
        }
        else
        {
            if (now - _lastManualFeedTime > 30000)
            {
                _feeding = true;
                _feedingStartTime = now;
                _lastManualFeedTime = now;
                _warnSpam = false;

                Serial.println("[Button] Feeding START → cho ăn thủ công");
                StepperMotor::getInstance().feedOnce();
            }
            else
            {
                _warnSpam = true;
                _warnStartTime = now;

                Serial.println("[Button] Feed ignored → PLEASE WAIT");
            }
        }
    }
}

void FeedingService::handleAutoFeeding()
{
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();

    if (ScheduleManager::getInstance().isTimeToFeed(nowRtc) &&
        now - _lastAutoFeedTime > 60000)
    {
        StepperMotor::getInstance().feedOnce();
        _feeding = true;
        _feedingStartTime = now;
        _lastAutoFeedTime = now;
        Serial.println("[Auto] Feeding START → cho ăn theo lịch");
    }
}

void FeedingService::updateDisplayAndLed()
{
    float v = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();

    auto &led = StatusLed::getInstance();
    const char *statusStr;

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

    if (_screenOn)
    {
        DateTime nowRtc = RTC::getInstance().now();
        const FeedTime *nextFeed = ScheduleManager::getInstance().getNextFeedTime(nowRtc);

        int hour12 = nextFeed->hour % 12;
        if (hour12 == 0) hour12 = 12;
        const char *ampm = nextFeed->hour < 12 ? "AM" : "PM";

        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, nextFeed->minute, ampm);

        TftDisplay::getInstance().showFullStatus(v, level, statusStr, timeStr);
    }
}
