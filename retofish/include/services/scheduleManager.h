// Kiểm tra thời gian hiện tại so với lịch đã cài để trigger việc cho ăn. Cập nhật lịch mới từ app
#pragma once

class ScheduleManager {
public:
  static ScheduleManager& getInstance();

  void setup();
  void loop();

private:
  ScheduleManager() = default;
};
