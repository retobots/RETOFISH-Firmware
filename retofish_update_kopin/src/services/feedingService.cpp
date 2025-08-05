
#include "services/FeedingService.h"
#include "hal/Battery.h"
#include "hal/RTC.h"
#include "hal/Button.h"
#include "hal/TftDisplay.h"
#include "hal/StepperMotor.h"
#include "hal/StatusLed.h"
#include "services/ScheduleManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


FeedingService& FeedingService::getInstance() {
    static FeedingService instance;
    return instance;
}

void FeedingService::setup() {
    _inStandbyMode = true; 
    _screenOn = true;
    _screenOnTime = millis();
    _lastManualFeedTime = millis() - 30000;
    _lastAutoFeedTime = 0;
    Battery::getInstance().update(true);           // ✅ cập nhật pin ngay khi khởi động
    updateDisplayAndLed();                     // ✅ vẽ trạng thái pin lên màn hình
}

void FeedingService::loop() {
    Button& btn = Button::getInstance();
    btn.update();
    auto evt = btn.getEvent();
    int delta = btn.getRotationDelta();
    // Battery::getInstance().update(true); 
    Battery::getInstance().update(false);

    if (_inSettingMode) {
        handleSetting(delta, evt);
        return;
    }



    handleButton(evt);
    handleAutoFeeding();
    checkScreenTimeout();
    checkFeedingTimeout();
    checkWarningTimeout();
    updateDisplayAndLed();


    delay(20);
}

// void FeedingService::handleSetting(int delta, Button::Event evt) {
//     switch (_settingPage) {
//         case SettingPage::NewPage:

//             if (delta != 0) {
//             _selectedSlot = constrain(_selectedSlot + delta, 0, 2); // 2: back
//             renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 if (_selectedSlot == 2) {
//                     _inSettingMode = false;  //
//                     _screenOnTime = millis(); //
//                     auto& display = TftDisplay::getInstance();
//                     display.clear();
//                     // ✅ XÓA CACHE để bắt buộc vẽ lại mọi thứ
//                     display.resetLastStatus();  // bạn sẽ thêm hàm này ở bước dưới

//                     updateDisplayAndLed();
//                 }
//             }

//             // Nếu người dùng nhấn nút xác nhận (Click)
//             if (evt == Button::Event::Click) {
//                 if (_selectedSlot == 0) {
//                     // Chọn "Select Feeding Time" → Quay lại trang SelectSlot
//                     _settingPage = SettingPage::SelectSlot;
//                     renderSettingPage();
//                 } else if (_selectedSlot == 1) {
//                     // Chọn "Setting Time Now" → Cài đặt thời gian thực
//                     _settingPage = SettingPage::SetHour_1; // Đi tới cài đặt giờ thực
//                     _hour = RTC::getInstance().now().hour();  // Đặt giờ từ RTC hiện tại
//                     renderSettingPage();  // Hiển thị trang chỉnh giờ
//                 }
//             }
//             break;

