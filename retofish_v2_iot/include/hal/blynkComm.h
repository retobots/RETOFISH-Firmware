#pragma once

class BlynkComm {
public:
    static BlynkComm& getInstance();

    void setup();
    void loop();

    // Gửi trạng thái lên Blynk (VD: Feeding, Low Battery...)
    void reportStatus(const char* status, const char* nextFeed);

    // Gửi thông tin lịch ăn (3 slot) lên Blynk
    void syncAllSlots();

private:
    BlynkComm() = default;
};

// ===== Các hàm xử lý riêng cho nội bộ =====
void handleSlotToggle(int index, int value);
void updateSlotFromBlynk(int index, int hour, int minute, int duration);
