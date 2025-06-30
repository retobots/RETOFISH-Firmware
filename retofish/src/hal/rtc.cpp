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
        Serial.println("DS3231 bị mất nguồn, thiết lập lại thời gian!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // dùng thời gian biên dịch
    }
}

DateTime RTC::now() {
    return rtc.now();
}
