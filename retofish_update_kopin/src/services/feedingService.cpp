
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

FeedingService &FeedingService::getInstance()
{
    static FeedingService instance;
    return instance;
}

void FeedingService::setup()
{
    _inStandbyMode = true;
    _screenOn = true;
    _screenOnTime = millis();
    _lastManualFeedTime = millis() - 30000;
    _lastAutoFeedTime = 0;
    Battery::getInstance().update(true); // ✅ cập nhật pin ngay khi khởi động
    updateDisplayAndLed();               // ✅ vẽ trạng thái pin lên màn hình
}

void FeedingService::loop()
{
    Button &btn = Button::getInstance();
    btn.update();
    Button::Event evt = btn.getEvent();
    int delta = btn.getRotationDelta();
    // Battery::getInstance().update(true);
    Battery::getInstance().update(false);

    if (_inSettingMode)
    {
        handleSetting(delta, evt);
        return;
    }

    // --- Hold-to-Feed: giữ 6s để cho ăn, buông là dừng ngay ---
    Button &b = Button::getInstance();
    uint32_t raw = b.getRawPressedDuration(); // 0 nếu đang thả

    // ===== Cấu hình ramp êm =====
    static uint32_t rampStartMs = 0;       // mốc bắt đầu ramp
    static bool rampArmed = false;         // đang ramp?
    static const uint16_t RAMP_MS = 400;   // thời gian ramp ~0.4s
    static const float RPM_START = 2.0f;   // bắt đầu chậm
    static const float RPM_TARGET = 10.0f; // tốc độ mục tiêu

    if (!_inSettingMode && _screenOn && raw >= 6000)
    {
        if (!_holdFeeding)
        {
            _holdFeeding = true;
            _feeding = true;
            _warnSpam = false;

            // UI/LED
            TftDisplay::getInstance().turnOnScreen();
            TftDisplay &display = TftDisplay::getInstance();
            display.clear();
            display.resetLastStatus();
            StatusLed::getInstance().setStatus(StatusLed::State::Feeding);
            updateDisplayAndLed();

            // tốc độ nếu bạn đã thêm setRpm() (Cách B)
            StepperMotor::getInstance().setRpm(5.0f); // tuỳ động cơ (28BYJ-48: 6–12 rpm là ổn)
        }
        StepperMotor::getInstance().feedingLevel(0.3f);
        Serial.println("StepperMotor::getInstance().feedingLevel(0.3f)");
        _feedingStartTime = millis();
    }

    // Nếu trước đó đang hold ≥6s mà bây giờ đã BUÔNG → dừng (kết thúc sau vòng vừa rồi)
    if (_holdFeeding && _lastRawHold >= 6000 && raw == 0)
    {
        Serial.print("_lastRawHold = ");
        Serial.println(_lastRawHold);
        Serial.print("raw = ");
        Serial.println(raw);

        _holdFeeding = false;
        _feeding = false;

        StatusLed::getInstance().setStatus(StatusLed::State::Idle);
        _screenOnTime = millis();
        updateDisplayAndLed();
        Serial.println("[Hold] Feeding STOP (release)");
        ///////////////////////////////////////////////
        StepperMotor::getInstance().disableMotor();
    }

    if (!_inSettingMode && _screenOn && _lastRawHold >= 3000 && _lastRawHold < 6000 && raw == 0)
    {
        TftDisplay &tft = TftDisplay::getInstance();

        Serial.println("Vao che do setting (3s < hold < 6s)");
        _inSettingMode = true;
        _settingPage = SettingPage::NewPage;

        _screenOn = true;
        _screenOnTime = millis();
        _selectedSlot = 0;
        _hour = 7;
        _minute = 0;
        _duration = 1;
        _confirmIndex = 0;

        renderSettingPage();
        // Không return; cho phép phần còn lại của loop chạy tiếp an toàn
    }

    _lastRawHold = raw; // cập nhật cho khung kế tiếp

    handleButton(evt);
    handleAutoFeeding();
    checkScreenTimeout();
    checkFeedingTimeout();
    checkWarningTimeout();
    updateDisplayAndLed();

    delay(20);
}

