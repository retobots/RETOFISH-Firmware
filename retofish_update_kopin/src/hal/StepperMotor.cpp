

#include "hal/StepperMotor.h"
#include "hal/StepperMotor.h"
#define STEPPER_FEED_SCALE 0.30f  // Tỉ lệ quay cho động cơ


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



bool StepperMotor::isMotorActive() {
    return _isMotorActive;  // Trả về trạng thái hiện tại của động cơ
}



void StepperMotor::feedForRounds(float numRounds) {
    int totalSteps = (int)(stepsPerRevolution * numRounds * STEPPER_FEED_SCALE);  
    _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ
    disableMotor();  
    _isMotorActive = false; 
}
void StepperMotor::feedForRounds1(float numRounds) {
    int totalSteps = (int)(stepsPerRevolution * numRounds * STEPPER_FEED_SCALE);  
    _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ
    // disableMotor();  
    _isMotorActive = false; 
}


void StepperMotor::disableMotor() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}
void StepperMotor::powerOff() {
    disableMotor();
}

void StepperMotor::setRpm(float rpm) {
    // Giới hạn an toàn cho 28BYJ-48 (bạn chỉnh tùy động cơ/driver)
    if (rpm < 1.0f)  rpm = 1.0f;
    if (rpm > 20.0f) rpm = 20.0f;
    _stepper.setSpeed(rpm);
}
