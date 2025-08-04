
// #pragma once
// #include <Arduino.h>

// class Battery {
// public:
//     static Battery& getInstance();

//     void setup();
//     // void update();
//     float readVoltage();            // renamed from getVoltage()
//     uint8_t getBatteryLevel();      // renamed from getLevel()
//     bool isCharging();
//     void update(bool force = false);  // ✅ cho phép gọi update() hoặc update(true)
    


// private:
//     Battery() = default;

//     uint8_t _adcPin = 36; // VP – đo vbat_measure
//     uint8_t _enablePin = 27; // bật đo pin _EN
//     // uint8_t _chrgPin = 34; bỏ cái báo sạc 
//     float _voltageDivider = 2.0;

//     float _voltage = 0;
//     uint8_t _level = 0;
//     const unsigned long _interval = 15000;  // thoi gian doc adc
//     unsigned long _lastUpdate = 0;
//     bool _charging = false;
    

    
// };
/////////////////////////////////////////
#pragma once
#include <Arduino.h>

class Battery {
public:
    static Battery& getInstance();

    void setup();
    void update(bool force = false);
    float readVoltage();
    uint8_t getBatteryLevel();
    bool isCharging();

private:
    Battery() = default;

    uint8_t _adcPin = 36;  // Pin ADC để đọc điện áp
    uint8_t _enablePin = 27;  // Pin enable cho mạch đo pin
    float _voltage = 0;  // Điện áp đọc được
    uint8_t _level = 0;  // Mức độ pin (0 đến 100%)
    const unsigned long _interval = 1000;  // Thời gian đọc lại mỗi lần
    unsigned long _lastUpdate = 0;  // Thời gian cập nhật gần nhất
    unsigned long _lastAdcReadTime = 0;  // Thời gian đọc ADC gần nhất

    void readBatteryVoltage();  // Hàm đọc điện áp và tính toán mức độ pin
};
