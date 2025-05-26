//Giao tiếp với Blynk: nhận lệnh từ app (Feed Now, lịch mới), gửi dữ liệu trạng thái lên cloud.
#pragma once

class BlynkComm {
public:
  static BlynkComm& getInstance();

  void setup();

private:
  BlynkComm() = default;
};
