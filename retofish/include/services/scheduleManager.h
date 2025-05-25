#pragma once

class ScheduleManager {
public:
  static ScheduleManager& getInstance();

  void setup();
  void loop();

private:
  ScheduleManager() = default;
};
