#pragma once

class StatusLed {
public:
  static StatusLed& getInstance();

  void setup();

private:
  StatusLed() = default;
};
