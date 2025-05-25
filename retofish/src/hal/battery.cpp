#include "hal/Battery.h"

Battery& Battery::getInstance() {
  static Battery instance;
  return instance;
}

void Battery::setup() {
  // TODO: viết hàm khởi tạo
}
