// Xử lý nút nhấn vật lý, phân biệt các kiểu nhấn nút
#pragma once

class Button {
public:
  static Button& getInstance();

  void setup();

private:
  Button() = default;
};
