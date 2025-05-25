#pragma once

class OledDisplay {
public:
  static OledDisplay& getInstance();

  void setup();

private:
  OledDisplay() = default;
};
