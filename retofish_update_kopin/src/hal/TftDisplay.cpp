

#include "hal/TftDisplay.h"
#include "hal/image_bmp_data.h"
#include "hal/RTC.h"

TftDisplay &TftDisplay::getInstance()
{
    static TftDisplay instance;
    return instance;
}

// Hàm vẽ BMP từ mảng byte (chuẩn BMP 24-bit)
void drawBMPFromArray(Adafruit_ST7789 *tft, const unsigned char *bmp, uint32_t bmp_len, int x, int y)
{
    uint32_t dataOffset = bmp[10] | (bmp[11] << 8) | (bmp[12] << 16) | (bmp[13] << 24);
    uint32_t width = bmp[18] | (bmp[19] << 8);
    uint32_t height = bmp[22] | (bmp[23] << 8);
    uint16_t bitDepth = bmp[28];
    if (bitDepth != 24)
        return;

    bool flip = true;
    int rowSize = (width * 3 + 3) & ~3;

    for (uint32_t row = 0; row < height; row++)
    {
        uint32_t bmpY = flip ? (height - 1 - row) : row;
        uint32_t rowStart = dataOffset + bmpY * rowSize;
        for (uint32_t col = 0; col < width; col++)
        {
            uint32_t pixelIndex = rowStart + col * 3;
            if (pixelIndex + 2 >= bmp_len)
                continue;
            uint8_t b = bmp[pixelIndex];
            uint8_t g = bmp[pixelIndex + 1];
            uint8_t r = bmp[pixelIndex + 2];
            uint16_t color = tft->color565(r, g, b);
            tft->drawPixel(x + col, y + row, color);
        }
    }
}

bool TftDisplay::isScreenON()
{
    return _screenOn;
}

void TftDisplay::setup(uint8_t csPin, uint8_t dcPin, uint8_t rstPin, uint8_t backlightPin)
{
    if (_tft == nullptr)
    {
        // Bắt buộc phải khởi tạo SPI đúng chân
        SPI.begin(18, -1, 23, csPin); // SCK=18, MISO=-1 (bỏ qua), MOSI=23, CS=csPin

        _tft = new Adafruit_ST7789(&SPI, csPin, dcPin, rstPin);
        _tft->init(TFT_WIDTH, TFT_HEIGHT); // kích thước thật màn ST7789 1.47 inch
        _tft->setRotation(3);
        _tft->fillScreen(ST77XX_BLACK);

        // Thiết lập chân đèn nền
        _backlightPin = backlightPin; // Lưu lại giá trị backlightPin
        pinMode(_backlightPin, OUTPUT);
        digitalWrite(_backlightPin, HIGH); // Bật đèn nền khi khởi động

        // Hiển thị logo trong 2 giây
        drawBMPFromArray(_tft, logo_bmp, logo_bmp_len, 0, 0);
        delay(2000);
        _tft->fillScreen(ST77XX_BLACK);
        _screenOn = true;
    }
}

void TftDisplay::showFullStatus(const char *status, const char *nextFeedTime)
{
    if (_tft == nullptr)
        return;

    // Kiểm tra và cập nhật phần status nếu có sự thay đổi
    if (String(status) != _lastStatus)
    {
        _tft->fillRect(0, 40, 320, 60, ST77XX_BLACK); // Xóa phần status cũ
        _tft->setTextSize(6);
        _tft->setTextColor(ST77XX_YELLOW);

        int16_t x1, y1;
        uint16_t w, h;
        _tft->getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

        int cursorX = (320 - w) / 2;
        int cursorY = 40;

        _tft->setCursor(cursorX, cursorY);
        _tft->print(status);

        _lastStatus = String(status);
    }

    // Hiển thị thời gian thực tế, ẩn giây khi đang trong trạng thái "Feeding"
    DateTime now = RTC::getInstance().now();
    char timeStr[32];

    if (String(status) == "Feeding")
    { // Kiểm tra trạng thái "Feeding"
        // Nếu đang cho ăn, chỉ hiển thị giờ và phút, không hiển thị giây
        snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d", now.hour(), now.minute());
    }
    else
    {
        // Hiển thị đầy đủ thời gian (giờ, phút, giây)
        snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
    }

    // Kiểm tra và cập nhật phần thời gian nếu có sự thay đổi
    if (String(timeStr) != _lastNextFeed)
    {
        _tft->fillRect(0, 150, 320, 30, ST77XX_BLACK); // Xóa phần thời gian cũ
        _tft->setTextSize(3);
        _tft->setTextColor(ST77XX_WHITE);
        _tft->setCursor(20, 150);
        _tft->print(timeStr);
        _lastNextFeed = String(timeStr);
    }
    _screenOn = true;
}

// === Các hàm tiện ích để vẽ trong chế độ cấu hình ===
void TftDisplay::clear()
{
    // Khi vẽ:
    if (_tft)
    {
        _tft->fillScreen(ST77XX_BLACK);
    }
    // ... các lệnh vẽ khác
}

void TftDisplay::setCursor(int16_t x, int16_t y)
{
    if (_tft)
        _tft->setCursor(x, y);
}

void TftDisplay::setTextColor(uint16_t color)
{
    if (_tft)
        _tft->setTextColor(color);
}

void TftDisplay::setTextSize(uint8_t size)
{
    if (_tft)
        _tft->setTextSize(size);
}

void TftDisplay::print(const char *str)
{
    if (_tft)
        _tft->print(str);
}

void TftDisplay::resetLastStatus()
{
    _lastLine1 = "";
    _lastStatus = "";
    _lastNextFeed = "";
}

void TftDisplay::turnOffScreen()
{
    digitalWrite(_backlightPin, LOW); // Tắt đèn nền
    _screenOn = false;
    // Serial.println("Screen OFF");
}

void TftDisplay::turnOnScreen()
{

    if (_backlightPin != 0)
    {
        digitalWrite(_backlightPin, HIGH); // Bật đèn nền
        Serial.println("Backlight turned ON");
        _screenOn = true;
    }
    else
    {
        Serial.println("Error: Backlight pin not set!");
    }
    
    _tft->fillScreen(ST77XX_BLACK);
}

void TftDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (_tft)
    {
        _tft->fillRect(x, y, w, h, color); // Gọi phương thức fillRect từ thư viện Adafruit_ST7789
    }
}

void TftDisplay::setScreen(bool s)
{
    _screenOn = s;
}