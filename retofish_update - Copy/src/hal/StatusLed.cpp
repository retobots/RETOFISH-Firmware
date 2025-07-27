#include "hal/StatusLed.h"

#define COMMON_ANODE true   // Nếu LED là common anode → chỉnh true

void StatusLed::setup(uint8_t redPin, uint8_t greenPin, uint8_t bluePin) {
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;

    // Setup PWM cho ESP32 (1 lần)
    ledcSetup(1, 5000, 8);
    ledcSetup(2, 5000, 8);
    ledcSetup(3, 5000, 8);

    ledcAttachPin(_redPin, 1);
    ledcAttachPin(_greenPin, 2);
    ledcAttachPin(_bluePin, 3);

    setStatus(State::Idle);
}

void StatusLed::setStatus(State newState) {
    if (_state != newState) {
        _state = newState;
        _lastToggleTime = millis();
        _ledOn = false;
        updateLed(true);
    }
}

void StatusLed::update() {
    updateLed(false);
}


void StatusLed::updateLed(bool forceUpdate) {
    unsigned long now = millis();

    if (_state == State::Idle) {
        setColor(0, 0, 0);
        return;
    }

    if (forceUpdate || (now - _lastToggleTime >= 250)) {   // thay đổi tốc độ chớp 
        _ledOn = !_ledOn;
        _lastToggleTime = now;
    }

    switch (_state) {
        case State::Feeding:
            setColor(0, 255, 0); // Xanh lá sáng liên tục
            break;

        case State::LowBattery:
            setColor(_ledOn ? 255 : 255, 0, 0); // Đỏ chớp
            break;

        default:
            setColor(0, 0 , 0); // tắt đèn
            break;
    }
}


void StatusLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    // Dùng PWM ledcWrite cho ESP32
    ledcWrite(1, COMMON_ANODE ? 255 - r : r);
    ledcWrite(2, COMMON_ANODE ? 255 - g : g);
    ledcWrite(3, COMMON_ANODE ? 255 - b : b);
}

