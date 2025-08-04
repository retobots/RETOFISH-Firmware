// // File: wakeup.cpp
// // Mục đích: Quản lý việc đưa ESP32 vào deep sleep
// // và tự tính toán thời gian để dậy lại khi đến giờ cho ăn tiếp theo

// #include "hal/wakeup.h"
// #include "hal/RTC.h"
// #include "services/ScheduleManager.h"
// #include <esp_sleep.h>

// #define WAKEUP_BUTTON_PIN GPIO_NUM_19  // D19: Nút nhấn FEEDING_BTN để đánh thức ESP32

// // Tính số giây còn lại từ thời gian hiện tại đến slot cho ăn gần nhất
// int Wakeup::calculateSecondsToNextFeeding() {
//     DateTime now = RTC::getInstance().now();
//     int nowMinutes = now.hour() * 60 + now.minute();

//     int minDeltaMinutes = 24 * 60; // Khởi tạo = 24h (tối đa)
//     for (int i = 0; i < 3; ++i) {
//         const FeedTime* ft = ScheduleManager::getInstance().getSlot(i);
//         if (!ft || !ft->enabled) continue; // Bỏ qua slot tắt

//         int slotMinutes = ft->hour * 60 + ft->minute;
//         int delta = slotMinutes - nowMinutes;
//         if (delta <= 0) delta += 24 * 60; // Giờ đã qua, tính sang ngày mai

//         if (delta < minDeltaMinutes) {
//             minDeltaMinutes = delta; // Giữ khoảng cách ngắn nhất
//         }
//     }

//     int seconds = minDeltaMinutes * 60;
//     Serial.printf("[\u23f1\ufe0f] Sleep duration until next feeding: %d min = %d sec\n", minDeltaMinutes, seconds);
//     return seconds;
// }

// // Gọi ESP32 vào deep sleep trong thời gian còn lại, kèm ngắt bằng nút nhấn
// void Wakeup::goToDeepSleep() {
//     int sleepSecs = calculateSecondsToNextFeeding();

//     // Serial.printf("[\ud83d\udecc] Going to deep sleep for %d seconds...\n", sleepSecs);
//     esp_sleep_enable_timer_wakeup((uint64_t)sleepSecs * 1000000ULL); // Wakeup theo thời gian
//     esp_sleep_enable_ext0_wakeup(WAKEUP_BUTTON_PIN, 0);              // Wakeup khi nhấn nút D19 (LOW)

//     esp_deep_sleep_start(); // Vào deep sleep ngay
// }