#pragma once

class Battery {
public:
  static Battery& getInstance();

  void setup();

private:
  Battery() = default;
};
