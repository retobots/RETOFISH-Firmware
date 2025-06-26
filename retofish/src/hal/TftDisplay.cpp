
#include "hal/TftDisplay.h"
#include "hal/moi_bmp.h"
TftDisplay& TftDisplay::getInstance() {
    static TftDisplay instance;
    return instance;
}

void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin) {
    if (_tft == nullptr) {
        _tft = new Adafruit_ST7789(csPin, dcPin, rstPin);

        _tft->init(172, 320);
        _tft->setRotation(3);
        _tft->fillScreen(ST77XX_BLACK);

        _tft->setTextSize(2);
        _tft->setTextColor(ST77XX_WHITE);
        _tft->setCursor(20, 80);
        _tft->print("RETOFISH Booting...");
        delay(1000);
        _tft->fillScreen(ST77XX_BLACK);
    }
}
// void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin) {
//     if (_tft == nullptr) {
//         _tft = new Adafruit_ST7789(csPin, dcPin, rstPin);

//         _tft->init(172, 320);
//         _tft->setRotation(3);
//         _tft->fillScreen(ST77XX_BLACK);

//         // Hiển thị logo trong 2 giây
//         _tft->startWrite();
//         _tft->setAddrWindow(0, 0, 172, 320);
//         for (int i = 0; i < 172 * 320; i++) {
//             _tft->pushColor(moi_bmp[i]);
//         }
//         _tft->endWrite();

//         delay(2000);
//         _tft->fillScreen(ST77XX_BLACK);
//     }
// }
void TftDisplay::showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime) {
    if (_tft == nullptr) return;

    // --- Dòng 1: Pin ---
    char line1[64];
    snprintf(line1, sizeof(line1), "Pin:%.2fv %d%%", voltage, level);

    if (String(line1) != _lastLine1) {
        // Xoá vùng cũ dòng 1
        _tft->fillRect(0, 20, 320, 30, ST77XX_BLACK);
        _tft->setTextSize(3);
        _tft->setTextColor(ST77XX_CYAN);
        _tft->setCursor(10, 20);
        _tft->print(line1);

        // Vẽ icon pin
        int iconX = 260;
        int iconY = 20;
        int iconW = 40;
        int iconH = 20;

        _tft->drawRect(iconX, iconY, iconW, iconH, ST77XX_WHITE);
        _tft->fillRect(iconX + iconW, iconY + 5, 5, 10, ST77XX_WHITE);

        int fillW = map(level, 0, 100, 0, iconW - 4);
        _tft->fillRect(iconX + 2, iconY + 2, fillW, iconH - 4, (level < 15) ? ST77XX_RED : ST77XX_GREEN);

        _lastLine1 = String(line1);
    }

    // --- Dòng 2: Status ---
    if (String(status) != _lastStatus) {
        _tft->fillRect(0, 80, 320, 40, ST77XX_BLACK);
        _tft->setTextSize(4);
        _tft->setTextColor(ST77XX_YELLOW);

        int16_t x1, y1;
        uint16_t w, h;
        _tft->getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

        int cursorX = (320 - w) / 2;
        int cursorY = 80;

        _tft->setCursor(cursorX, cursorY);
        _tft->print(status);

        _lastStatus = String(status);
    }

    // --- Dòng 3: Next feed ---
    if (String(nextFeedTime) != _lastNextFeed) {
        _tft->fillRect(0, 160, 320, 30, ST77XX_BLACK);
        _tft->setTextSize(3);
        _tft->setTextColor(ST77XX_WHITE);
        _tft->setCursor(10, 150);
        _tft->print("Next : ");
        _tft->print(nextFeedTime);

        _lastNextFeed = String(nextFeedTime);
    }
}
void TftDisplay::turnOff() {
    if (_tft) {
        _tft->fillScreen(ST77XX_BLACK);  // Tắt hẳn màn
                // Reset cache → lần sau showFullStatus vẽ lại đủ 3 dòng
        _lastLine1 = "";
        _lastStatus = "";
        _lastNextFeed = "";
    }
}

