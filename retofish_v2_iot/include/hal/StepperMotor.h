
#pragma once
#include <Arduino.h>
#include <Stepper.h>

class StepperMotor {
public:
    static StepperMotor& getInstance();

    void setup();
    unsigned long getFeedDuration();
    void feedForRounds(int numRounds);

private:
    StepperMotor() = default;

    static const int stepsPerRevolution = 2048;
    Stepper _stepper = Stepper(stepsPerRevolution, 33, 12, 22, 13);  // IN1, IN3, IN2, IN4
};
