#include "hal/StepperMotor.h"

StepperMotor& StepperMotor::getInstance() {
    static StepperMotor instance;
    return instance;
}

void StepperMotor::setup() {
    pinMode(_pin1, OUTPUT);
    pinMode(_pin2, OUTPUT);
    pinMode(_pin3, OUTPUT);
    pinMode(_pin4, OUTPUT);

    digitalWrite(_pin1, LOW);
    digitalWrite(_pin2, LOW);
    digitalWrite(_pin3, LOW);
    digitalWrite(_pin4, LOW);
}

void StepperMotor::feedOnce() {
    const int totalSteps = stepsPerRevolution * 3;  // 3 vòng
    for (int i = 0; i < totalSteps; ++i) {
        stepMotor(_currentStep % 8);
        _currentStep++;
        delay(2);  // tốc độ quay
    }

    // Tắt hết các chân sau khi quay xong
    digitalWrite(_pin1, LOW);
    digitalWrite(_pin2, LOW);
    digitalWrite(_pin3, LOW);
    digitalWrite(_pin4, LOW);
}

void StepperMotor::stepMotor(int step) {
    switch (step) {
        case 0:
            digitalWrite(_pin1, HIGH); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
        case 1:
            digitalWrite(_pin1, HIGH); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
        case 2:
            digitalWrite(_pin1, LOW); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
        case 3:
            digitalWrite(_pin1, LOW); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, LOW); break;
        case 4:
            digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, LOW); break;
        case 5:
            digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, HIGH); break;
        case 6:
            digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, HIGH); break;
        case 7:
            digitalWrite(_pin1, HIGH); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, HIGH); break;
    }
}