//         case SettingPage::SetHour_1:
//             if (delta != 0) {
//                 _hour = (_hour + delta + 24) % 24; // Cập nhật giờ khi xoay encoder
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 // Nhấn để xác nhận giờ và chuyển sang chỉnh phút
//                 _settingPage = SettingPage::SetMinute_1;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::SetMinute_1:
//             if (delta != 0) {
//                 _minute = (_minute + delta + 60) % 60; // Cập nhật phút khi xoay encoder
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 // Nhấn để xác nhận phút và chuyển đến trang xác nhận lưu
//                 _settingPage = SettingPage::SetSave;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::SetSave:
//             if (delta != 0) {
//                 _confirmIndex = (_confirmIndex + delta + 2) % 2;  // Chọn "Yes" hoặc "No"
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 if (_confirmIndex == 0) {
//                     // Chọn "Yes" → Lưu thời gian vào RTC
                    
//                     RTC::getInstance().setTime(2025, 7, 24, _hour, _minute, 0);
//                     Serial.printf("Time set to: %02d:%02d\n", _hour, _minute);
//                 }
//                 // Quay lại trang NewPage sau khi lưu hoặc huỷ
//                 _settingPage = SettingPage::NewPage;
//                 renderSettingPage();
//             }
//             break;
//         case SettingPage::SelectSlot:
//             if (delta != 0) {
//                 _selectedSlot = constrain(_selectedSlot + delta, 0, 3); // 0-2: slot, 3: back
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 if (_selectedSlot == 3) {

//                     _settingPage = SettingPage::NewPage;
//                     renderSettingPage();

//                 } else {
//                     const FeedTime* ft = ScheduleManager::getInstance().getSlot(_selectedSlot);
//                     if (ft) {
//                         _hour = ft->hour;
//                         _minute = ft->minute;
//                         _duration = ft->duration;
//                     } else {
//                         _hour = 7;
//                         _minute = 0;
//                         _duration = 10;
//                     }
//                     _settingPage = SettingPage::SetHour;
//                     renderSettingPage();
//                 }
//             }   

//             break;

//         case SettingPage::SetHour:
//             if (delta != 0) {
//                 _hour = (_hour + delta + 24) % 24;
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 _settingPage = SettingPage::SetMinute;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::SetMinute:
//             if (delta != 0) {
//                 _minute = (_minute + delta + 60) % 60;
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 _settingPage = SettingPage::SetDuration;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::SetDuration:
//             if (delta != 0) {
//                 _duration = constrain(_duration + delta, 4, 10);  // 4-10 số vòng đc chọn
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 _settingPage = SettingPage::SetEnabled;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::SetEnabled:
//             if (delta != 0) {
//                 _confirmIndex = (_confirmIndex + delta + 2) % 2;  // ✅ Xoay chọn YES/NO
//                 renderSettingPage();
//             }
//             if (evt == Button::Event::Click) {
//                 _enabled = (_confirmIndex == 0);  // ✅ YES → bật, NO → tắt
//                 _settingPage = SettingPage::ConfirmSave;
//                 renderSettingPage();
//             }
//             break;

//         case SettingPage::ConfirmSave:
//             if (delta != 0) {
//                 _confirmIndex = (_confirmIndex + delta + 2) % 2;
//                 renderSettingPage();
//             }


//             if (evt == Button::Event::Click) {
//                     if (_confirmIndex == 0) {
//                         // ✅ Kiểm tra trùng giờ
//                         bool isDuplicate = ScheduleManager::getInstance().isTimeUsedByOtherSlot(_selectedSlot, _hour, _minute);
//                         if (isDuplicate) {
//                             Serial.println("⚠️ Trùng giờ với slot khác → Không lưu");

//                             auto& tft = TftDisplay::getInstance();
//                             tft.clear();
//                             tft.setTextSize(3);
//                             tft.setTextColor(ST77XX_RED);
//                             tft.setCursor(10, 70);
//                             tft.print("Time already used");
//                             delay(2000);
//                             _settingPage = SettingPage::SetHour;
//                             renderSettingPage();
//                             return;
//                         }

//                         Serial.printf("✅ Saved: Slot %d = %02d:%02d for %ds (%s)\n",
//                             _selectedSlot + 1, _hour, _minute, _duration, _enabled ? "ENABLED" : "DISABLED");

//                         ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration, _enabled);

//                         // ✅ Lưu vào EEPROM ngay sau khi cập nhật
//                         ScheduleManager::getInstance().saveToEEPROM();
//                     } else {
//                         Serial.printf("❌ Cancel Save\n");
//                     }

//                     _settingPage = SettingPage::SelectSlot;
//                     renderSettingPage();
//                 }

//             break;
//     }
// }

void FeedingService::handleSetting(int delta, Button::Event evt) {
    switch (_settingPage) {
        case SettingPage::NewPage:

            if (delta != 0) {
            _selectedSlot = constrain(_selectedSlot + delta, 0, 2); // 2: back
            renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                if (_selectedSlot == 2) {
                    _inSettingMode = false;  //
                    _screenOnTime = millis(); //
                    auto& display = TftDisplay::getInstance();
                    display.clear();
                    // ✅ XÓA CACHE để bắt buộc vẽ lại mọi thứ
                    display.resetLastStatus();  // bạn sẽ thêm hàm này ở bước dưới

                    updateDisplayAndLed();
                }
            }

            // Nếu người dùng nhấn nút xác nhận (Click)
            if (evt == Button::Event::Click) {
                if (_selectedSlot == 0) {
                    // Chọn "Select Feeding Time" → Quay lại trang SelectSlot
                    _settingPage = SettingPage::SelectSlot;
                    renderSettingPage();
                } else if (_selectedSlot == 1) {
                    // Chọn "Setting Time Now" → Cài đặt thời gian thực
                    _settingPage = SettingPage::SetHour_1; // Đi tới cài đặt giờ thực
                    _hour = RTC::getInstance().now().hour();  // Đặt giờ từ RTC hiện tại
                    renderSettingPage();  // Hiển thị trang chỉnh giờ
                }
            }
            break;

        case SettingPage::SetHour_1:
            if (delta != 0) {
                _hour = (_hour + delta + 24) % 24; // Cập nhật giờ khi xoay encoder
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                // Nhấn để xác nhận giờ và chuyển sang chỉnh phút
                _settingPage = SettingPage::SetMinute_1;
                renderSettingPage();
            }
            break;

        case SettingPage::SetMinute_1:
            if (delta != 0) {
                _minute = (_minute + delta + 60) % 60; // Cập nhật phút khi xoay encoder
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                // Nhấn để xác nhận phút và chuyển đến trang xác nhận lưu
                _settingPage = SettingPage::SetSave;
                renderSettingPage();
            }
            break;

        case SettingPage::SetSave:
            if (delta != 0) {
                _confirmIndex = (_confirmIndex + delta + 2) % 2;  // Chọn "Yes" hoặc "No"
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                if (_confirmIndex == 0) {
                    // Chọn "Yes" → Lưu thời gian vào RTC
                    
                    RTC::getInstance().setTime(2025, 7, 24, _hour, _minute, 0);
                    Serial.printf("Time set to: %02d:%02d\n", _hour, _minute);
                }
                // Quay lại trang NewPage sau khi lưu hoặc huỷ
                _settingPage = SettingPage::NewPage;
                renderSettingPage();
            }
            break;
        case SettingPage::SelectSlot:
            if (delta != 0) {
                _selectedSlot = constrain(_selectedSlot + delta, 0, 3); // 0-2: slot, 3: back
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                if (_selectedSlot == 3) {

                    _settingPage = SettingPage::NewPage;
                    renderSettingPage();

                } else {
                    const FeedTime* ft = ScheduleManager::getInstance().getSlot(_selectedSlot);
                    if (ft) {
                        _hour = ft->hour;
                        _minute = ft->minute;
                        _duration = ft->duration;
                    } else {
                        _hour = 7;
                        _minute = 0;
                        _duration = 10;
                    }
                    _settingPage = SettingPage::SetHour;
                    renderSettingPage();
                }
            }   

            break;

        case SettingPage::SetHour:
            if (delta != 0) {
                _hour = (_hour + delta + 24) % 24;
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetMinute;
                renderSettingPage();
            }
            break;

        case SettingPage::SetMinute:
            if (delta != 0) {
                _minute = (_minute + delta + 60) % 60;
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetDuration;
                renderSettingPage();
            }
            break;

        case SettingPage::SetDuration:
            if (delta != 0) {
                _duration = constrain(_duration + delta, 4, 10);  // 4-10 số vòng đc chọn
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _settingPage = SettingPage::SetEnabled;
                renderSettingPage();
            }
            break;

        case SettingPage::SetEnabled:
            if (delta != 0) {
                _confirmIndex = (_confirmIndex + delta + 2) % 2;  // ✅ Xoay chọn YES/NO
                renderSettingPage();
            }
            if (evt == Button::Event::Click) {
                _enabled = (_confirmIndex == 0);  // ✅ YES → bật, NO → tắt
                _settingPage = SettingPage::ConfirmSave;
                renderSettingPage();
            }
            break;

        case SettingPage::ConfirmSave:
            if (delta != 0) {
                _confirmIndex = (_confirmIndex + delta + 2) % 2;
                renderSettingPage();
            }
if (evt == Button::Event::Click) {
    if (_confirmIndex == 0) {
        // Tính số phút của Time 1 (Time 1 là giờ hiện tại _hour và _minute)
        int time1InMinutes = (_hour * 60) + _minute;
        Serial.printf("Time 1: %02d:%02d = %d minutes\n", _hour, _minute, time1InMinutes); // Log Time 1

        // Kiểm tra Time 2: Time 2 phải cách Time 1 ít nhất 10 phút
        int minTime2 = time1InMinutes + 10;  // Time 1 + 10 phút (giới hạn cho Time 2)
        int selectedTime2 = _minute + _hour * 60;  // Thời gian hiện tại cho Time 2
        Serial.printf("Time 2: %02d:%02d = %d minutes\n", _hour, _minute, selectedTime2); // Log Time 2

        if (_selectedSlot == 1) {  // Kiểm tra cho Time 2 (Slot 1)
            if (selectedTime2 < minTime2) {
                // Nếu Time 2 không hợp lệ (nhỏ hơn Time 1 + 10 phút), yêu cầu người dùng nhập lại
                auto& tft = TftDisplay::getInstance();
                tft.clear();
                tft.setTextSize(3);
                tft.setTextColor(ST77XX_RED);
                tft.setCursor(10, 70);
                tft.print("Time 2 must be 10 mins after Time 1");
                delay(2000);
                _settingPage = SettingPage::SetHour;
                renderSettingPage();
                return;
            }
        }

        // Kiểm tra Time 3: Time 3 phải cách Time 2 ít nhất 10 phút và nhỏ hơn Time 1
        if (_selectedSlot == 2) {  // Kiểm tra cho Time 3 (Slot 2)
            int minTime3 = selectedTime2 + 10;  // Time 2 + 10 phút (giới hạn cho Time 3)
            int selectedTime3 = _minute + _hour * 60;  // Thời gian hiện tại cho Time 3
            Serial.printf("Time 3: %02d:%02d = %d minutes\n", _hour, _minute, selectedTime3); // Log Time 3

            if (selectedTime3 < minTime3) {
                // Nếu Time 3 không hợp lệ (nhỏ hơn Time 2 + 10 phút), yêu cầu người dùng nhập lại
                auto& tft = TftDisplay::getInstance();
                tft.clear();
                tft.setTextSize(3);
                tft.setTextColor(ST77XX_RED);
                tft.setCursor(10, 70);
                tft.print("Time 3 must be 10 mins after Time 2");
                delay(2000);
                _settingPage = SettingPage::SetHour;
                renderSettingPage();
                return;
            }

            // Kiểm tra Time 3 phải nhỏ hơn Time 1
            if (selectedTime3 >= time1InMinutes) {
                auto& tft = TftDisplay::getInstance();
                tft.clear();
                tft.setTextSize(3);
                tft.setTextColor(ST77XX_RED);
                tft.setCursor(10, 70);
                tft.print("Time 3 must be before Time 1");
                delay(2000);
                _settingPage = SettingPage::SetHour;
                renderSettingPage();
                return;
            }
        }

        // Lưu thời gian vào RTC
        RTC::getInstance().setTime(2025, 7, 24, _hour, _minute, 0);
        Serial.printf("Time set to: %02d:%02d\n", _hour, _minute);

        // Lưu vào lịch và EEPROM
        ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration, _enabled);
        ScheduleManager::getInstance().saveToEEPROM();
    }
    _settingPage = SettingPage::SelectSlot;
    renderSettingPage();
}
break;







    }
}
void FeedingService::handleButton(Button::Event evt) {
    unsigned long now = millis();
    

    if (Button::getInstance().getRawPressedDuration() >= 3000 && !_inSettingMode) {
        if (!_screenOn) {

                return; 
            } else {
        
        Serial.println("Vao che do setting");
        _inSettingMode = true;
        _settingPage = SettingPage::NewPage;
        _screenOn = true;
        _screenOnTime = now;
        _selectedSlot = 0;
        _hour = 7;
        _minute = 0;
        _duration = 10;
        _confirmIndex = 0;
        renderSettingPage();
        return;
        }
    }



if (evt == Button::Event::Click) {
    if (!_screenOn) {
       
        TftDisplay::getInstance().turnOnScreen(); 
        _inStandbyMode = true;

        _screenOn = true;
        
        _warnSpam = false;
        
        _screenOnTime = millis(); 
        auto& display = TftDisplay::getInstance();
        display.clear();
        
        display.resetLastStatus();  

        updateDisplayAndLed();
        Serial.println("Screen ON by Button");
    }
}




if (evt == Button::Event::DoubleClick) {
    if (!_screenOn) {

        return; 
    } else {

        if (now - _lastManualFeedTime > 30000) {
            _feeding = true;
            _warnSpam = false;
            Serial.println("Feeding START → 4 vòng");
            updateDisplayAndLed();
            StepperMotor::getInstance().feedForRounds(4);  // ✅ QUAY 4 VÒNG  định

            _feedingStartTime = now;
            _lastManualFeedTime = now;
             
            
            updateDisplayAndLed();
        } else {
            _warnSpam = true;
            _warnStartTime = now;
            Serial.println("Feed ignored → PLEASE WAIT");
            updateDisplayAndLed();
        }
    }
}

}

void FeedingService::updateDisplayAndLed() {
    float voltage = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();

    const char* statusStr;
    auto& led = StatusLed::getInstance();

    if (_warnSpam) {
        statusStr = "Wait 30s";
    } else if (_feeding) {
        led.setStatus(StatusLed::State::Feeding);
        statusStr = "Feeding";
    // } else if (level < 15) {
    //     led.setStatus(StatusLed::State::LowBattery);
    //     statusStr = "Low Battery";
    } else {
        led.setStatus(StatusLed::State::Idle);
        statusStr = "Standby";
    }

    led.update();

    if (_screenOn) {

        DateTime now = RTC::getInstance().now();
        const FeedTime* next = ScheduleManager::getInstance().getNextFeedTime(now);

        if (next) {
            int hour12 = next->hour % 12;
            if (hour12 == 0) hour12 = 12;
            const char* ampm = next->hour < 12 ? "AM" : "PM";

            char timeStr[16];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, next->minute, ampm);

           
            // TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, timeStr, Battery::getInstance().isCharging());
            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, timeStr, false);
        } else {
      
            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, "No setting", Battery::getInstance().isCharging());

        }
    }
}


