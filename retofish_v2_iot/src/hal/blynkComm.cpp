#define BLYNK_TEMPLATE_ID "TMPL6bsTRiJgX"
#define BLYNK_TEMPLATE_NAME "retofishv2"
#define BLYNK_AUTH_TOKEN "1bGCezQgx1UEjgDeXD3S4kYwv_2Y6fvI"

#include <WiFiManager.h>
#include <BlynkSimpleEsp32.h>
#include "hal/BlynkComm.h"
#include "hal/Battery.h"
#include "hal/StepperMotor.h"
#include "services/ScheduleManager.h"

char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

BlynkComm& BlynkComm::getInstance() {
    static BlynkComm instance;
    return instance;
}

void BlynkComm::setup() {
    WiFiManager wm;
    bool res = wm.autoConnect("RetoFish", "12345678");
    if (!res) {
        Serial.println("⚠️ Không kết nối Wi-Fi được. Reset lại.");
        delay(3000);
        ESP.restart();
    }

    Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());

    // Cập nhật pin mỗi 5s
    timer.setInterval(5000L, []() {
        auto& battery = Battery::getInstance();
        float voltage = battery.readVoltage();
        uint8_t level = battery.getBatteryLevel();
        bool charging = battery.isCharging();

        Blynk.virtualWrite(V0, level);               // % pin
        Blynk.virtualWrite(V4, charging ? 255 : 0);  // sạc
        Blynk.virtualWrite(V5, voltage);             // volt
    });

    // Gửi thông tin slot sau 3s khi khởi động
    timer.setTimeout(3000L, []() {
        BlynkComm::getInstance().syncAllSlots();
    });
}

void BlynkComm::loop() {
    Blynk.run();
    timer.run();
}

void BlynkComm::reportStatus(const char* status, const char* nextFeed) {
    Blynk.virtualWrite(V1, status);    // trạng thái hiện tại
    Blynk.virtualWrite(V2, nextFeed);  // giờ ăn tiếp theo
}

void BlynkComm::syncAllSlots() {
    for (int i = 0; i < 3; ++i) {
        const FeedTime* ft = ScheduleManager::getInstance().getSlot(i);
        if (!ft) continue;

        int pinInfo = V6 + i;
        int pinSwitch = V9 + i;

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%02d:%02d (%d vòng)", ft->hour, ft->minute, ft->duration);
        Blynk.virtualWrite(pinInfo, buffer);
        Blynk.virtualWrite(pinSwitch, ft->enabled ? 1 : 0);
    }
}

// ====== Feed Now (V3) ======
BLYNK_WRITE(V3) {
    static unsigned long lastFeed = 0;
    unsigned long now = millis();
    if (now - lastFeed >= 30000) {
        Serial.println("[Blynk] Feed now (4 rounds)");
        StepperMotor::getInstance().feedForRounds(4);
        lastFeed = now;
    } else {
        Serial.println("[Blynk] Too soon to feed again.");
    }
}

// ====== Toggle Enable Slot ======
BLYNK_WRITE(V9)  { handleSlotToggle(0, param.asInt()); }
BLYNK_WRITE(V10) { handleSlotToggle(1, param.asInt()); }
BLYNK_WRITE(V11) { handleSlotToggle(2, param.asInt()); }

void handleSlotToggle(int index, int value) {
    const FeedTime* ft = ScheduleManager::getInstance().getSlot(index);
    if (!ft) return;

    Serial.printf("[Blynk] Toggle slot %d → %s\n", index + 1, value ? "ENABLED" : "DISABLED");

    ScheduleManager::getInstance().updateSlot(index, ft->hour, ft->minute, ft->duration, value);
    ScheduleManager::getInstance().saveToEEPROM();
    BlynkComm::getInstance().syncAllSlots();
}

// ====== Update Slot Time from App ======

void updateSlotFromBlynk(int index, int hour, int minute, int duration) {
    if (index < 0 || index > 2) return;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || duration < 4 || duration > 10) {
        Serial.println("❌ Dữ liệu không hợp lệ, bỏ qua");
        return;
    }

    const FeedTime* ft = ScheduleManager::getInstance().getSlot(index);
    if (!ft) return;

    bool enabled = ft->enabled;

    Serial.printf("🛠️ [Blynk] Update slot %d → %02d:%02d - %d vòng\n", index + 1, hour, minute, duration);
    ScheduleManager::getInstance().updateSlot(index, hour, minute, duration, enabled);
    ScheduleManager::getInstance().saveToEEPROM();

    BlynkComm::getInstance().syncAllSlots();
}

// === SLOT 1: V20-V21-V22 ===
int slot1_hour = 7, slot1_min = 0, slot1_dur = 10;
BLYNK_WRITE(V20) { slot1_hour = param.asInt(); }
BLYNK_WRITE(V21) { slot1_min  = param.asInt(); }
BLYNK_WRITE(V22) {
    slot1_dur = param.asInt();
    updateSlotFromBlynk(0, slot1_hour, slot1_min, slot1_dur);
}

// === SLOT 2: V23-V24-V25 ===
int slot2_hour = 11, slot2_min = 30, slot2_dur = 10;
BLYNK_WRITE(V23) { slot2_hour = param.asInt(); }
BLYNK_WRITE(V24) { slot2_min  = param.asInt(); }
BLYNK_WRITE(V25) {
    slot2_dur = param.asInt();
    updateSlotFromBlynk(1, slot2_hour, slot2_min, slot2_dur);
}

// === SLOT 3: V26-V27-V28 ===
int slot3_hour = 18, slot3_min = 0, slot3_dur = 10;
BLYNK_WRITE(V26) { slot3_hour = param.asInt(); }
BLYNK_WRITE(V27) { slot3_min  = param.asInt(); }
BLYNK_WRITE(V28) {
    slot3_dur = param.asInt();
    updateSlotFromBlynk(2, slot3_hour, slot3_min, slot3_dur);
}
