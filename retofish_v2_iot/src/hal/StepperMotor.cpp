
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
    _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ 
}

// unsigned long StepperMotor::getFeedDuration() {
//     return stepsPerRevolution * 3 * 2;  // giống như trước: 3 vòng * 2ms
// }

