

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

    // Khởi tạo trạng thái ban đầu là Idle
    setStatus(State::Idle);
}

void StatusLed::setStatus(State newState) {
    if (_state != newState) {
        _state = newState;            // Cập nhật trạng thái mới
        _lastToggleTime = millis();   // Ghi lại thời gian chuyển trạng thái
        _ledOn = false;               // Đặt lại trạng thái nhấp nháy LED
        updateLed(true);              // Cập nhật LED ngay lập tức
    }
}

void StatusLed::update() {
    updateLed(false); // Cập nhật LED mà không ép buộc ngay lập tức
}

void StatusLed::updateLed(bool forceUpdate) {
    unsigned long now = millis();  // Lấy thời gian hiện tại

    if (_state == State::Idle) {
        setColor(255, 255, 255);  // Tắt LED khi ở trạng thái Idle
        return;
    }

    // Cập nhật LED mỗi 250ms hoặc nếu forceUpdate là true
    if (forceUpdate || (now - _lastToggleTime >= 250)) {   
        _ledOn = !_ledOn;  // Chuyển đổi trạng thái bật/tắt của LED
        _lastToggleTime = now;  // Cập nhật thời gian khi thay đổi trạng thái
    }

    // Xử lý các trạng thái khác nhau
    switch (_state) {
        case State::Feeding:
            setColor(255, 0, 255); // LED Xanh lá sáng liên tục khi Feeding
            break;

        case State::LowBattery:
            // LED đỏ nhấp nháy khi LowBattery
            setColor(_ledOn ? 255 : 0, 255, 255); 
           
            break;

        case State::Idle:
            // setColor(255 , 255 , 255); // Tắt LED (Trắng) cho các trạng thái khác
                ledcWrite(1, 1);   // Tắt LED đỏ
                ledcWrite(2, 1);   // Tắt LED xanh lá
                ledcWrite(3, 1);   // Tắt LED xanh dương
                
            break;
    }
}

void StatusLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    // Điều khiển PWM cho ESP32 bằng ledcWrite
    ledcWrite(1, COMMON_ANODE ? 255 - r : r);  // Điều khiển LED đỏ
    ledcWrite(2, COMMON_ANODE ? 255 - g : g);  // Điều khiển LED xanh lá
    ledcWrite(3, COMMON_ANODE ? 255 - b : b);  // Điều khiển LED xanh dương
}
