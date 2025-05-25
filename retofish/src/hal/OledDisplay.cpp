#include "hal/OledDisplay.h"

OledDisplay& OledDisplay::getInstance() {
  static OledDisplay instance;
  return instance;
}

void OledDisplay::setup() {
  // TODO: viết hàm khởi tạo
}

