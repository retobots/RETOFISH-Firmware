


#include "hal/Button.h"

Button& Button::getInstance() {
    static Button instance;
    return instance;
}

// Bảng mã Gray decoding 4-bit cho encoder
const int8_t encoder_table[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

volatile int Button::_rotationDelta = 0;
volatile int Button::_rotationTotal = 0;
volatile uint8_t Button::_lastAB = 0;

void Button::setup(uint8_t swPin, uint8_t pinA, uint8_t pinB) {
    _pinSW = swPin;
    _pinA = pinA;
    _pinB = pinB;

    pinMode(_pinSW, INPUT_PULLUP);
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);

    uint8_t a = digitalRead(_pinA);
    uint8_t b = digitalRead(_pinB);
    _lastAB = (a << 1) | b;

    attachInterrupt(digitalPinToInterrupt(_pinA), handleEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pinB), handleEncoderISR, CHANGE);
}

void IRAM_ATTR Button::handleEncoderISR() {
    uint8_t a = digitalRead(17);  // A = D17
    uint8_t b = digitalRead(16);  // B = D16
    uint8_t ab = (a << 1) | b;

    uint8_t index = (_lastAB << 2) | ab;
    int8_t direction = encoder_table[index & 0x0F];

    if (direction != 0) {
        _rotationDelta += direction;
        _rotationTotal += direction;
        
    }

    _lastAB = ab;
}

void Button::update() {
    bool currentState = digitalRead(_pinSW);
    unsigned long now = millis();

    if (_lastState != currentState) {
        _lastDebounceTime = now;
        _lastState = currentState;
    }

    if ((now - _lastDebounceTime) > 50) {
        if (_lastState == LOW && !_isPressed) {
            _pressedTime = now;
            _isPressed = true;
        }

        if (_lastState == LOW && _isPressed) {
        unsigned long duration = now - _pressedTime;
        if (duration >= 3000 && _lastEvent != Event::HoldLong) {
            _lastEvent = Event::HoldLong; 
        }
    }

        if (_lastState == HIGH && _isPressed) {
            unsigned long duration = now - _pressedTime;

            if (duration < 300) {
                if (_waitingDoubleClick && (now - _doubleClickTimer) < 500) {
                    _lastEvent = Event::DoubleClick;
                    _waitingDoubleClick = false;
                } else {
                    _waitingDoubleClick = true;
                    _doubleClickTimer = now;
                }
            } else if (duration < 1000) {
                _lastEvent = Event::HoldShort;
            } else {
                _lastEvent = Event::HoldLong;
            }

            _isPressed = false;
        }
    }

    if (_waitingDoubleClick && (now - _doubleClickTimer) > 500) {
        _waitingDoubleClick = false;
        _lastEvent = Event::Click;
    }
}

Button::Event Button::getEvent() {
    Event e = _lastEvent;
    _lastEvent = Event::None;
    return e;
}

int Button::getRotationDelta() {

    static int remainder = 0;

    noInterrupts();
    int raw = _rotationDelta;
    _rotationDelta = 0;
    interrupts();

    raw += remainder;
    int fullSteps = raw / 4;
    remainder = raw % 4;

    return fullSteps;
}

int Button::getRotationTotal() {
    noInterrupts();
    int total = _rotationTotal;
    interrupts();
    return total;
}

void Button::resetRotationTotal() {
    noInterrupts();
    _rotationTotal = 0;
    interrupts();
}