void FeedingService::handleSetting(int delta, Button::Event evt)
{
    switch (_settingPage)
    {
    case SettingPage::NewPage:
        if (delta != 0)
        {
            _selectedSlot = constrain(_selectedSlot + delta, 0, 2); // 2: back
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_selectedSlot == 0)
            {
                // Chọn "Select Feeding Time" → Quay lại trang SelectSlot
                _settingPage = SettingPage::SelectSlot;
                renderSettingPage();
            }
            else if (_selectedSlot == 1)
            {
                // Chọn "Setting Time Now" → Cài đặt thời gian thực
                _settingPage = SettingPage::SetHour_1;   // Đi tới cài đặt giờ thực
                _hour = RTC::getInstance().now().hour(); // Đặt giờ từ RTC hiện tại
                renderSettingPage();                     // Hiển thị trang chỉnh giờ
            }
            else if (_selectedSlot == 2)
            {
                // Chọn "Back" → Quay lại homepage
                _inSettingMode = false;   //
                _screenOnTime = millis(); //
                TftDisplay &display = TftDisplay::getInstance();
                display.clear();
                // ✅ XÓA CACHE để bắt buộc vẽ lại mọi thứ
                display.resetLastStatus(); // bạn sẽ thêm hàm này ở bước dưới

                updateDisplayAndLed();
            }
        }
        break;

    case SettingPage::SetHour_1:
        if (delta != 0)
        {
            _hour = (_hour + delta + 24) % 24; // Cập nhật giờ khi xoay encoder
            // Không render lại toàn bộ màn hình, chỉ render lại phần giờ thay đổi
            TftDisplay &tft = TftDisplay::getInstance();
            tft.fillRect(160, 70, 100, 30, ST77XX_BLACK); // Xóa phần giờ cũ
            // renderSettingPage(); // Gọi lại để chỉ render phần giờ
            renderNumber(160, 70, _hour, 4, ST77XX_CYAN);
        }
        if (evt == Button::Event::Click)
        {
            // Nhấn để xác nhận giờ và chuyển sang chỉnh phút
            _settingPage = SettingPage::SetMinute_1;
            renderSettingPage();
        }
        break;

    case SettingPage::SetMinute_1:
        if (delta != 0)
        {
            _minute = (_minute + delta + 60) % 60; // Cập nhật phút khi xoay encoder
            TftDisplay &tft = TftDisplay::getInstance();
            tft.fillRect(200, 70, 150, 30, ST77XX_BLACK); // Xóa phần giờ cũ
            // renderSettingPage(); // Gọi lại để chỉ render phần giờ
            renderNumber(200, 70, _minute, 4, ST77XX_CYAN);
        }
        if (evt == Button::Event::Click)
        {
            // Nhấn để xác nhận phút và chuyển đến trang xác nhận lưu
            _settingPage = SettingPage::SetSave;
            renderSettingPage();
        }
        break;

    case SettingPage::SetSave:
        if (delta != 0)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2; // Chọn "Yes" hoặc "No"
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_confirmIndex == 0)
            {
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
        if (delta != 0)
        {
            _selectedSlot = constrain(_selectedSlot + delta, 0, 3); // 0-2: slot, 3: back
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            if (_selectedSlot == 3)
            {

                _settingPage = SettingPage::NewPage;
                renderSettingPage();
            }
            else
            {
                const FeedTime *ft = ScheduleManager::getInstance().getSlot(_selectedSlot);
                if (ft)
                {
                    _hour = ft->hour;
                    _minute = ft->minute;
                    _duration = ft->duration;
                }
                else
                {
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
        if (delta != 0)
        {
            _hour = (_hour + delta + 24) % 24;
            // renderSettingPage();
            TftDisplay &tft = TftDisplay::getInstance();
            tft.fillRect(160, 70, 100, 30, ST77XX_BLACK); // Xóa phần giờ cũ
            // renderSettingPage(); // Gọi lại để chỉ render phần giờ
            renderNumber(160, 70, _hour, 4, ST77XX_CYAN);
        }
        if (evt == Button::Event::Click)
        {
            _settingPage = SettingPage::SetMinute;
            renderSettingPage();
        }
        break;

    case SettingPage::SetMinute:
        if (delta != 0)
        {
            _minute = (_minute + delta + 60) % 60;
            // renderSettingPage();
            TftDisplay &tft = TftDisplay::getInstance();
            tft.fillRect(210, 70, 100, 30, ST77XX_BLACK); // Xóa phần giờ cũ
            // renderSettingPage(); // Gọi lại để chỉ render phần giờ
            renderNumber(210, 70, _minute, 4, ST77XX_CYAN);
        }
        if (evt == Button::Event::Click)
        {
            _settingPage = SettingPage::SetDuration;
            renderSettingPage();
        }
        break;

    case SettingPage::SetDuration:
        if (delta != 0)
        {
            _duration = constrain(_duration + delta, 1, 10);
            // renderSettingPage();
            TftDisplay &tft = TftDisplay::getInstance();
            tft.fillRect(240, 70, 100, 30, ST77XX_BLACK); // Xóa phần giờ cũ
            // renderSettingPage(); // Gọi lại để chỉ render phần giờ
            renderNumber(240, 70, _duration, 4, ST77XX_CYAN);
        }
        if (evt == Button::Event::Click)
        {
            _settingPage = SettingPage::SetEnabled;
            renderSettingPage();
        }
        break;

    case SettingPage::SetEnabled:
        if (delta != 0)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2; // ✅ Xoay chọn YES/NO
            renderSettingPage();
        }
        if (evt == Button::Event::Click)
        {
            _enabled = (_confirmIndex == 0); // ✅ YES → bật, NO → tắt
            _settingPage = SettingPage::ConfirmSave;
            renderSettingPage();
        }
        break;

    case SettingPage::ConfirmSave:
        if (delta != 0)
        {
            _confirmIndex = (_confirmIndex + delta + 2) % 2;
            renderSettingPage();
        }

        if (evt == Button::Event::Click)
        {
            if (_confirmIndex == 0)
            {

                bool en1 = ScheduleManager::getInstance().getSlot(0)->enabled; // Lấy giá trị enabled của slot 1
                bool en2 = ScheduleManager::getInstance().getSlot(1)->enabled; // Lấy giá trị enabled của slot 2
                bool en3 = ScheduleManager::getInstance().getSlot(2)->enabled; // Lấy giá trị enabled của slot 3

                int time1_eps = (ScheduleManager::getInstance().getSlot(0)->hour) * 60 + ScheduleManager::getInstance().getSlot(0)->minute;
                int time2_esp = (ScheduleManager::getInstance().getSlot(1)->hour) * 60 + ScheduleManager::getInstance().getSlot(1)->minute;
                int time3_esp = (ScheduleManager::getInstance().getSlot(2)->hour) * 60 + ScheduleManager::getInstance().getSlot(2)->minute;

                // Lưu Time 1 khi người dùng chọn Slot 0
                if (_selectedSlot == 0)
                {
                    int time1InMinutes = (_hour * 60) + _minute; // Lưu thời gian của Time 1
                    _time1 = time1InMinutes;                     // Lưu vào biến time1
                    Serial.printf("Time 1 saved: %02d:%02d = %d minutes, en1 = %d. en2 = %d en3 = %d\n", _hour, _minute, _time1, en1, en2, en3);
                    Serial.printf("Time difference (abs): %d minutes\n", abs(_time2 - time1InMinutes));
                    Serial.printf("Time difference (abs): %d minutes\n", abs(_time3 - time1InMinutes));
                    if (en2 && abs(time2_esp - time1InMinutes) < 10)
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart  time 2");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }

                    if (en3 && abs(time3_esp - time1InMinutes) < 10)
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart  time 3");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }
                }

                // Kiểm tra Time 2 (Slot 1): Time 2 phải cách Time 1 ít nhất 10 phút
                if (_selectedSlot == 1)
                {                                                // Kiểm tra cho Time 2 (Slot 1)
                    int time2InMinutes = (_hour * 60) + _minute; // Thời gian hiện tại cho Time 2
                    int minTime2 = _time1 + 10;                  // Time 1 + 10 phút (giới hạn cho Time 2)

                    // Kiểm tra nếu en1 khác 0 và time2InMinutes nhỏ hơn minTime2, yêu cầu nhập lại
                    if (en1 && abs(time1_eps - time2InMinutes) < 10)
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart time1 ");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }
                    if (en3 && abs(time3_esp - time2InMinutes) < 10)
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart time 3");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }

                    _time2 = time2InMinutes;                                                                                                   // Lưu thời gian của Time 2 vào _time2
                    Serial.printf("Time 2 saved: %02d:%02d = %d minutes en1 = %d en2 = %d en3 = %d\n", _hour, _minute, _time2, en1, en2, en3); // Log Time 2 lưu
                }

                // Kiểm tra Time 3 (Slot 2): Time 3 phải cách Time 2 ít nhất 10 phút và phải lớn hơn Time 1
                if (_selectedSlot == 2)
                {                                                // Kiểm tra cho Time 3 (Slot 2)
                    int time3InMinutes = (_hour * 60) + _minute; // Thời gian hiện tại cho Time 3
                    int minTime3 = _time2 + 10;                  // Time 2 + 10 phút (giới hạn cho Time 3)
                    Serial.printf("Time difference (abs): %d minutes\n", abs(_time2 - time3InMinutes));
                    // Kiểm tra nếu en2 khác 0 và time3InMinutes nhỏ hơn minTime3, yêu cầu nhập lại
                    if (en2 && (abs(time2_esp - time3InMinutes) < 10))
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart time 2 ");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }

                    // Kiểm tra Time 3 phải lớn hơn Time 1
                    if (en1 && (abs(time1_eps - time3InMinutes) < 10))
                    {
                        TftDisplay &tft = TftDisplay::getInstance();
                        tft.clear();

                        tft.setTextSize(4);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(90, 30);
                        tft.print("ERROR");

                        tft.setTextSize(2);
                        tft.setTextColor(ST77XX_RED);
                        tft.setCursor(10, 70);
                        tft.print("10 minutes apart time 1");
                        delay(2000);
                        _settingPage = SettingPage::SetHour;
                        renderSettingPage();
                        return;
                    }
                    _time3 = time3InMinutes;                                                                                                       // Lưu thời gian của Time 3 vào _time3
                    Serial.printf("Time 3 saved: %02d:%02d = %d minutes \n en1 = %d en2 = %d en3 = %d \n", _hour, _minute, _time3, en1, en2, en3); // Log Time 3 lưu
                }
                Serial.printf("✅ Saved: Slot %d = %02d:%02d for %ds (%s)\n",
                              _selectedSlot + 1, _hour, _minute, _duration, _enabled ? "ENABLED" : "DISABLED");

                // Cập nhật các slot và lưu vào EEPROM
                ScheduleManager::getInstance().updateSlot(_selectedSlot, _hour, _minute, _duration, _enabled);
                ScheduleManager::getInstance().saveToEEPROM();
            }
            else
            {
                Serial.printf("❌ Cancel Save\n");
            }

            _settingPage = SettingPage::SelectSlot;
            renderSettingPage();
        }

        break;
    }
}
void FeedingService::handleButton(Button::Event evt)
{
    unsigned long now = millis();

    // if (Button::getInstance().getRawPressedDuration() >= 3000 && !_inSettingMode) {
    //     if (!_screenOn) {
    //             return;
    //         } else {

    //     TftDisplay &tft = TftDisplay::getInstance();
    //     tft.clear();
    //     delay(50);

    //     Serial.println("Vao che do setting");
    //     _inSettingMode = true;
    //     _settingPage = SettingPage::NewPage;

    //     _screenOn = true;
    //     _screenOnTime = millis();
    //     _selectedSlot = 0;
    //     _hour = 7;
    //     _minute = 0;
    //     _duration = 1;
    //     _confirmIndex = 0;
    //     tft.clear();
    //     delay(50);
    //     renderSettingPage();
    //     return;
    //     }
    // }

    if (evt == Button::Event::Click)
    {
        if (!_screenOn)
        {

            TftDisplay::getInstance().turnOnScreen();
            _inStandbyMode = true;

            _screenOn = true;

            _warnSpam = false;

            _screenOnTime = millis();
            TftDisplay &display = TftDisplay::getInstance();
            display.clear();

            display.resetLastStatus();

            updateDisplayAndLed();
            Serial.println("Screen ON by Button");
        }
    }

    if (evt == Button::Event::DoubleClick)
    {
        if (!_screenOn || _feeding)
        { // Nếu màn hình tắt hoặc đang cho ăn, không làm gì
            return;
        }
        else
        {
            _feeding = true;
            _warnSpam = false;
            updateDisplayAndLed();
            StepperMotor::getInstance().feedingLevel(1.0f);
            StepperMotor::getInstance().disableMotor();
            _feedingStartTime = now;
            _lastManualFeedTime = now;
            _lastAnyFeedTime = now;

            updateDisplayAndLed();
        }
    }
}

void FeedingService::updateDisplayAndLed()
{
    float voltage = Battery::getInstance().readVoltage();
    uint8_t level = Battery::getInstance().getBatteryLevel();

    const char *statusStr;
    StatusLed &led = StatusLed::getInstance();

    if (_warnSpam)
    {
        statusStr = "Wait 30s";
    }
    else if (_feeding)
    {
        led.setStatus(StatusLed::State::Feeding);
        statusStr = "Feeding";
    }
    else
    {
        led.setStatus(StatusLed::State::Idle);
        statusStr = "Standby";
    }

    led.update();

    if (_screenOn)
    {

        DateTime now = RTC::getInstance().now();
        const FeedTime *next = ScheduleManager::getInstance().getNextFeedTime(now);

        if (next)
        {
            int hour12 = next->hour % 12;
            if (hour12 == 0)
                hour12 = 12;
            const char *ampm = next->hour < 12 ? "AM" : "PM";

            char timeStr[16];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", hour12, next->minute, ampm);

            // TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, timeStr, Battery::getInstance().isCharging());
            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, timeStr, false);
        }
        else
        {

            TftDisplay::getInstance().showFullStatus(voltage, level, statusStr, "No setting", Battery::getInstance().isCharging());
        }
    }
}

