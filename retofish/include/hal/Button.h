#pragma once

class Button {
public:
  static Button& getInstance();

  void setup();
  void loop();

private:
  Button() = default;
};
