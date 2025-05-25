#include "hal/Button.h"

Button& Button::getInstance() {
  static Button instance;
  return instance;
}

void Button::setup() {
  // TODO: viết hàm khởi tạo
}
