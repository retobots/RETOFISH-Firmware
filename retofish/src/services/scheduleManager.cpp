#include "services/ScheduleManager.h"

ScheduleManager& ScheduleManager::getInstance() {
  static ScheduleManager instance;
  return instance;
}

void ScheduleManager::setup() {
  // TODO: viết hàm khởi tạo
}

void ScheduleManager::loop() {
  // TODO: viết hàm xử lý định kỳ
}
