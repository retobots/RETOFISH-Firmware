#include "hal/StepperMotor.h"

StepperMotor& StepperMotor::getInstance() {
  static StepperMotor instance;
  return instance;
}

void StepperMotor::setup() {
  // TODO: viết hàm khởi tạo
}

void StepperMotor::feedOnce() {
    // ✅ Định nghĩa hàm để tránh lỗi linker
    // (Để trống cũng được nếu chưa điều khiển động cơ)
}