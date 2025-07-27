
// #pragma once
// #include <Arduino.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ST7789.h>
// #include <SPI.h>

// class TftDisplay {
// public:
//     static TftDisplay& getInstance();
//     // void setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin);
    
//     void setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin, uint8_t backlightPin);



//     // void showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime);


//     void setCursor(int16_t x, int16_t y);
//     void setTextColor(uint16_t color);
//     void setTextSize(uint8_t size);
//     void print(const char* str);
//     void clear();
//     void resetLastStatus();
    
//     void showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime, bool charging);

    
// private:
//     TftDisplay() = default;

//     Adafruit_ST7789* _tft = nullptr;

//     String _lastLine1 = "";
//     String _lastStatus = "";
//     String _lastNextFeed = "";
//     bool _lastCharging = false;  // ⚡ Theo dõi thay đổi trạng thái sạc

 
//     uint8_t _backlightPin = 0;

//     bool _screenOn = true;
//     uint32_t _screenOnTime = 0;
//     bool _warnSpam = false;
//     void turnOffScreen();   // Tắt đèn nền
//     void turnOnScreen();    // Bật đèn nền lại

// };


#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

class TftDisplay {
public:
    static TftDisplay& getInstance();

    void setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin, uint8_t backlightPin);
    void turnOffScreen();   // Tắt đèn nền
    void turnOnScreen();    // Bật đèn nền lại

    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t color);
    void setTextSize(uint8_t size);
    void print(const char* str);
    void clear();
    void resetLastStatus();

    void showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime, bool charging);

private:
    TftDisplay() = default;

    Adafruit_ST7789* _tft = nullptr;

    String _lastLine1 = "";
    String _lastStatus = "";
    String _lastNextFeed = "";
    bool _lastCharging = false;  // ⚡ Theo dõi thay đổi trạng thái sạc

    uint8_t _backlightPin = 0; // Pin điều khiển đèn nền
    bool _screenOn = true;     // Trạng thái màn hình: bật hay tắt
    uint32_t _screenOnTime = 0; // Thời gian màn hình được bật
    bool _warnSpam = false;     // Cảnh báo spam
};
