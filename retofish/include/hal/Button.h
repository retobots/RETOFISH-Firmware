// #pragma once
// #include <Arduino.h>


// class Button {
// public:
//     enum class Event {
//         None,
//         Click,      // nhấn nhanh <300ms
//         HoldShort,  // nhấn giữ 500ms ~ 1s
//         HoldLong,    // nhấn giữ >2s
//         DoubleClick, // 2 lần nhấn nhanh 

//     };

//     static Button& getInstance();

//     void setup(uint8_t pin = 22);  // mặc định GPIO22
//     void update();

//     Event getEvent();   // trả về sự kiện mới nhất (xong sẽ clear)

// private:
//     Button() = default;

//     uint8_t _pin = 22;
//     bool _lastState = HIGH;
//     unsigned long _lastDebounceTime = 0;
//     unsigned long _pressedTime = 0;
//     bool _isPressed = false;
//     bool _waitingDoubleClick;
//     Event _lastEvent = Event::None;
// };
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
