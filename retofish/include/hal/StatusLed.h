// 	Điều khiển LED RGB để báo trạng thái hoạt động (Idle, Feeding, LowBatt, Offline...).
#pragma once

class StatusLed {
public:
  static StatusLed& getInstance();

  void setup();

private:
  StatusLed() = default;
};
