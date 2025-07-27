

#pragma once
#include <Arduino.h>

class Button {
public:
    enum class Event {
        None,
        Click,
        HoldShort,
        HoldLong,
        DoubleClick
    };

    static Button& getInstance();

    void setup(uint8_t swPin = 19, uint8_t pinA = 34, uint8_t pinB = 35);
    void update();  // Xử lý nút nhấn
    Event getEvent();

    int getRotationDelta();       // ⏩ trả về +1/-1 mỗi lần xoay
    int getRotationTotal();       // tổng số đã xoay từ đầu
    void resetRotationTotal();    // đặt lại về 0

private:
    Button() = default;
    static void IRAM_ATTR handleEncoderISR();  // ISR cho encoder

    // Encoder
    volatile static int _rotationDelta;
    volatile static int _rotationTotal;
    uint8_t _pinA, _pinB;
    static uint8_t _lastStateA;

    // Button
    uint8_t _pinSW = 19;
    bool _lastState = HIGH;
    unsigned long _lastDebounceTime = 0;
    unsigned long _pressedTime = 0;
    bool _isPressed = false;
    bool _waitingDoubleClick = false;
    unsigned long _doubleClickTimer = 0;
    Event _lastEvent = Event::None;
    static volatile uint8_t _lastAB;
};


