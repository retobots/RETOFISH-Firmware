

#include "hal/StatusLed.h"

#define COMMON_ANODE true // Nếu LED là common anode → chỉnh true

void StatusLed::setup(uint8_t redPin, uint8_t greenPin, uint8_t bluePin)
{
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;

    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);

    // Khởi tạo trạng thái ban đầu là Idle
    updateLed(State::Idle);
}
void StatusLed::updateLed(State s)
{
    // Xử lý các trạng thái khác nhau
    switch (s)
    {
    case State::Feeding:
        setColor(0, 1, 0); // LED Xanh lá sáng liên tục khi Feeding
        break;

    case State::Idle:
        setColor(0, 0, 0);

    default:
        break;
    }
}

void StatusLed::setColor(bool r, bool g, bool b)
{
    digitalWrite(_redPin, r);
    digitalWrite(_greenPin, g);
    digitalWrite(_bluePin, b);
}
