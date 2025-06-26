
#include "hal/Battery.h"

Battery& Battery::getInstance() {
    static Battery instance;
    return instance;
}

void Battery::setup() {
    // không cần làm gì khi giả lập
}

float Battery::readVoltage() {
    // Giả lập: pin từ 100% (4.2V) xuống 0% (3.3V), rồi lên lại
    static uint8_t level = 100;
    static int8_t step = -1; // giảm

    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    // mỗi 1 giây mới cập nhật level
    if (now - lastUpdate > 1000) {
        lastUpdate = now;

        level += step;

        if (level == 0) step = +1;
        if (level == 100) step = -1;
    }

    float voltage = 3.3 + (level / 100.0f) * (4.2 - 3.3); // từ 3.3V đến 4.2V

    return voltage;
}

uint8_t Battery::getBatteryLevel() {
    float v = readVoltage();
    if (v >= 4.2) return 100;
    if (v <= 3.3) return 0;
    return (uint8_t)(((v - 3.3) / (4.2 - 3.3)) * 100);
}
