
#include "services/FeedingService.h"
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"
#include "hal/StatusLed.h"
#include "services/ScheduleManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

FeedingService &FeedingService::getInstance()
{
    static FeedingService instance;
    return instance;
}

void FeedingService::setup()
{

    _lastAutoFeedTime = 0;
    Battery::getInstance().update(true);       // ✅ cập nhật pin ngay khi khởi động
    UIService::getInstance().updateHomePage(); // ✅ vẽ trạng thái pin lên màn hình
}

void FeedingService::loop()
{
    Button &btn = Button::getInstance();
    btn.update();
    Button::Event evt = btn.getEvent();
    // Serial.print("Event: ");
    // Serial.println((int)evt);
    // Serial.print("Screen: ");
    // Serial.println((int)UIService::getInstance().getScreen());
    btn.handleEvent(evt);
    handleAutoFeeding();
    UIService::getInstance().updateScreen(evt);
}

void FeedingService::handleAutoFeeding()
{
    if (millis() - _lastAutoFeedTime < 5000)
    {
        return;
    }
    else
    {
        _lastAutoFeedTime = 0;
    }
    if (_feeding || UIService::getInstance().getScreen() != UIService::Screen::Home)
    {
        return;
    }
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();

    const FeedTime *matchedSlot = nullptr;
    for (int i = 0; i < 3; ++i)
    {
        const FeedTime *s = ScheduleManager::getInstance().getSlot(i);
        if (!s)
            return;
        if (!s->enabled)
            return; // chỉ slot ENABLED
        if (nowRtc.hour() == s->hour &&
            nowRtc.minute() == s->minute &&
            nowRtc.second() < 2)
        {

            matchedSlot = s;
            Serial.printf("[Auto] Feeding START (slot %d | %02d:%02d | %ds)\n",
                          i + 1, matchedSlot->hour, matchedSlot->minute, (float)matchedSlot->duration);
            // 🖥️ Bật màn hình + chuẩn bị UI
            TftDisplay::getInstance().turnOnScreen();
            _feeding = true;
            UIService::getInstance().setScreenOnTime(now);
            TftDisplay &display = TftDisplay::getInstance();
            display.clear();
            display.resetLastStatus();

            // Cập nhật trạng thái hiển thị/LED (ví dụ LED Feeding)
            UIService::getInstance().updateHomePage(); // Cập nhật trạng thái sau khi cho ăn xong
            // 🚚 Thực thi cho ăn
            feeding((float)matchedSlot->duration, true);
            _feeding = false;
            _lastAutoFeedTime = millis();
            UIService::getInstance().updateHomePage(); // Cập nhật trạng thái sau khi cho ăn xong

            // Sau khi cho ăn xong, chuyển vào chế độ "chờ"
            UIService::getInstance().setScreenOnTime(millis()); // Ghi lại thời gian vào chế độ "chờ"

            // 🕒 Ghi dấu thời gian để chống kích hoạt lại
            Serial.println("Screen ON by Auto");
            break;
        }
    }
}

bool FeedingService::isFeeding()
{
    return _feeding;
}

void FeedingService::setFeeding(bool f)
{
    _feeding = f;
}

void FeedingService::feeding(float level, bool disableAfterFeeding)
{
    StepperMotor::getInstance().feedingLevel(level);
    if (disableAfterFeeding)
    {
        StepperMotor::getInstance().disableMotor();
    }
}
