#pragma once

class RTC {
public:
  static RTC& getInstance();

  void setup();

private:
  RTC() = default;
};
