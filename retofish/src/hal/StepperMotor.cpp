

#include "hal/StepperMotor.h"
#include "hal/StepperMotor.h"
#include "hal/config.h"


StepperMotor& StepperMotor::getInstance() {
    static StepperMotor instance;
    return instance;
}

void StepperMotor::setup() {
    _stepper.setSpeed(StepperCfg::MOTOR_SPEED);  // tốc độ quay: 5 vòng/phút

    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

void StepperMotor::feeding(float level) {
    int totalSteps = (int)(stepsPerRevolution * level * StepperCfg::FEEDING_SCALE);  
    _stepper.step(-totalSteps);  // cùng chiều kim đồng hồ
}


void StepperMotor::disableMotor() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

void StepperMotor::setRpm(uint8_t rpm) {
    // Giới hạn an toàn cho 28BYJ-48 (bạn chỉnh tùy động cơ/driver)
    if (rpm < StepperCfg::MOTOR_MIN_SPEED)  rpm = StepperCfg::MOTOR_MIN_SPEED;
    if (rpm > StepperCfg::MOTOR_MAX_SPEED) rpm = StepperCfg::MOTOR_MAX_SPEED;
    _stepper.setSpeed(rpm);
}
