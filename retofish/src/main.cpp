#include <Arduino.h>

// HAL layer
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/StatusLed.h"
#include "hal/Button.h"
#include "hal/OledDisplay.h"
#include "hal/BlynkComm.h"
#include "hal/StepperMotor.h"

// Service layer
#include "services/FeedingService.h"
#include "services/ScheduleManager.h"
#include "services/StatusReporter.h"

void setup() {
  Serial.begin(9600);
  delay(500);

  // --- Hardware initialization ---
  Battery::getInstance().setup();
  RTC::getInstance().setup();
  StatusLed::getInstance().setup();
  Button::getInstance().setup();
  OledDisplay::getInstance().setup();
  StepperMotor::getInstance().setup();
  BlynkComm::getInstance().setup();

  // --- Service layer initialization ---
  FeedingService::getInstance().setup();
  ScheduleManager::getInstance().setup();
  StatusReporter::getInstance().setup();

  Serial.println("[INFO] RETOFISH Firmware ready.");
}

void loop() {
  // --- Core loop modules ---
  FeedingService::getInstance().loop();    // Kiểm tra trạng thái cần cho ăn từ nút nhấn, app, hoặc thời gian cho ăn hẹn trước. Điều khiển động cơ cho ăn khi có yêu cầu.
  ScheduleManager::getInstance().loop();   // Kiểm tra lịch trình, update lịch trình mới nếu có từ app
  StatusReporter::getInstance().loop();    // Kiểm tra trạng thái của robot và gửi dữ liệu lên cloud, cập nhật OLED, đèn RGB
}
