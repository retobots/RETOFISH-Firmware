// Giao tiếp với mạch thời gian thực DS3231 hoặc DS1307, lấy thời gian chính xác để kiểm tra lịch.
#pragma once

#include <Arduino.h>
#include <RTClib.h>

class RTC {
public:
  static RTC& getInstance();

  void setup();
  DateTime now();

private:
  RTC() = default;
};
