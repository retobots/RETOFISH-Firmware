// Hiển thị thông tin lên màn hình OLED: thời gian, trạng thái, điện áp, lịch cho ăn.
#pragma once

class OledDisplay {
public:
  static OledDisplay& getInstance();

  void setup();

private:
  OledDisplay() = default;
};
