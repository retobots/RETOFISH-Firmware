// Điều khiển động cơ bước 28BYJ-48 nhả thức ăn. Hỗ trợ quay theo từng bước (non-blocking).
#pragma once

class StepperMotor {
public:
  static StepperMotor& getInstance();

  void setup();

private:
  StepperMotor() = default;
};
