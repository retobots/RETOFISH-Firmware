#include "hal/BlynkComm.h"

BlynkComm& BlynkComm::getInstance() {
  static BlynkComm instance;
  return instance;
}

void BlynkComm::setup() {
  // TODO: viết hàm khởi tạo
}

void BlynkComm::loop() {
  // TODO: viết hàm xử lý định kỳ
}
