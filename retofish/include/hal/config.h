#pragma once
#include <Arduino.h>
#include <Adafruit_ST7789.h>

// -------------------------
// Pin map & tunables
// -------------------------
namespace Pins
{
    // Status LED RGB
    constexpr uint8_t LED_R = 12;
    constexpr uint8_t LED_G = 13;
    constexpr uint8_t LED_B = 15;

    // Rotary encoder + SW
    constexpr uint8_t BTN_SW = 19;
    constexpr uint8_t ENC_A = 34;
    constexpr uint8_t ENC_B = 35;

    // TFT (ví dụ: ST7735 / SPI)
    constexpr uint8_t TFT_CS = 5;
    constexpr uint8_t TFT_DC = 2;
    constexpr uint8_t TFT_RST = 4;
    constexpr uint8_t TFT_BLK = 14; // backlight

    // I2C device
    constexpr uint8_t SDA = 21;
    constexpr uint8_t SCL = 22;
}

// Nhịp cập nhật (cooperative scheduling)
namespace Cadence
{
    constexpr uint32_t UI_MS = 50;          // ~20 Hz
    constexpr uint32_t AUTO_FEED_MS = 200;  // nhẹ nhàng, không cần mỗi vòng
    constexpr uint32_t HOUSEKEEP_MS = 1000; // log/nháy led heartbeat...
}

// ========= Tunables =========
namespace BtnCfg
{
    constexpr uint32_t DEBOUNCE_MS = 50;
    constexpr uint32_t DOUBLECLICK_MS = 500;
    constexpr uint32_t HOLD_SETTING_MIN = 2000; // 2s
    constexpr uint32_t HOLD_FEEDING_MIN = 6000; // 6s
    constexpr int STEPS_PER_DETENT = 4;         // encoder resolution
}

// ---------- Tunables ----------
namespace UiCfg
{
    constexpr uint32_t SCREEN_TIMEOUT_MS = 15000; // F04: tự tắt sau 15s
    constexpr uint16_t COLOR_TITLE = ST77XX_YELLOW;
    constexpr uint16_t COLOR_HINT = ST77XX_WHITE;
    constexpr uint16_t COLOR_VALUE = ST77XX_CYAN;
    constexpr uint16_t COLOR_OK = ST77XX_GREEN;
    constexpr uint16_t COLOR_ERR = ST77XX_RED;
    constexpr uint32_t ERROR_SHOW_MS = 3000; // F05: lỗi hiển thị 3 giây
}

namespace FeedCfg
{
    // Chống kích hoạt lại trong một khoảng ngắn (ms)
    constexpr uint32_t AUTOTRIGGER_COOLDOWN_MS = 5000;
    // Cửa sổ kích hoạt theo giây tại phút đúng lịch (ví dụ < 2s đầu phút)
    constexpr uint8_t TRIGGER_WINDOW_SECONDS = 2;
}

namespace StepperCfg
{
    constexpr uint8_t MOTOR_SPEED = 5;
    constexpr float FEEDING_SCALE = 0.1f;
    constexpr uint8_t MOTOR_MAX_SPEED = 10;
    constexpr uint8_t MOTOR_MIN_SPEED = 3;
}