void FeedingService::handleAutoFeeding()
{
    if (_feeding)
        return;
    DateTime nowRtc = RTC::getInstance().now();
    unsigned long now = millis();

    if (StepperMotor::getInstance().isMotorActive())
    {
        Serial.println("Motor is already active, skipping Auto feed.");
        return; // Nếu động cơ đang quay, bỏ qua mốc auto feeding
    }

    const FeedTime *matchedSlot = nullptr;
    for (int i = 0; i < 3; ++i)
    {
        const FeedTime *s = ScheduleManager::getInstance().getSlot(i);
        if (!s)
            return;
        if (!s->enabled)
            return; // chỉ slot ENABLED
        if (nowRtc.hour() == s->hour &&
            nowRtc.minute() == s->minute &&
            nowRtc.second() < 2)
        {
            matchedSlot = s;
            Serial.printf("[Auto] Feeding START (slot %d | %02d:%02d | %ds)\n",
                          i + 1, matchedSlot->hour, matchedSlot->minute, (float)matchedSlot->duration);
            // 🖥️ Bật màn hình + chuẩn bị UI
            TftDisplay::getInstance().turnOnScreen();
            _feeding = true;

            _screenOn = true;
            _screenOnTime = now;
            TftDisplay &display = TftDisplay::getInstance();
            display.clear();
            display.resetLastStatus();

            // Cập nhật trạng thái hiển thị/LED (ví dụ LED Feeding)
            updateDisplayAndLed();

            // 🚚 Thực thi cho ăn
            StepperMotor::getInstance().feedingLevel((float)matchedSlot->duration);

            // 🕒 Ghi dấu thời gian để chống kích hoạt lại
            _feedingStartTime = now;
            _lastAutoFeedTime = now;
            _lastAnyFeedTime = now;
            Serial.println("Screen ON by Auto");
            break;
        }
    }
}

