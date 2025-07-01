
#pragma once
#include <Arduino.h>

class Button {
public:
    enum class Event {
        None,
        Click,
        HoldShort,
        HoldLong,
        DoubleClick,        
    };

    static Button& getInstance();

    void setup(uint8_t pin = 22);
    void update();
    Event getEvent();

private:
    Button() = default;

    uint8_t _pin = 22;
    bool _lastState = HIGH;
    unsigned long _lastDebounceTime = 0;
    unsigned long _pressedTime = 0;
    bool _isPressed = false;

    bool _waitingDoubleClick = false;
    unsigned long _doubleClickTimer = 0;

    Event _lastEvent = Event::None;
};
