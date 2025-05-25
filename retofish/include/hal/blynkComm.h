#pragma once

class BlynkComm {
public:
  static BlynkComm& getInstance();

  void setup();
  void loop();

private:
  BlynkComm() = default;
};