void FeedingService::checkScreenTimeout()
{
    // Trả về ngay nếu đang trong chế độ cài đặt hoặc đang cho ăn
    if (_inSettingMode || _feeding)
        return;

    // Kiểm tra nếu hệ thống đang ở chế độ "chờ" và đã 15 giây không có hoạt động
    if (_inStandbyMode && _screenOn && (millis() - _screenOnTime > 15000))
    {
        _screenOn = false;                         // Tắt màn hình
        digitalWrite(14, LOW);                     // Tắt đèn nền
        TftDisplay::getInstance().turnOffScreen(); // Tắt màn hình TFT
        _inStandbyMode = false;                    // Kết thúc chế độ "chờ" sau khi tắt màn hình
    }
}
void FeedingService::checkFeedingTimeout()
{
    if (_feeding && !_holdFeeding)
    {
        _feeding = false;
        updateDisplayAndLed(); // Cập nhật trạng thái sau khi cho ăn xong

        // Sau khi cho ăn xong, chuyển vào chế độ "chờ"
        _inStandbyMode = true;    // Đặt _inStandbyMode thành true (vào chế độ "chờ")
        _screenOnTime = millis(); // Ghi lại thời gian vào chế độ "chờ"
    }
}

void FeedingService::checkWarningTimeout()
{
    if (_warnSpam && (millis() - _warnStartTime > 5000))
    {
        _warnSpam = false;
    }
}

