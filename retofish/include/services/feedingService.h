#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "services/UIService.h"
#include "hal/RTC.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"
#include "hal/StatusLed.h"
#include "services/ScheduleManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

class FeedingService {
public:
    static FeedingService& getInstance();

    void setup();
    bool isFeeding();
    void setFeeding(bool f);
    void feeding(float level, bool disableAfterFeeding);
    void handleAutoFeeding();         // Kiểm tra và cho ăn tự động khi đến giờ

private:
    FeedingService() = default;
    
    bool _feeding = false;

    unsigned long _lastAutoFeedTime = 0;

};
