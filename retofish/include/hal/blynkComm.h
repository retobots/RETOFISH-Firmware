#pragma once

class BlynkComm {
public:
  static BlynkComm& getInstance();

  void setup();

private:
  BlynkComm() = default;
};
