

#include "hal/Button.h"
#include <services/UIService.h>

Button &Button::getInstance()
{
    static Button instance;
    return instance;
}

// Bảng mã Gray decoding 4-bit cho encoder
const int8_t encoder_table[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

volatile int Button::_rotationDelta = 0;
volatile int Button::_rotationTotal = 0;
volatile uint8_t Button::_lastAB = 0;

void Button::setup(uint8_t swPin, uint8_t pinA, uint8_t pinB)
{
    _pinSW = swPin;
    _pinA = pinA;
    _pinB = pinB;

    pinMode(_pinSW, INPUT_PULLUP);
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);

    uint8_t a = digitalRead(_pinA);
    uint8_t b = digitalRead(_pinB);
    _lastAB = (a << 1) | b;

    attachInterrupt(digitalPinToInterrupt(_pinA), handleEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pinB), handleEncoderISR, CHANGE);
}

void IRAM_ATTR Button::handleEncoderISR()
{
    uint8_t a = digitalRead(34);
    uint8_t b = digitalRead(35);
    uint8_t ab = (a << 1) | b;

    uint8_t index = (_lastAB << 2) | ab;
    int8_t direction = encoder_table[index & 0x0F];

    if (direction != 0)
    {
        _rotationDelta += direction;
        _rotationTotal += direction;
    }

    _lastAB = ab;
}

void Button::update()
{
    bool currentState = digitalRead(_pinSW);
    // Serial.print("currentState: ");
    // Serial.println(currentState);
    unsigned long now = millis();

    if (_lastState != currentState)
    {
        _lastDebounceTime = now;
        _lastState = currentState;
    }

    if ((now - _lastDebounceTime) > 50)
    {

        if (_lastState == LOW && !_isPressed) // press button
        {
            Serial.println("_pressedTime = now");
            _pressedTime = now;
            _isPressed = true;
        }
        else if (_lastState == LOW && _isPressed)
        {
            unsigned long duration = now - _pressedTime;
            if (duration >= 6000 && _lastEvent == Event::None)
            {
                _lastEvent = Event::HoldFeeding;
            }
        }
        else if (_lastState == HIGH && _isPressed) // release button
        {
            unsigned long duration = now - _pressedTime;

            if (duration < 300)
            {
                if (_waitingDoubleClick && (now - _doubleClickTimer) < 500)
                {
                    _lastEvent = Event::DoubleClick;
                    _waitingDoubleClick = false;
                }
                else
                {
                    _waitingDoubleClick = true;
                    _doubleClickTimer = now;
                }
            }
            else if (duration > 2000 && duration < 6000)
            {
                if (_lastEvent == Event::None)
                {
                    _lastEvent = Event::HoldSetting;
                }
            }
            else if (duration >= 6000)
            {
                if (_lastEvent == Event::HoldFeeding)
                {
                    _lastEvent = Event::None;
                }
            }

            _isPressed = false;
        }
    }

    if (_waitingDoubleClick && (now - _doubleClickTimer) > 500)
    {
        _waitingDoubleClick = false;
        _lastEvent = Event::Click;
    }
}

Button::Event Button::getEvent()
{
    Event e = _lastEvent;
    _lastEvent = Button::Event::None;
    return e;
}

void Button::setEvent(Event e)
{
    _lastEvent = e;
}

int Button::getRotationDelta()
{

    static int remainder = 0;

    noInterrupts();
    int raw = _rotationDelta;
    _rotationDelta = 0;
    interrupts();

    raw += remainder;
    int fullSteps = raw / 4;
    remainder = raw % 4;

    return fullSteps;
}

unsigned long Button::getRawPressedDuration()
{
    unsigned long deltaT;
    if (_isPressed)
    {
        deltaT = millis() - _pressedTime;
        return deltaT;
    }
    else
    {
        // Serial.println("return 0");
        return 0;
    }
}

void Button::handleEvent(Event evt)
{
    unsigned long now = millis();
    TftDisplay *tft = &TftDisplay::getInstance();
    UIService *ui = &UIService::getInstance();
    FeedingService *feed = &FeedingService::getInstance();
    if (evt == Event::Click)
    {
        if (!tft->isScreenON())
        {

            TftDisplay::getInstance().turnOnScreen();
            ui->setScreenOnTime(millis());
            ui->setScreen(UIService::Screen::Home);

            TftDisplay &display = TftDisplay::getInstance();
            display.clear();

            display.resetLastStatus();

            ui->updateHomePage();
            Serial.println("Screen ON by Button");
        }
        else
        {
            UIService::getInstance().setScreenOnTime(millis());
        }
    }
    else if (evt == Event::DoubleClick)
    {
        if (!tft->isScreenON() || feed->isFeeding() || UIService::getInstance().getScreen() != UIService::Screen::Home)
        { // Nếu màn hình tắt hoặc đang cho ăn, không làm gì
            return;
        }
        else
        {
            feed->setFeeding(true);
            ui->setScreen(UIService::Screen::Home);
            ui->updateHomePage();
            feed->feeding(1.0f, true);
            feed->setFeeding(false);
            ui->updateHomePage(); // Cập nhật trạng thái sau khi cho ăn xong

            // Sau khi cho ăn xong, chuyển vào chế độ "chờ"
            // Đặt _inStandbyMode thành true (vào chế độ "chờ")
            ui->setScreenOnTime(millis()); // Ghi lại thời gian vào chế độ "chờ"
        }
    }
    else if (evt == Event::HoldSetting)
    {
        if (!tft->isScreenON() || feed->isFeeding() || UIService::getInstance().getScreen() != UIService::Screen::Home)
        {
            return;
        }
        else
        {
            TftDisplay &tft = TftDisplay::getInstance();

            Serial.println("Vao che do setting (3s < hold < 6s)");
            ui->setScreen(UIService::Screen::Setting);
            ui->setSettingPageState(UIService::SettingPageState::NewPage);
            // Không return; cho phép phần còn lại của loop chạy tiếp an toàn
        }
    }
    else if (evt == Event::HoldFeeding)
    {
        if (!tft->isScreenON() || UIService::getInstance().getScreen() != UIService::Screen::Home)
        {
            return;
        }
        else
        {
            if (!_holdFeeding)
            {
                _holdFeeding = true;
                feed->setFeeding(true);

                // UI/LED
                TftDisplay::getInstance().turnOnScreen();
                TftDisplay &display = TftDisplay::getInstance();
                display.clear();
                display.resetLastStatus();
                StatusLed::getInstance().setStatus(StatusLed::State::Feeding);
                ui->updateHomePage();
            }
            feed->feeding(0.3f, false);
            Serial.println("StepperMotor::getInstance().feedingLevel(0.3f)");
        }
    }
    else if (evt == Event::None)
    {
        if (_holdFeeding)
        {
            _holdFeeding = false;

            StatusLed::getInstance().setStatus(StatusLed::State::Idle);
            Serial.println("[Hold] Feeding STOP (release)");
            ///////////////////////////////////////////////
            feed->feeding(0, false);
            feed->setFeeding(false);
            ui->updateHomePage(); // Cập nhật trạng thái sau khi cho ăn xong

            // Sau khi cho ăn xong, chuyển vào chế độ "chờ"
            ui->setScreen(UIService::Screen::Home); // Đặt _inStandbyMode thành true (vào chế độ "chờ")
            ui->setScreenOnTime(millis());          // Ghi lại thời gian vào chế độ "chờ"
        }
    }
}