
// #include "hal/StepperMotor.h"

// StepperMotor& StepperMotor::getInstance() {
//     static StepperMotor instance;
//     return instance;
// }

// void StepperMotor::setup() {
//     _stepper.setSpeed(10);  // tốc độ quay: 10 vòng/phút
// }

// void StepperMotor::feedForRounds(int numRounds) {
//     int totalSteps = stepsPerRevolution * numRounds * 2;
//     _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ 
// }

#include "hal/StepperMotor.h"

StepperMotor& StepperMotor::getInstance() {
    static StepperMotor instance;
    return instance;
}

void StepperMotor::setup() {
    _stepper.setSpeed(10);  // tốc độ quay: 5 vòng/phút

    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

void StepperMotor::feedForRounds(int numRounds) {
    int totalSteps = stepsPerRevolution * numRounds * 2;
    _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ
    disableMotor();  // tắt điện sau khi quay
}

void StepperMotor::disableMotor() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