void FeedingService::handleAutoFeeding() {    
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();

    if (ScheduleManager::getInstance().isTimeToFeed(nowRtc)) {
        // Bật màn hình khi bắt đầu cho ăn tự động
        TftDisplay::getInstance().turnOnScreen(); 
        _feeding = true;
        Serial.println("[Auto] Feeding START");

        _screenOn = true;
        _screenOnTime = millis();
        auto& display = TftDisplay::getInstance();
        display.clear();
        
        // Xóa cache để bắt buộc vẽ lại mọi thứ
        display.resetLastStatus(); 

        // Hiển thị trạng thái "Đang cho ăn"
        updateDisplayAndLed();  // Cập nhật màn hình hiển thị trạng thái

        // Thực hiện cho ăn tự động theo số vòng đã chọn
        StepperMotor::getInstance().feedForRounds(_duration); 
        
        _feedingStartTime = now;
        _lastAutoFeedTime = now;

        Serial.println("Screen ON by Auto");
    }
}




void FeedingService::checkScreenTimeout() {
    // Trả về ngay nếu đang trong chế độ cài đặt hoặc đang cho ăn
    if (_inSettingMode || _feeding) return;

    // Kiểm tra nếu hệ thống đang ở chế độ "chờ" và đã 15 giây không có hoạt động
    if (_inStandbyMode && _screenOn && (millis() - _screenOnTime > 15000)) {
        _screenOn = false;  // Tắt màn hình
        digitalWrite(14, LOW);  // Tắt đèn nền
        TftDisplay::getInstance().turnOffScreen();  // Tắt màn hình TFT
        _inStandbyMode = false;  // Kết thúc chế độ "chờ" sau khi tắt màn hình
    }
}
void FeedingService::checkFeedingTimeout() {
    if (_feeding && (millis() - _feedingStartTime > _feedingDuration)) {
        _feeding = false;
        updateDisplayAndLed();  // Cập nhật trạng thái sau khi cho ăn xong

        // Sau khi cho ăn xong, chuyển vào chế độ "chờ"
        _inStandbyMode = true;  // Đặt _inStandbyMode thành true (vào chế độ "chờ")
        _screenOnTime = millis();  // Ghi lại thời gian vào chế độ "chờ"
    }
}



