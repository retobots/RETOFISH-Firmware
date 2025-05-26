// 	Quản lý toàn bộ tiến trình cho ăn: xử lý yêu cầu, kiểm tra pin, điều khiển motor, cập nhật trạng thái.
#pragma once

class FeedingService
{
public:
  static FeedingService &getInstance();

  void setup();
  void loop();

private:
  FeedingService() = default;
};
