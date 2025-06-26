#include "hal/Button.h"

Button& Button::getInstance() {
    static Button instance;
    return instance;
}

void Button::setup(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT);  // vì đã có R14 kéo lên ngoài
}

void Button::update() {
    bool currentState = digitalRead(_pin);

    // Debounce 20ms
    if (currentState != _lastState) {
        _lastDebounceTime = millis();
        _lastState = currentState;
    }

    if ((millis() - _lastDebounceTime) > 20) {
        if (currentState == LOW && !_isPressed) {
            _isPressed = true;
            _pressedTime = millis();
            _lastEvent = Event::None;
        }
        else if (currentState == HIGH && _isPressed) {
            unsigned long pressDuration = millis() - _pressedTime;

            if (pressDuration < 300) {
                _lastEvent = Event::Click;
            } else if (pressDuration < 1000) {
                _lastEvent = Event::HoldShort;
            } else {
                _lastEvent = Event::HoldLong;
            }

            _isPressed = false;
        }
    }
}

Button::Event Button::getEvent() {
    Event e = _lastEvent;
    _lastEvent = Event::None;
    return e;
}
