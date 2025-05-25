#pragma once

class Button {
public:
  static Button& getInstance();

  void setup();

private:
  Button() = default;
};
