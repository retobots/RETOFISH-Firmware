#include "hal/RTC.h"

RTC &RTC::getInstance()
{
    static RTC instance;
    return instance;
}

void RTC::setup()
{
    if (!_ds1307.begin())
    {
        Serial.println("Không tìm thấy DS3231!");
        while (1)
            ; // treo để debug
    }
    // //  Chỉ mở dòng này để chỉnh giờ bằng tay 1 lần, sau đó COMMENT lại
    // setTime(2025, 7, 14, 18, 34, 20);  // Ví dụ chỉnh giờ thủ công
}

DateTime RTC::now()
{
    return _ds1307.now();
}

// ✅ Hàm chỉnh giờ thủ công
void RTC::setTime(uint16_t year, uint8_t month, uint8_t day,
                  uint8_t hour, uint8_t minute, uint8_t second)
{
    _ds1307.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.printf("🛠️ Đã chỉnh RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
                  day, month, year, hour, minute, second);
}
