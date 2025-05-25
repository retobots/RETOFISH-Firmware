#pragma once

class StatusReporter {
public:
  static StatusReporter& getInstance();

  void setup();
  void loop();

private:
  StatusReporter() = default;
};
