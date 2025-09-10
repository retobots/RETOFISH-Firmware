#pragma once
#include <Arduino.h>

class StatusLed
{
public:
    enum class State
    {
        Idle,    // trạng thái chờ
        Feeding, // Đang cho ăn
    };

    static StatusLed &getInstance()
    {
        static StatusLed instance;
        return instance;
    }

    void setup(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);

    void updateLed(State s);

private:
    uint8_t _redPin = 0;
    uint8_t _greenPin = 0;
    uint8_t _bluePin = 0;

    State _state = State::Idle;

    StatusLed() = default;

    void setColor(bool r, bool g, bool b);
};
