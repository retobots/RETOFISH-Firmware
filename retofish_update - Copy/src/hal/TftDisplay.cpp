

#include "hal/TftDisplay.h"
#include "hal/image_bmp_data.h"
#include "hal/RTC.h"


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


// void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin, uint8_t backlightPin) {
//     if (_tft == nullptr) {
//         // Bắt buộc phải khởi tạo SPI đúng chân
//         SPI.begin(18, -1, 23, csPin);  // SCK=18, MISO=-1 (bỏ qua), MOSI=23, CS=csPin

//         _tft = new Adafruit_ST7789(&SPI, csPin, dcPin, rstPin);
//         _tft->init(172, 320);  // kích thước thật màn ST7789 1.47 inch
//         _tft->setRotation(3);
//         _tft->fillScreen(ST77XX_BLACK);

//         // Thiết lập chân đèn nền
//         pinMode(backlightPin, OUTPUT);
//         digitalWrite(backlightPin, HIGH); // Bật đèn nền khi khởi động

//         // Hiển thị logo trong 2 giây
//         drawBMPFromArray(_tft, logo_bmp, logo_bmp_len, 0, 0);
//         delay(2000);
//         _tft->fillScreen(ST77XX_BLACK);
//     }
// }

void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin, uint8_t backlightPin) {
    if (_tft == nullptr) {
        // Bắt buộc phải khởi tạo SPI đúng chân
        SPI.begin(18, -1, 23, csPin);  // SCK=18, MISO=-1 (bỏ qua), MOSI=23, CS=csPin

        _tft = new Adafruit_ST7789(&SPI, csPin, dcPin, rstPin);
        _tft->init(172, 320);  // kích thước thật màn ST7789 1.47 inch
        _tft->setRotation(3);
        _tft->fillScreen(ST77XX_BLACK);

        // Thiết lập chân đèn nền
        _backlightPin = backlightPin;  // Lưu lại giá trị backlightPin
        pinMode(_backlightPin, OUTPUT);
        digitalWrite(_backlightPin, HIGH); // Bật đèn nền khi khởi động

        // Hiển thị logo trong 2 giây
        drawBMPFromArray(_tft, logo_bmp, logo_bmp_len, 0, 0);
        delay(2000);
        _tft->fillScreen(ST77XX_BLACK);
    }
}


void TftDisplay::showFullStatus(float voltage, uint8_t level, const char* status, const char* nextFeedTime, bool charging){

    if (_tft == nullptr) return;

    char line1[64];
    snprintf(line1, sizeof(line1), "Pin:%.2fv %d%%", voltage, level);


        // if (String(line1) != _lastLine1) {
        if (String(line1) != _lastLine1 || charging != _lastCharging) {

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


                if (charging) {
                        int cx = iconX + iconW / 2;
                        int cy = iconY + iconH / 2;

                        // ⚡ Đường chính (to hơn, dài hơn)
                        _tft->drawLine(cx - 5, cy - 10, cx + 1, cy - 4, ST77XX_WHITE);
                        _tft->drawLine(cx + 1, cy - 4, cx - 3, cy + 4, ST77XX_WHITE);
                        _tft->drawLine(cx - 3, cy + 4, cx + 3, cy + 12, ST77XX_WHITE);

                        // ⚡ Đường phụ trái
                        _tft->drawLine(cx - 6, cy - 10, cx - 1, cy - 4, ST77XX_WHITE);
                        _tft->drawLine(cx - 1, cy - 4, cx - 5, cy + 4, ST77XX_WHITE);
                        _tft->drawLine(cx - 5, cy + 4, cx + 1, cy + 12, ST77XX_WHITE);

                        // ⚡ Đường phụ phải
                        _tft->drawLine(cx - 4, cy - 11, cx + 3, cy - 5, ST77XX_WHITE);
                        _tft->drawLine(cx + 3, cy - 5, cx - 1, cy + 2, ST77XX_WHITE);
                        _tft->drawLine(cx - 1, cy + 2, cx + 5, cy + 9, ST77XX_WHITE);
                    }




        _lastLine1 = String(line1);
        _lastCharging = charging;   // ✅ Cập nhật trạng thái sạc
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

    DateTime now = RTC::getInstance().now();

            char timeStr[32];
            snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());

            // Vùng hiển thị: x=10, y=150, size=3 → khoảng cao ~24px, rộng ~200px (tùy font)
            if (String(timeStr) != _lastNextFeed) {
                _tft->fillRect(0, 150, 320, 30, ST77XX_BLACK);  // 🔁 Xóa sạch vùng hiển thị cũ

                _tft->setTextSize(3);
                _tft->setTextColor(ST77XX_WHITE);
                _tft->setCursor(10, 150);
                _tft->print(timeStr);

                _lastNextFeed = String(timeStr);
            }


}

// void TftDisplay::turnOff() {
//     if (_tft) {
//         _tft->fillScreen(ST77XX_BLACK);
//         _lastLine1 = "";
//         _lastStatus = "";
//         _lastNextFeed = "";
//     }
// }

// === Các hàm tiện ích để vẽ trong chế độ cấu hình ===
void TftDisplay::clear() {
    if (_tft) _tft->fillScreen(ST77XX_BLACK);
}

void TftDisplay::setCursor(int16_t x, int16_t y) {
    if (_tft) _tft->setCursor(x, y);
}

void TftDisplay::setTextColor(uint16_t color) {
    if (_tft) _tft->setTextColor(color);
}

void TftDisplay::setTextSize(uint8_t size) {
    if (_tft) _tft->setTextSize(size);
}

void TftDisplay::print(const char* str) {
    if (_tft) _tft->print(str);
}

void TftDisplay::resetLastStatus() {
    _lastLine1 = "";
    _lastStatus = "";
    _lastNextFeed = "";
}

void TftDisplay::turnOffScreen() {
    digitalWrite(_backlightPin, LOW);  // Tắt đèn nền
    _screenOn = false;
    Serial.println("Screen OFF");
}

void TftDisplay::turnOnScreen() {
   
    if (_backlightPin != 0) {
        digitalWrite(_backlightPin, HIGH);  // Bật đèn nền
        Serial.println("Backlight turned ON");
    } else {
        Serial.println("Error: Backlight pin not set!");
    }
    // _screenOn = true;
    // _screenOnTime = millis();
    // Serial.println("Screen ON");

    // Optionally clear screen
    _tft->fillScreen(ST77XX_BLACK);
}
