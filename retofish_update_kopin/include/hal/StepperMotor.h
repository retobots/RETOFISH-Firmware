
// #pragma once
// #include <Arduino.h>
// #include <Stepper.h>

// class StepperMotor {
// public:
//     static StepperMotor& getInstance();

//     void setup();
//     unsigned long getFeedDuration();
//     void feedForRounds(int numRounds);

// private:
//     StepperMotor() = default;

//     static const int stepsPerRevolution = 2048;
//     // Stepper _stepper = Stepper(stepsPerRevolution, 33, 12, 22, 13);  // IN1, IN3, IN2, IN4
//     Stepper _stepper = Stepper(stepsPerRevolution, 26, 25, 33, 32);  // IN1, IN3, IN2, IN4

// };

#pragma once
#include <Arduino.h>
#include <Stepper.h>

class StepperMotor {
public:
    static StepperMotor& getInstance();

    void setup();
    unsigned long getFeedDuration();
    void feedForRounds(float numRounds);
    void feedForRounds1(float numRounds);

    bool isMotorActive(); 
    void powerOff(); 
    void setRpm(float rpm);   // đặt tốc độ quay (RPM)



private:
    StepperMotor() = default;

    static const int stepsPerRevolution = 2048;
    Stepper _stepper = Stepper(stepsPerRevolution, 26, 25, 33, 32);  // IN1, IN3, IN2, IN4

    const int in1 = 26;
    const int in2 = 33;
    const int in3 = 25;
    const int in4 = 32;

    void disableMotor();  // Tắt điện các cuộn dây
    bool _isMotorActive = false; 
};
