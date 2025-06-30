#include <Arduino.h>
#include <Wire.h>   // cần thêm cái này để quét I2C
#include <RTClib.h>

// HAL layer
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/StatusLed.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/BlynkComm.h"
#include "hal/StepperMotor.h"

// Service layer
#include "services/FeedingService.h"
#include "services/ScheduleManager.h"
#include "services/StatusReporter.h"

static bool screenOn = false;
static unsigned long screenOnTime = 0;

static bool feeding = false;
static unsigned long feedingStartTime = 0;

static bool warnSpam = false;
static unsigned long warnStartTime = 0;

static unsigned long lastManualFeedTime = 0;


void setup() {
  Serial.begin(9600);
  delay(500);
  

  // --- Hardware initialization ---
  Battery::getInstance().setup();
  RTC::getInstance().setup();
  StatusLed::getInstance().setup(25, 26, 27);
  Button::getInstance().setup(19);
  TftDisplay::getInstance().setup(5, 2, 4);
    screenOn = true;
    screenOnTime = millis();

  StepperMotor::getInstance().setup();
  BlynkComm::getInstance().setup();

  // --- Service layer initialization ---
  FeedingService::getInstance().setup();
  ScheduleManager::getInstance().setup();
  StatusReporter::getInstance().setup();

  lastManualFeedTime = millis() - 30000;
 



}


void loop() {
    unsigned long now = millis();

    // Update button
    Button::getInstance().update();
    auto evt = Button::getInstance().getEvent();

    // Xử lý nhấn nút
    if (evt == Button::Event::Click || evt == Button::Event::HoldShort) {
        if (!screenOn) {
            // Lần Click đầu tiên → bật màn
            screenOn = true;
            screenOnTime = now;
            warnSpam = false;
            Serial.println("Screen ON");
        } else {
            // Màn đang sáng → xử lý cho ăn hoặc cảnh báo
            if (now - lastManualFeedTime > 30000) {  // >30s mới cho ăn
                feeding = true;
                feedingStartTime = now;
                lastManualFeedTime = now;
                warnSpam = false;
                Serial.println("Feeding START");
                // StepperMotor::getInstance().feedOnce();
            } else {
                // Spam → cảnh báo
                warnSpam = true;
                warnStartTime = now;
                Serial.println("Feed ignored → PLEASE WAIT");
            }
        }
    }

    // Tự tắt màn sau 15s
    if (screenOn && (now - screenOnTime > 15000)) {
        screenOn = false;
        Serial.println("Screen OFF");

        auto& tft = TftDisplay::getInstance();
        tft.turnOff();
    }

    // Feeding timeout sau 5s
    if (feeding && (now - feedingStartTime > 5000)) {
        feeding = false;
        Serial.println("Feeding STOP");
    }

    // Cảnh báo "Please wait..." timeout sau 5s
    if (warnSpam && (now - warnStartTime > 5000)) {
        warnSpam = false;
        Serial.println("Warning END");
    }

    // Đọc pin
    float v = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();
    Serial.printf("Battery Voltage: %.2f V %d %%\n", v, level);

    // Update LED
    auto& led = StatusLed::getInstance();
    const char* statusStr;

    if (warnSpam) {
        statusStr = "Wait 30s";
    } else if (feeding) {
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

    auto& tft = TftDisplay::getInstance();
    if (screenOn) {
    unsigned long now = millis();    
    DateTime nowRtc = RTC::getInstance().now();

    static unsigned long lastAutoFeedTime = 0;

    if (ScheduleManager::getInstance().isTimeToFeed(nowRtc) &&
    now - lastAutoFeedTime > 60000) {
    
    feeding = true;
    feedingStartTime = now;
    lastAutoFeedTime = now;
    
    Serial.println("Auto Feeding START");
    StepperMotor::getInstance().feedOnce();
}


    const FeedTime* nextFeed = ScheduleManager::getInstance().getNextFeedTime(nowRtc);

    int hour12 = nextFeed->hour % 12;
    if (hour12 == 0) hour12 = 12;
    const char* ampm = nextFeed->hour < 12 ? "AM" : "PM";

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, nextFeed->minute, ampm);

    tft.showFullStatus(v, level, statusStr, timeStr);
    }
    delay(20);
    
 

}

