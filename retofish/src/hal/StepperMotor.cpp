#include "hal/StepperMotor.h"

StepperMotor& StepperMotor::getInstance() {
  static StepperMotor instance;
  return instance;
}

void StepperMotor::setup() {
  // TODO: viết hàm khởi tạo
}