void FeedingService::checkWarningTimeout() {
    if (_warnSpam && (millis() - _warnStartTime > 5000)) {
        _warnSpam = false;
    }
}



void FeedingService::renderSettingPage() {
    auto& tft = TftDisplay::getInstance();
    tft.clear();

    switch (_settingPage) {
        

        case SettingPage::NewPage: {
            tft.clear();
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(20, 10);
            tft.print("Select time now");
            // Mảng các lựa chọn
            const char* options[] = {
                "Feeding Time",   // 0
                "Time Now",      // 1
                "Back"                   // 2 (thêm "Back" nếu cần)
            };

            // Vẽ các lựa chọn với vòng lặp
            for (int i = 0; i < 3; i++) {
                tft.setCursor(20, 50 + i * 30);  // Cập nhật vị trí của từng dòng
                tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);  // Dùng màu vàng cho lựa chọn đang được chọn
                tft.print(options[i]);  // In lựa chọn ra màn hình
            }



            break;
        }


        case SettingPage::SetHour_1: {
            char hourStr[16];
            snprintf(hourStr, sizeof(hourStr), "Hour: %02d", _hour);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(hourStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");
            tft.setCursor(20, 150);
            tft.print("Click to confirm");
            break;
        }



        case SettingPage::SetMinute_1: {
            char minStr[16];
            snprintf(minStr, sizeof(minStr), "Minute: %02d", _minute);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(minStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");
            tft.setCursor(20, 150);
            tft.print("Click to confirm");
            break;
        }

        case SettingPage::SetSave: {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(60, 20);
            tft.print("Save this setting?");

            const char* options[2] = { "YES", "NO" };
            for (int i = 0; i < 2; i++) {
                tft.setCursor(60 + i * 160, 100);
                tft.setTextSize(3);
                tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
                tft.print(options[i]);
            }
            break;
        }

        case SettingPage::SelectSlot: {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(20, 10);
            tft.print("Select feeding time");

            const char* labels[4] = { "Timer 1/3", "Timer 2/3", "Timer 3/3", "BACK" };


            for (int i = 0; i < 4; i++) {
                    tft.setCursor(20, 50 + i * 30);
                    tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);
                    tft.print(labels[i]);

                    if (i < 3) {
                        const FeedTime* ft = ScheduleManager::getInstance().getSlot(i);

                        // Hiển thị thời gian nếu có, ngược lại là "--:--"
                        char timeStr[8];
                        if (ft && ft->enabled) {
                            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", ft->hour, ft->minute);
                        } else {
                            snprintf(timeStr, sizeof(timeStr), "--:--");
                        }

                        tft.setCursor(160, 50 + i * 30);
                        tft.print(timeStr);

                        tft.setCursor(240, 50 + i * 30);
                        tft.setTextColor(ft && ft->enabled ? ST77XX_GREEN : ST77XX_RED);
                        tft.print(ft && ft->enabled ? "[V]" : "[X]");
                    }
                }


            break;
        }

        case SettingPage::SetHour: {


            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char hourStr[16];
            snprintf(hourStr, sizeof(hourStr), "Hour: %02d", _hour);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(hourStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }

        case SettingPage::SetMinute: {

            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char minStr[16];
            snprintf(minStr, sizeof(minStr), "Minute: %02d", _minute);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(minStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }

        case SettingPage::SetDuration: {


            char label[32];
            snprintf(label, sizeof(label), "Feeding %d/3", _selectedSlot + 1);
            tft.setCursor(20, 20);
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(label);

            char durationStr[32];
            snprintf(durationStr, sizeof(durationStr), "Duration: %d ", _duration);
            tft.setCursor(20, 70);
            tft.setTextSize(4);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(durationStr);

            tft.setTextSize(2);
            tft.setCursor(20, 130);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Rotate to adjust,");

            tft.setTextSize(2);
            tft.setCursor(20, 150);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("Click to confirm");
            break;
        }
        case SettingPage::SetEnabled: {
                tft.setTextSize(2);
                tft.setTextColor(ST77XX_WHITE);
                tft.setCursor(60, 20);
                tft.print("Enable this slot?");  // ✅ Tiêu đề

                const char* options[2] = { "YES", "NO" };
                for (int i = 0; i < 2; i++) {
                    tft.setCursor(60 + i * 160, 100);  // ✅ Giãn cách xa nhau
                    tft.setTextSize(3);
                    tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
                    tft.print(options[i]);
                }

                tft.setTextSize(2);
                tft.setCursor(20, 130);
                tft.setTextColor(ST77XX_WHITE);
                tft.print("Rotate to adjust,");

                tft.setTextSize(2);
                tft.setCursor(20, 150);
                tft.setTextColor(ST77XX_WHITE);
                tft.print("Click to confirm");
                break;
            }

        case SettingPage::ConfirmSave: {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(60, 20);
            tft.print("Save this setting?");

            const char* options[2] = { "YES", "NO" };
            for (int i = 0; i < 2; i++) {
                tft.setCursor(60 + i * 160, 100);
                tft.setTextSize(3);
                tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
                tft.print(options[i]);
            }
            break;
        }

        default:
            break;
    }
}
