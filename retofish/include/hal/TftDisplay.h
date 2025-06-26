
#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

class TftDisplay {
public:
    static TftDisplay& getInstance();

    void setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin);

    void showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime);
    void turnOff();  // Tắt hẳn màn

private:
    TftDisplay() = default;

    Adafruit_ST7789* _tft = nullptr;

    String _lastLine1 = "";
    String _lastStatus = "";
    String _lastNextFeed = "";
};
