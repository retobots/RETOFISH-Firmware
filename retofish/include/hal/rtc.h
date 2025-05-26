// Giao tiếp với mạch thời gian thực DS3231 hoặc DS1307, lấy thời gian chính xác để kiểm tra lịch.
#pragma once

class RTC {
public:
  static RTC& getInstance();

  void setup();

private:
  RTC() = default;
};
