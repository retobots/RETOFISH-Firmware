
#include "hal/TftDisplay.h"
#include "hal/image_bmp_data.h"

TftDisplay& TftDisplay::getInstance() {
    static TftDisplay instance;
    return instance;
}

// Hàm vẽ BMP từ mảng byte (chuẩn BMP 24-bit)
void drawBMPFromArray(Adafruit_ST7789* tft, const unsigned char *bmp, uint32_t bmp_len, int x, int y) {
    uint32_t dataOffset = bmp[10] | (bmp[11] << 8) | (bmp[12] << 16) | (bmp[13] << 24);
    uint32_t width = bmp[18] | (bmp[19] << 8);
    uint32_t height = bmp[22] | (bmp[23] << 8);
    uint16_t bitDepth = bmp[28];
    if (bitDepth != 24) return;

    bool flip = true;
    int rowSize = (width * 3 + 3) & ~3;

    for (uint32_t row = 0; row < height; row++) {
        uint32_t bmpY = flip ? (height - 1 - row) : row;
        uint32_t rowStart = dataOffset + bmpY * rowSize;
        for (uint32_t col = 0; col < width; col++) {
            uint32_t pixelIndex = rowStart + col * 3;
            if (pixelIndex + 2 >= bmp_len) continue;
            uint8_t b = bmp[pixelIndex];
            uint8_t g = bmp[pixelIndex + 1];
            uint8_t r = bmp[pixelIndex + 2];
            uint16_t color = tft->color565(r, g, b);
            tft->drawPixel(x + col, y + row, color);
        }
    }
}

void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin) {
    if (_tft == nullptr) {
        _tft = new Adafruit_ST7789(csPin, dcPin, rstPin);

        _tft->init(172, 320);
        _tft->setRotation(3);
        _tft->fillScreen(ST77XX_BLACK);

        // ✅ Hiển thị logo trong 2 giây 
        drawBMPFromArray(_tft, logo_bmp, logo_bmp_len, 0, 0);
        delay(2000);

        _tft->fillScreen(ST77XX_BLACK);
    }
}

void TftDisplay::showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime) {
    if (_tft == nullptr) return;

    char line1[64];
    snprintf(line1, sizeof(line1), "Pin:%.2fv %d%%", voltage, level);

    if (String(line1) != _lastLine1) {
        _tft->fillRect(0, 20, 320, 30, ST77XX_BLACK);
        _tft->setTextSize(3);
        _tft->setTextColor(ST77XX_CYAN);
        _tft->setCursor(10, 20);
        _tft->print(line1);

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
        _tft->fillScreen(ST77XX_BLACK);
        _lastLine1 = "";
        _lastStatus = "";
        _lastNextFeed = "";
    }
}

