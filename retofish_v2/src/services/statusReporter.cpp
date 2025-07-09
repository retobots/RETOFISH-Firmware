#include "services/StatusReporter.h"

StatusReporter& StatusReporter::getInstance() {
  static StatusReporter instance;
  return instance;
}

void StatusReporter::setup() {
  // TODO: viết hàm khởi tạo
}

void StatusReporter::loop() {
  // TODO: viết hàm xử lý định kỳ
}
