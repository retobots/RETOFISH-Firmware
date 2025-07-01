#pragma once
#include <Arduino.h>

class StepperMotor {
public:
    static StepperMotor& getInstance();

    void setup();
    void feedOnce();  // Cho ăn = xoay 3 vòng
    unsigned long getFeedDuration(); 

private:
    StepperMotor() = default;

    void stepMotor(int step);

    static const int stepsPerRevolution = 2048;  // 1 vòng = 2048 bước
    int _currentStep = 0;

    // Cần khai báo chân GPIO của động cơ
    const int _pin1 = 33;
    const int _pin2 = 22;
    const int _pin3 = 12;
    const int _pin4 = 13;
};
