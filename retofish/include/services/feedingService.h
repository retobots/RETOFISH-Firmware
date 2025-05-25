#pragma once

class FeedingService {
public:
  static FeedingService& getInstance();

  void setup();
  void loop();

private:
  FeedingService() = default;
};
