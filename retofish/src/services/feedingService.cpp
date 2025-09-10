#include <Arduino.h>
#include "services/FeedingService.h"
#include "services/UIService.h"
#include "services/ScheduleManager.h"
#include "hal/TftDisplay.h"
#include "hal/RTC.h"
#include "hal/StepperMotor.h"
#include "hal/config.h"

FeedingService& FeedingService::getInstance() {
  static FeedingService instance;
  return instance;
}

void FeedingService::setup() {
  _feeding = false;
  _lastAutoFeedTime = 0;
  UIService::getInstance().updateHomePage(); // vẽ trạng thái ban đầu
}

void FeedingService::handleAutoFeeding() {
  const unsigned long nowMs = millis();

  // Cooldown: không xét auto-feed quá dày
  if (nowMs - _lastAutoFeedTime < FeedCfg::AUTOTRIGGER_COOLDOWN_MS) {
    return;
  }

  // Không tự cho ăn khi đang cho ăn thủ công / UI không ở Home
  if (_feeding || UIService::getInstance().getScreen() != UIService::Screen::Home) {
    return;
  }

  // Lấy thời gian hiện tại từ RTC
  const DateTime nowRtc = RTC::getInstance().now();

  // Tìm slot phù hợp
  const FeedTime* matchedSlot = nullptr;
  int matchedIndex = -1;

  for (int i = 0; i < 3; ++i) {
    const FeedTime* s = ScheduleManager::getInstance().getSlot(i);
    if (!s)            { continue; }           // không có dữ liệu slot -> bỏ qua
    if (!s->enabled)   { continue; }           // chỉ xét slot đang bật

    const bool hitHour   = (nowRtc.hour()   == s->hour);
    const bool hitMinute = (nowRtc.minute() == s->minute);
    const bool hitSecond = (nowRtc.second() <  FeedCfg::TRIGGER_WINDOW_SECONDS);

    if (hitHour && hitMinute && hitSecond) {
      matchedSlot = s;
      matchedIndex = i;
      break;
    }
  }

  if (!matchedSlot) {
    return; // không có slot nào khớp
  }

  // Chuẩn bị hiển thị
  auto& tft = TftDisplay::getInstance();
  auto& ui  = UIService::getInstance();

  tft.turnOnScreen();
  tft.clear();
  tft.resetLastStatus();
  ui.setScreenOnTime(nowMs);

  // Đánh dấu đang feeding
  _feeding = true;

  // Log & cập nhật UI trước khi chạy
  Serial.printf("[Auto] Feeding START (slot %d | %02d:%02d | %.2f)\n",
                matchedIndex + 1, matchedSlot->hour, matchedSlot->minute,
                static_cast<float>(matchedSlot->duration));
  ui.updateHomePage();

  // Thực thi cho ăn
  feeding(static_cast<float>(matchedSlot->duration), /*disableAfterFeeding=*/true);

  // Kết thúc feeding
  _feeding = false;
  _lastAutoFeedTime = millis(); // ghi dấu thời gian để chống kích hoạt lại
  ui.updateHomePage();
  ui.setScreenOnTime(millis()); // ở lại chế độ "chờ" một lúc cho người dùng quan sát

  Serial.println("[Auto] Feeding DONE");
}

bool FeedingService::isFeeding() {
  return _feeding;
}

void FeedingService::setFeeding(bool f) {
  _feeding = f;
}

void FeedingService::feeding(float level, bool disableAfterFeeding) {
  // level: số "vòng" hay "mức" – tùy StepperMotor::feeding() định nghĩa
  StepperMotor::getInstance().feeding(level);
  if (disableAfterFeeding) {
    StepperMotor::getInstance().disableMotor();
  }
}
