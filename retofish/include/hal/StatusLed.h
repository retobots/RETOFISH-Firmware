#pragma once
#include <Arduino.h>

class StatusLed {
public:
    enum class State {
        Idle,                       // trạng thái chờ
        Feeding,                    // Đang cho ăn
        LowBattery                  // Pin yếu 
    };

    static StatusLed& getInstance() {
        static StatusLed instance;
        return instance;
    }

    void setup(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);

    void setStatus(State newState);

    void update();

private:
    uint8_t _redPin = 0;
    uint8_t _greenPin = 0;
    uint8_t _bluePin = 0;

    State _state = State::Idle;
    unsigned long _lastToggleTime = 0;
    bool _ledOn = false;

    StatusLed() = default;

    void updateLed(bool forceUpdate);

    void setColor(uint8_t r, uint8_t g, uint8_t b);
};

