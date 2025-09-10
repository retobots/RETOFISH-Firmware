#include "hal/Button.h"
#include "services/UIService.h"

// ========= Internal data =========
static constexpr int8_t kGrayStep[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

// ESP32 critical section cho vùng ISR<->loop (optional nhưng “đẹp”)
static portMUX_TYPE s_isrMux = portMUX_INITIALIZER_UNLOCKED;

// Pins cho ISR đọc theo cấu hình runtime
static uint8_t s_pinA = 0;
static uint8_t s_pinB = 0;

// Shared giữa ISR và loop
volatile static int _rotationDelta = 0;
volatile static int _rotationTotal = 0;
volatile static uint8_t _lastAB = 0;

Button &Button::getInstance()
{
    static Button instance;
    return instance;
}

void Button::setup(uint8_t swPin, uint8_t pinA, uint8_t pinB)
{
    _pinSW = swPin;
    _pinA = pinA;
    _pinB = pinB;
    s_pinA = pinA;
    s_pinB = pinB;

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
    const uint8_t a = digitalRead(s_pinA);
    const uint8_t b = digitalRead(s_pinB);
    const uint8_t ab = (a << 1) | b;

    const uint8_t idx = (_lastAB << 2) | ab;
    const int8_t dir = kGrayStep[idx & 0x0F];

    if (dir != 0)
    {
        portENTER_CRITICAL_ISR(&s_isrMux);
        _rotationDelta += dir;
        _rotationTotal += dir;
        portEXIT_CRITICAL_ISR(&s_isrMux);
    }
    _lastAB = ab;
}

void Button::update()
{
    const bool currentState = digitalRead(_pinSW);
    const unsigned long now = millis();

    if (_lastState != currentState)
    {
        _lastDebounceTime = now;
        _lastState = currentState;
    }

    if ((now - _lastDebounceTime) > BtnCfg::DEBOUNCE_MS)
    {
        if (_lastState == LOW && !_isPressed)
        {
            _pressedTime = now;
            _isPressed = true;
        }
        else if (_lastState == LOW && _isPressed)
        {
            const unsigned long duration = now - _pressedTime;
            if (duration >= BtnCfg::HOLD_FEEDING_MIN && _lastEvent == Event::None)
            {
                _lastEvent = Event::HoldFeeding;
            }
        }
        else if (_lastState == HIGH && _isPressed)
        {
            const unsigned long duration = now - _pressedTime;

            if (duration < 300)
            {
                if (_waitingDoubleClick && (now - _doubleClickTimer) < BtnCfg::DOUBLECLICK_MS)
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
            else if (duration > BtnCfg::HOLD_SETTING_MIN && duration < BtnCfg::HOLD_FEEDING_MIN)
            {
                if (_lastEvent == Event::None)
                    _lastEvent = Event::HoldSetting;
            }
            else if (duration >= BtnCfg::HOLD_FEEDING_MIN)
            {
                if (_holdFeeding)
                    _lastEvent = Event::StopHoldFeeding;
            }
            _isPressed = false;
        }
    }

    if (_waitingDoubleClick && (now - _doubleClickTimer) > BtnCfg::DOUBLECLICK_MS)
    {
        _waitingDoubleClick = false;
        _lastEvent = Event::Click;
    }
}

Button::Event Button::getEvent()
{
    Event e = _lastEvent;
    _lastEvent = Event::None;
    return e;
}

void Button::setEvent(Event e)
{
    _lastEvent = e;
}

int Button::getRotationDelta()
{
    static int remainder = 0;

    portENTER_CRITICAL(&s_isrMux);
    int raw = _rotationDelta;
    _rotationDelta = 0;
    portEXIT_CRITICAL(&s_isrMux);

    raw += remainder;
    const int steps = raw / BtnCfg::STEPS_PER_DETENT;
    remainder = raw % BtnCfg::STEPS_PER_DETENT;
    return steps;
}

unsigned long Button::getRawPressedDuration()
{
    if (_isPressed)
        return millis() - _pressedTime;
    return 0;
}

void Button::handleEvent(Event evt)
{
    auto &tft = TftDisplay::getInstance();
    auto &ui = UIService::getInstance();
    auto &feed = FeedingService::getInstance();
    auto &led = StatusLed::getInstance();

    const bool screenOn = tft.isScreenON();
    const bool isHome = (ui.getScreen() == UIService::Screen::Home);
    const bool feedingOn = feed.isFeeding();

    switch (evt)
    {
    case Event::Click:
        if (!screenOn)
        {
            tft.turnOnScreen();
            ui.setScreenOnTime(millis());
            ui.setScreen(UIService::Screen::Home);
            tft.clear();
            tft.resetLastStatus();
            ui.updateHomePage();
            Serial.println("Screen ON by Button");
        }
        else
        {
            ui.setScreenOnTime(millis());
        }
        break;

    case Event::DoubleClick:
        if (!screenOn || feedingOn || !isHome)
            break;
        feed.setFeeding(true);
        ui.setScreen(UIService::Screen::Home);
        ui.updateHomePage();
        feed.feeding(1.0f, true);
        feed.setFeeding(false);
        ui.updateHomePage();
        ui.setScreenOnTime(millis());
        break;

    case Event::HoldSetting:
        if (!screenOn || feedingOn || !isHome)
            break;
        Serial.println("Vao che do setting (3s < hold < 6s)");
        ui.setScreen(UIService::Screen::Setting);
        ui.setSettingPageState(UIService::SettingPageState::NewPage);
        break;

    case Event::HoldFeeding:
        if (!screenOn || !isHome)
            break;
        if (!_holdFeeding)
        {
            _holdFeeding = true;
            feed.setFeeding(true);
            tft.turnOnScreen();
            tft.clear();
            tft.resetLastStatus();
            led.updateLed(StatusLed::State::Feeding);
            ui.updateHomePage();
        }
        feed.feeding(0.3f, false);
        // hạn chế Serial ở path nhanh
        break;

    case Event::StopHoldFeeding:
    case Event::None:
        if (_holdFeeding)
        {
            _holdFeeding = false;
            led.updateLed(StatusLed::State::Idle);
            Serial.println("[Hold] Feeding STOP (release)");
            feed.feeding(0, false);
            feed.setFeeding(false);
            ui.updateHomePage();
            ui.setScreen(UIService::Screen::Home);
            ui.setScreenOnTime(millis());
        }
        break;
    }
}
