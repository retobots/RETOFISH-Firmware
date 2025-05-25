#include "hal/StatusLed.h"

StatusLed& StatusLed::getInstance() {
  static StatusLed instance;
  return instance;
}

void StatusLed::setup() {
  // TODO: viết hàm khởi tạo
}
