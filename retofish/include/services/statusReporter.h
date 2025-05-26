// Định kỳ gửi trạng thái lên cloud, cập nhật LED & OLED theo trạng thái hệ thống.
#pragma once

class StatusReporter {
public:
  static StatusReporter& getInstance();

  void setup();
  void loop();

private:
  StatusReporter() = default;
};
