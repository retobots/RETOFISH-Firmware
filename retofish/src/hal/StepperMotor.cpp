// #include "hal/StepperMotor.h"

// StepperMotor& StepperMotor::getInstance() {
//     static StepperMotor instance;
//     return instance;
// }


// void StepperMotor::setup() {
//     pinMode(_pin1, OUTPUT);
//     pinMode(_pin2, OUTPUT);
//     pinMode(_pin3, OUTPUT);
//     pinMode(_pin4, OUTPUT);

//     digitalWrite(_pin1, LOW);
//     digitalWrite(_pin2, LOW);
//     digitalWrite(_pin3, LOW);
//     digitalWrite(_pin4, LOW);
// }

// void StepperMotor::feedForRounds(int numRounds) {
//     const int stepsPerRevolution = 2048;  // chuẩn với 28BYJ-48
//     int totalSteps = stepsPerRevolution * numRounds * 4 ;
//     // chìu thuận
//     for (int i = 0; i < totalSteps; ++i) {
//         stepMotor(_currentStep % 8);
//         _currentStep++;
//         delay(2);  // tốc độ quay
//     }



//     // Tắt motor
//     digitalWrite(_pin1, LOW);
//     digitalWrite(_pin2, LOW);
//     digitalWrite(_pin3, LOW);
//     digitalWrite(_pin4, LOW);
// }


// unsigned long StepperMotor::getFeedDuration() {
//     return stepsPerRevolution * 3 * 2;  // Mỗi bước delay(2ms)
//     // 2048 * 3 * 2 = 12288 ms
// }

// void StepperMotor::stepMotor(int step) {
//     switch (step) {
//         case 0:
//             digitalWrite(_pin1, HIGH); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
//         case 1:
//             digitalWrite(_pin1, HIGH); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
//         case 2:
//             digitalWrite(_pin1, LOW); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, LOW); digitalWrite(_pin4, LOW); break;
//         case 3:
//             digitalWrite(_pin1, LOW); digitalWrite(_pin2, HIGH); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, LOW); break;
//         case 4:
//             digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, LOW); break;
//         case 5:
//             digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, HIGH); digitalWrite(_pin4, HIGH); break;
//         case 6:
//             digitalWrite(_pin1, LOW); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, HIGH); break;
//         case 7:
//             digitalWrite(_pin1, HIGH); digitalWrite(_pin2, LOW); digitalWrite(_pin3, LOW); digitalWrite(_pin4, HIGH); break;
//     }
// }
#include "hal/StepperMotor.h"

StepperMotor& StepperMotor::getInstance() {
    static StepperMotor instance;
    return instance;
}

void StepperMotor::setup() {
    _stepper.setSpeed(10);  // tốc độ quay: 10 vòng/phút
}

void StepperMotor::feedForRounds(int numRounds) {
    int totalSteps = stepsPerRevolution * numRounds * 2;
    _stepper.step(-totalSteps);  // quay thuận
}

// unsigned long StepperMotor::getFeedDuration() {
//     return stepsPerRevolution * 3 * 2;  // giống như trước: 3 vòng * 2ms
// }
//hjjhlkkjl


