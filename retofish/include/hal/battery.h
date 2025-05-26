//Đọc điện áp pin từ ADC, kiểm tra pin yếu, hỗ trợ hiển thị % pin.
#pragma once

class Battery {
public:
  static Battery& getInstance();

  void setup();

private:
  Battery() = default;
};
