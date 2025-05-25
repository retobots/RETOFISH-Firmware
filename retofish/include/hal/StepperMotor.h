#pragma once

class StepperMotor {
public:
  static StepperMotor& getInstance();

  void setup();

private:
  StepperMotor() = default;
};