void FeedingService::renderSettingPage()
{
    TftDisplay &tft = TftDisplay::getInstance();
    tft.clear();

    switch (_settingPage)
    {

    case SettingPage::NewPage:
    {
        tft.clear();
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(20, 10);
        tft.print("SETUP MENU");
        // Mảng các lựa chọn
        const char *options[] = {
            "Feeding Time", // 0
            "System Time",  // 1
            "Back"          // 2 (thêm "Back" nếu cần)
        };

        // Vẽ các lựa chọn với vòng lặp
        for (int i = 0; i < 3; i++)
        {
            tft.setCursor(20, 50 + i * 30);                                      // Cập nhật vị trí của từng dòng
            tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE); // Dùng màu vàng cho lựa chọn đang được chọn
            tft.print(options[i]);                                               // In lựa chọn ra màn hình
        }

        break;
    }

    case SettingPage::SetHour_1:
    {
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

    case SettingPage::SetMinute_1:
    {
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

    case SettingPage::SetSave:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(60, 20);
        tft.print("Save this setting?");

        const char *options[2] = {"YES", "NO"};
        for (int i = 0; i < 2; i++)
        {
            tft.setCursor(60 + i * 160, 100);
            tft.setTextSize(3);
            tft.setTextColor(i == _confirmIndex ? ST77XX_GREEN : ST77XX_WHITE);
            tft.print(options[i]);
        }
        break;
    }

    case SettingPage::SelectSlot:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(20, 10);
        tft.print("FEEDING SLOT");

        const char *labels[4] = {"Feeding 1/3", "Feeding 2/3", "Feeding 3/3", "BACK"};

        for (int i = 0; i < 4; i++)
        {
            tft.setCursor(20, 50 + i * 30);
            tft.setTextColor(i == _selectedSlot ? ST77XX_YELLOW : ST77XX_WHITE);
            tft.print(labels[i]);

            if (i < 3)
            {
                const FeedTime *ft = ScheduleManager::getInstance().getSlot(i);

                // Hiển thị thời gian nếu có, ngược lại là "--:--"
                char timeStr[8];
                // if (ft && ft->enabled) {
                //     snprintf(timeStr, sizeof(timeStr), "%02d:%02d", ft->hour, ft->minute);
                // } else {
                //     snprintf(timeStr, sizeof(timeStr), "--:--");
                // }
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d", ft->hour, ft->minute);

                tft.setCursor(160, 50 + i * 30);
                tft.print(timeStr);

                tft.setCursor(240, 50 + i * 30);
                tft.setTextColor(ft && ft->enabled ? ST77XX_GREEN : ST77XX_RED);
                tft.print(ft && ft->enabled ? "[V]" : "[X]");
            }
        }

        break;
    }

    case SettingPage::SetHour:
    {

        char label[32];
        snprintf(label, sizeof(label), "SET HOUR %d/3", _selectedSlot + 1);
        tft.setCursor(20, 20);
        tft.setTextSize(2);
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

    case SettingPage::SetMinute:
    {

        char label[32];
        snprintf(label, sizeof(label), "SET MINUTE %d/3", _selectedSlot + 1);
        tft.setCursor(20, 20);
        tft.setTextSize(2);
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

    case SettingPage::SetDuration:
    {

        char label[32];
        snprintf(label, sizeof(label), "SET ROUNDS %d/3", _selectedSlot + 1);
        tft.setCursor(20, 20);
        tft.setTextSize(2);
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
    case SettingPage::SetEnabled:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(60, 20);
        tft.print("Enable this slot?"); // ✅ Tiêu đề

        const char *options[2] = {"YES", "NO"};
        for (int i = 0; i < 2; i++)
        {
            tft.setCursor(60 + i * 160, 100); // ✅ Giãn cách xa nhau
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

    case SettingPage::ConfirmSave:
    {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(80, 20);
        tft.print("Save changes ?");

        const char *options[2] = {"YES", "NO"};
        for (int i = 0; i < 2; i++)
        {
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

void FeedingService::renderNumber(int x, int y, int value, int size, uint16_t color)
{
    // Xóa phần số cũ trước khi in lại (nếu cần thiết)
    TftDisplay &tft = TftDisplay::getInstance();
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d", value); // Chuyển giá trị sang chuỗi (ví dụ: 08, 15)

    tft.setTextSize(size);   // Cài đặt kích thước chữ
    tft.setTextColor(color); // Cài đặt màu chữ
    tft.setCursor(x, y);     // Vị trí của số
    tft.print(buffer);       // In số lên màn hình
}
