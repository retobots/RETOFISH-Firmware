// Giao tiếp với mạch thời gian thực DS3231 hoặc DS1307, lấy thời gian chính xác để kiểm tra lịch.
#pragma once

#include <Arduino.h>
#include <RTClib.h>

class RTC {
public:
  static RTC& getInstance();

  void setup();
  DateTime now();

  // ✅ Hàm chỉnh giờ thủ công (chỉ dùng 1 lần rồi comment lại)
  void setTime(uint16_t year, uint8_t month, uint8_t day,
               uint8_t hour, uint8_t minute, uint8_t second);

private:
  RTC() = default;
};
