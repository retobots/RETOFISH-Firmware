
#pragma once
#include <Arduino.h>

class Battery {
public:
    static Battery& getInstance();

    void setup();
    // void update();
    float readVoltage();            // renamed from getVoltage()
    uint8_t getBatteryLevel();      // renamed from getLevel()
    bool isCharging();
    void update(bool force = false);  // ✅ cho phép gọi update() hoặc update(true)
    


private:
    Battery() = default;

    uint8_t _adcPin = 36; // VP – đo vbat_measure
    uint8_t _enablePin = 5;
    uint8_t _chrgPin = 34;
    float _voltageDivider = 2.0;

    float _voltage = 0;
    uint8_t _level = 0;
    const unsigned long _interval = 15000; // 30 giây
    unsigned long _lastUpdate = 0;
    bool _charging = false;
    

    
};
