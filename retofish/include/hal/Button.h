

#pragma once
#include <Arduino.h>

class Button
{
public:
    static Button &getInstance();
    enum class Event
    {
        None,
        Click,
        HoldSetting,
        HoldFeeding,
        DoubleClick
    };
    void setup(uint8_t swPin = 19, uint8_t pinA = 34, uint8_t pinB = 35);
    void update(); // Xử lý nút nhấn
    Button::Event getEvent();
    void setEvent(Button::Event e);

    int getRotationDelta(); // ⏩ trả về +1/-1 mỗi lần xoay
    unsigned long getRawPressedDuration();
    void handleEvent(Button::Event evt);

private:
    Button() = default;
    static void IRAM_ATTR handleEncoderISR(); // ISR cho encoder

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
    Button::Event _lastEvent = Button::Event::None;
    static volatile uint8_t _lastAB;
    bool _holdFeeding = false; // đang “giữ để cho ăn”
};
