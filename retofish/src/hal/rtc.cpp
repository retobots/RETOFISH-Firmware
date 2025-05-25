#include "hal/RTC.h"

RTC& RTC::getInstance() {
  static RTC instance;
  return instance;
}

void RTC::setup() {
  // TODO: viết hàm khởi tạo
}

