

#include "hal/Battery.h"

Battery& Battery::getInstance() {
    static Battery instance;
    return instance;
}

void Battery::setup() {
    pinMode(_adcPin, INPUT);
    pinMode(_enablePin, OUTPUT);
    // pinMode(_chrgPin, INPUT);
    analogReadResolution(12);
    _lastUpdate = 0;
    void update(bool force = false); 

}



void Battery::update(bool force) {
    unsigned long now = millis();

    if (!force && (now - _lastUpdate < _interval)) return;
    _lastUpdate = now;

    // Kích hoạt chân enable và đọc giá trị ADC từ _adcPin
    digitalWrite(_enablePin, HIGH);
    delay(5);  // Đợi tín hiệu ổn định
    int raw = analogRead(_adcPin);  // Đọc giá trị từ ADC
    digitalWrite(_enablePin, LOW);  // Tắt chân enable sau khi đọc

    // In ra giá trị ADC thô
    Serial.print("Raw ADC Value: ");
    Serial.println(raw);  // In giá trị ADC thô

    // Chuyển giá trị ADC thành điện áp (không áp dụng bộ chia điện áp)
    float v = (raw / 4095.0f) * 3.3f;  // Tính điện áp từ giá trị ADC
    _voltage = v;    // Không nhân với bộ chia điện áp nếu không dùng


    _voltage = v* 1.31f; 

    // In ra giá trị điện áp đã hiệu chỉnh
    Serial.print("Corrected Voltage: ");
    Serial.println(_voltage, 2);  // In điện áp đã hiệu chỉnh

    if (_voltage > 4.3f) {
        // Nếu điện áp trên 4.3V, hiển thị biểu tượng sạc
        Serial.println("Charging...");
        _level = 110; 
        
    } else if (_voltage >= 4.1f) {
        _level = 100;  // Pin đầy (từ 4.1V đến 4.3V)
    } else if (_voltage <= 3.3f) {
        _level = 0;    // Pin cạn kiệt
    } else {
       
        _level = (uint8_t)((_voltage - 3.3f) / (4.1f - 3.3f) * 100);
    }

    // In giá trị ADC và mức độ pin
    Serial.print("Level: ");
    Serial.print(_level);  // In mức độ pin (từ 0 đến 100)
    Serial.println("%");  // In phần trăm

    Serial.println();  // In một dòng trống để dễ phân biệt giữa các lần đọc
}



float Battery::readVoltage() {
    return _voltage;
}

uint8_t Battery::getBatteryLevel() {
    return _level;
}

bool Battery::isCharging() {
    return false;
}
