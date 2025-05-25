#include "services/FeedingService.h"

FeedingService& FeedingService::getInstance() {
  static FeedingService instance;
  return instance;
}

void FeedingService::setup() {
  // TODO: viết hàm khởi tạo
}

void FeedingService::loop() {
  // TODO: viết hàm xử lý định kỳ
}
