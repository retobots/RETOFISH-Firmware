

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
    void update(bool force = false);  // Thêm tham số force

}



void Battery::update(bool force) {
    unsigned long now = millis();

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
    return false;
}
