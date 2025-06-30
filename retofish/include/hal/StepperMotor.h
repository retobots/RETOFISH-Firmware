// Điều khiển động cơ bước 28BYJ-48 nhả thức ăn. Hỗ trợ quay theo từng bước (non-blocking).
#pragma once

class StepperMotor {
public:
  static StepperMotor& getInstance();

  void setup();
  void feedOnce();  // ✅ Bổ sung khai báo hàm


private:
  StepperMotor() = default;
};
