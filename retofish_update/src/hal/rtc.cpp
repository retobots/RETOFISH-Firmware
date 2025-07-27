#include "hal/RTC.h"
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

RTC& RTC::getInstance() {
    static RTC instance;
    return instance;
}

void RTC::setup() {
    Wire.begin(21, 22);  // SDC = D21, SCL = D22

    if (!rtc.begin()) {
        Serial.println("Không tìm thấy DS3231!");
        while (1);  // treo để debug
    }

    if (rtc.lostPower()) {
        Serial.println("⚠️ DS3231 bị mất nguồn, thiết lập lại thời gian!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // dùng thời gian biên dịch
    }

    // //  Chỉ mở dòng này để chỉnh giờ bằng tay 1 lần, sau đó COMMENT lại
    // setTime(2025, 7, 14, 17, 06, 0);  // Ví dụ chỉnh giờ thủ công
}

DateTime RTC::now() {
    return rtc.now();
}

// ✅ Hàm chỉnh giờ thủ công
void RTC::setTime(uint16_t year, uint8_t month, uint8_t day,
                  uint8_t hour, uint8_t minute, uint8_t second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.printf("🛠️ Đã chỉnh RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
        day, month, year, hour, minute, second);
}
