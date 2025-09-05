

// #include "hal/Battery.h"

// Battery& Battery::getInstance() {
//     static Battery instance;
//     return instance;
// }

// void Battery::setup() {
//     pinMode(_adcPin, INPUT);
//     pinMode(_enablePin, OUTPUT);
//     // pinMode(_chrgPin, INPUT);
//     analogReadResolution(12);
//     _lastUpdate = 0;
//     void update(bool force = false); 

// }

// void Battery::update(bool force) {
//     unsigned long now = millis();

//     if (!force && (now - _lastUpdate < _interval)) return;
//     _lastUpdate = now;

//     // Kích hoạt chân enable và đọc giá trị ADC từ _adcPin
//     digitalWrite(_enablePin, HIGH);
//     delay(5);  // Đợi tín hiệu ổn định
//     int raw = analogRead(_adcPin);  // Đọc giá trị từ ADC
//     digitalWrite(_enablePin, LOW);  // Tắt chân enable sau khi đọc

//     // In ra giá trị ADC thô
//     Serial.print("Raw ADC Value: ");
//     Serial.println(raw);  // In giá trị ADC thô

//     // Chuyển giá trị ADC thành điện áp (không áp dụng bộ chia điện áp)
//     float v = (raw / 4095.0f) * 3.3f;  // Tính điện áp từ giá trị ADC
//     _voltage = v;    // Không nhân với bộ chia điện áp nếu không dùng


//     _voltage = v* 1.31f; 

//     // In ra giá trị điện áp đã hiệu chỉnh
//     Serial.print("Corrected Voltage: ");
//     Serial.println(_voltage, 2);  // In điện áp đã hiệu chỉnh

//     if (_voltage > 4.3f) {
//         // Nếu điện áp trên 4.3V, hiển thị biểu tượng sạc
//         Serial.println("Charging..."); 
        
//     } else if (_voltage >= 4.1f) {
//         _level = 100;  // Pin đầy (từ 4.1V đến 4.3V)
//     } else if (_voltage <= 3.3f) {
//         _level = 0;    // Pin cạn kiệt
//     } else {
       
//         _level = (uint8_t)((_voltage - 3.3f) / (4.1f - 3.3f) * 100);
//     }

//     // In giá trị ADC và mức độ pin
//     Serial.print("Level: ");
//     Serial.print(_level);  // In mức độ pin (từ 0 đến 100)
//     Serial.println("%");  // In phần trăm

//     Serial.println();  // In một dòng trống để dễ phân biệt giữa các lần đọc
// }


// float Battery::readVoltage() {
//     return _voltage;
// }

// uint8_t Battery::getBatteryLevel() {
//     return _level;
// }

// bool Battery::isCharging() {
//     return false;
// }
 //////////////////////////////////
 #include "hal/Battery.h"

Battery& Battery::getInstance() {
    static Battery instance;
    return instance;
}

void Battery::setup() {
    pinMode(_adcPin, INPUT);      // Chân ADC
    pinMode(_enablePin, OUTPUT);  // Chân enable
    // pinMode(_chrgPin, INPUT);  // Nếu bạn có mạch sạc, đọc chân GPIO này
    analogReadResolution(12);     // Đọc ADC với độ phân giải 12 bit
    _lastUpdate = 0;
    _lastAdcReadTime = 0;         // Thêm biến thời gian đọc ADC gần nhất
}

void Battery::update(bool force) {
    unsigned long now = millis();

    // Kiểm tra nếu không cần thiết phải cập nhật (nếu force không được gọi)
    if (!force && (now - _lastUpdate < _interval)) return;
    _lastUpdate = now;

    // Kiểm tra nếu điện áp trên 4.3V
    if (_voltage > 4.3f) {
        // Nếu điện áp trên 4.3V, đọc ADC mỗi 1 giây
        if (now - _lastAdcReadTime >= 1000) {  // 1000 ms = 1 giây
            // Đọc giá trị ADC
            readBatteryVoltage();
            _lastAdcReadTime = now;  // Cập nhật thời gian
        }
    } else {
        // Nếu điện áp <= 4.3V, đọc ADC mỗi 5 giây
        if (now - _lastAdcReadTime >= 5000) {  // 15000 ms = 15 giây
            // Đọc giá trị ADC
            readBatteryVoltage();
            _lastAdcReadTime = now;  // Cập nhật thời gian
        }
    }
}

void Battery::readBatteryVoltage() {
    // Kích hoạt chân enable và đọc giá trị ADC từ _adcPin
    digitalWrite(_enablePin, HIGH);
    delay(5);  // Đợi tín hiệu ổn định
    int raw = analogRead(_adcPin);  // Đọc giá trị từ ADC
    digitalWrite(_enablePin, LOW);  // Tắt chân enable sau khi đọc

    // Chuyển giá trị ADC thành điện áp (không áp dụng bộ chia điện áp)
    float v = (raw / 4095.0f) * 3.3f;  // Tính điện áp từ giá trị ADC
    _voltage = v * 1.31f;  // Hiệu chỉnh điện áp nếu có bộ chia điện áp

    // In ra giá trị điện áp đã hiệu chỉnh
    // Serial.print("Corrected Voltage: ");
    // Serial.println(_voltage, 2);  // In điện áp đã hiệu chỉnh

    if (_voltage > 4.3f) {
    // Nếu điện áp trên 4.3V, hiển thị biểu tượng sạc
    Serial.println("Charging..."); 
    
    } else if (_voltage >= 4.1f) {
        _level = 100;  // Pin đầy (từ 4.1V đến 4.3V)
    } else if (_voltage <= 3.3f) {
        _level = 0;    // Pin cạn kiệt (dưới hoặc bằng 3.3V)
    } else {
        // Tính mức độ pin nếu điện áp trong khoảng 3.3V đến 4.1V
        _level = (uint8_t)((_voltage - 3.3f) / (4.1f - 3.3f) * 100);
    }


    // In giá trị ADC và mức độ pin
    // Serial.print("Level: ");
    // Serial.print(_level);  // In mức độ pin (từ 0 đến 100)
    // Serial.println("%");   // In phần trăm

    Serial.println();  // In một dòng trống để dễ phân biệt giữa các lần đọc
}

float Battery::readVoltage() {
    return _voltage;
}

uint8_t Battery::getBatteryLevel() {
    return _level;
}

bool Battery::isCharging() {
    // Kiểm tra chân GPIO _chrgPin nếu có (Nếu có mạch sạc, cần đọc chân này)
    // Nếu có mạch sạc, có thể đọc trạng thái của chân GPIO này
    // return digitalRead(_chrgPin) == HIGH;
    return false;  // Ví dụ này không sử dụng mạch sạc, luôn trả về false
}
