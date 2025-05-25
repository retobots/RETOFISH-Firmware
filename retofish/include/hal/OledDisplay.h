#pragma once

class OledDisplay {
public:
  static OledDisplay& getInstance();

  void setup();
  void loop();

private:
  OledDisplay() = default;
};
