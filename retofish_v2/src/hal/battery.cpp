
// #include "hal/Battery.h"

// Battery& Battery::getInstance() {
//     static Battery instance;
//     return instance;
// }

// void Battery::setup() {
//     // không cần làm gì khi giả lập
// }

// float Battery::readVoltage() {
//     // Giả lập: pin từ 100% (4.2V) xuống 0% (3.3V), rồi lên lại
//     static uint8_t level = 100;
//     static int8_t step = -1; // giảm

//     static unsigned long lastUpdate = 0;
//     unsigned long now = millis();

//     // mỗi 1 giây mới cập nhật level
//     if (now - lastUpdate > 1000) {
//         lastUpdate = now;

//         level += step;

//         if (level == 0) step = +1;
//         if (level == 100) step = -1;
//     }

//     float voltage = 3.3 + (level / 100.0f) * (4.2 - 3.3); // từ 3.3V đến 4.2V

//     return voltage;
// }

// uint8_t Battery::getBatteryLevel() {
//     float v = readVoltage();
//     if (v >= 4.2) return 100;
//     if (v <= 3.3) return 0;
//     return (uint8_t)(((v - 3.3) / (4.2 - 3.3)) * 100);
// }
////////////////////////////////////////////////////////////////////////////////////////////

#include "hal/Battery.h"

Battery& Battery::getInstance() {
    static Battery instance;
    return instance;
}

void Battery::setup() {
    pinMode(_adcPin, INPUT);
    pinMode(_enablePin, OUTPUT);
    pinMode(_chrgPin, INPUT);
    analogReadResolution(12);
    _lastUpdate = 0;
    void update(bool force = false);  // Thêm tham số force

}




// void Battery::update(bool force) {
//     unsigned long now = millis();
//     if (!force && (now - _lastUpdate < _interval)) {
//         // ⚡️ Chỉ cập nhật trạng thái sạc (không đọc lại pin)
//         _charging = digitalRead(_chrgPin) == LOW;
//         return;
//     }
//     _lastUpdate = now;

//     digitalWrite(_enablePin, HIGH);
//     delay(5);
//     int raw = analogRead(_adcPin);
//     digitalWrite(_enablePin, LOW);

//     float v = (raw / 4095.0f) * 3.3f;
//     Serial.printf("[VP] Điện áp tại chân VP (GPIO36): %.2f V\n", v);

//     _voltage = v * _voltageDivider;

//     if (_voltage >= 4.2f) _level = 100;
//     else if (_voltage <= 3.3f) _level = 0;
//     else _level = (uint8_t)((_voltage - 3.3f) / (4.2f - 3.3f) * 100);
// }
// void Battery::update(bool force) {
//     unsigned long now = millis();

//     // ⚡ Luôn cập nhật trạng thái sạc mỗi lần gọi
//     _charging = digitalRead(_chrgPin) == LOW;

//     // ✅ Chỉ cập nhật voltage & level mỗi 30s hoặc khi force == true
//     if (!force && (now - _lastUpdate < _interval)) return;
//     _lastUpdate = now;

//     digitalWrite(_enablePin, HIGH);
//     delay(5);
//     int raw = analogRead(_adcPin);
//     digitalWrite(_enablePin, LOW);

//     float v = (raw / 4095.0f) * 3.3f;
//     _voltage = v * _voltageDivider;

//     if (_voltage >= 4.2f) _level = 100;
//     else if (_voltage <= 3.3f) _level = 0;
//     else _level = (uint8_t)((_voltage - 3.3f) / (4.2f - 3.3f) * 100);
// }
void Battery::update(bool force) {
    unsigned long now = millis();

    // Luôn cập nhật trạng thái sạc
    bool newCharging = digitalRead(_chrgPin) == LOW;
    if (newCharging != _charging) {
        _charging = newCharging;
        Serial.printf("⚡ Charging status changed: %s\n", _charging ? "CHARGING" : "NOT CHARGING");
    }

    // Cập nhật voltage và % nếu đủ 30s hoặc force == true
    if (!force && (now - _lastUpdate < _interval)) return;
    _lastUpdate = now;

    digitalWrite(_enablePin, HIGH);
    delay(5);
    int raw = analogRead(_adcPin);
    digitalWrite(_enablePin, LOW);

    float v = (raw / 4095.0f) * 3.3f;
    _voltage = v * _voltageDivider;

    if (_voltage >= 4.2f) _level = 100;
    else if (_voltage <= 3.3f) _level = 0;
    else _level = (uint8_t)((_voltage - 3.3f) / (4.2f - 3.3f) * 100);
}



float Battery::readVoltage() {
    return _voltage;
}

uint8_t Battery::getBatteryLevel() {
    return _level;
}

bool Battery::isCharging() {
    return _charging;
}
