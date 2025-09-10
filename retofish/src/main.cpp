#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

// HAL layer
#include "hal/RTC.h"
#include "hal/StatusLed.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"

// Service layer
#include "services/FeedingService.h"
#include "services/ScheduleManager.h"
#include "services/UIService.h"

#include "hal/config.h"

// -------------------------
// Singletons (references) — private to this TU
// -------------------------
namespace {
  auto& rtc       = RTC::getInstance();
  auto& statusLed = StatusLed::getInstance();
  auto& button    = Button::getInstance();
  auto& tft       = TftDisplay::getInstance();
  auto& stepper   = StepperMotor::getInstance();
  auto& ui        = UIService::getInstance();
  auto& feeding   = FeedingService::getInstance();
  auto& schedule  = ScheduleManager::getInstance();
}

// -------------------------
// Helpers
// -------------------------
static void initHardware() {
  Serial.begin(9600);
  delay(200);

  // Start I2C trước mọi thiết bị dùng Wire
  Wire.begin(Pins::SDA, Pins::SCL);

  rtc.setup();
  statusLed.setup(Pins::LED_R, Pins::LED_G, Pins::LED_B);
  button.setup(Pins::BTN_SW, Pins::ENC_A, Pins::ENC_B);
  tft.setup(Pins::TFT_CS, Pins::TFT_DC, Pins::TFT_RST, Pins::TFT_BLK);
  stepper.setup();

  Serial.println(F("[BOOT] Hardware initialized"));
}

static void initServices() {
  ui.setup();
  feeding.setup();
  schedule.setup();
  Serial.println(F("[BOOT] Services initialized"));
}

// -------------------------
// Arduino entry points
// -------------------------
void setup() {
  initHardware();
  initServices();
}

void loop() {
  button.update();
  const Button::Event evt = button.getEvent();

  if (evt != Button::Event::None) {
    button.handleEvent(evt);
    ui.updateScreen(evt);
  }

  const uint32_t now = millis();
  static uint32_t lastUiTick = 0;
  if (now - lastUiTick >= Cadence::UI_MS) {
    ui.updateScreen(Button::Event::None);
    lastUiTick = now;
  }

  static uint32_t lastAutoFeedTick = 0;
  if (now - lastAutoFeedTick >= Cadence::AUTO_FEED_MS) {
    feeding.handleAutoFeeding();
    lastAutoFeedTick = now;
  }

  static uint32_t lastHousekeep = 0;
  if (now - lastHousekeep >= Cadence::HOUSEKEEP_MS) {
    // ví dụ: statusLed.heartbeat();
    lastHousekeep = now;
  }

  // Nhường CPU (cooperative)
  yield(); // hoặc delay(1);
}
