//Đọc điện áp pin từ ADC, kiểm tra pin yếu, hỗ trợ hiển thị % pin.

#pragma once
#include <Arduino.h>

class Battery {
public:
  static Battery& getInstance();

  void setup();
  float readVoltage();      // đọc điện áp pin
  uint8_t getBatteryLevel(); // tính % pin

private:
  Battery() = default;

  uint8_t _adcPin = 35;  // vẫn để cho đúng template, nhưng chưa dùng khi giả lập
  float _voltageDivider = 2.0; 
};
