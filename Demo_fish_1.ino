#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6eiTozC8L"
#define BLYNK_TEMPLATE_NAME "Retrofish"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h" // Thư viện chuẩn của ESP-IDF cho NTP và thời gian
#include "esp_sntp.h" // Thư viện cụ thể cho SNTP trên ESP-IDF
#include <ESP32Servo.h> // Thư viện Servo cho ESP32

// --- Cấu hình Blynk ---
char auth[] = "6CDIgIpP4P3tF-ip8V4OdDa5fY9OIYIx";
char blynk_server[] = "blynk.cloud";
uint16_t blynk_port = 80;

// --- Cấu hình WiFi ---
const char *ssid = "Minh Thuan 2.4G";
const char *password = "31214800";

// --- Cấu hình NTP (Giờ Việt Nam - GMT+7) ---
const long gmtOffset_sec = 25200; // GMT+7 (7 * 3600 = 25200 giây)
const int daylightOffset_sec = 0; // Việt Nam không có giờ tiết kiệm ánh sáng ban ngày

const char *ntpServer1 = "pool.ntp.org"; // Máy chủ NTP chính
const char *ntpServer2 = "time.nist.gov"; // Máy chủ NTP dự phòng

// --- Cấu hình Servo ---
Servo myServo;
const int servoPin = 13; // Chân GPIO kết nối với Servo

// --- Biến cho thời gian chạy Servo (cho V0 - kích hoạt thủ công) ---
// Biến này chỉ dùng cho trường hợp kích hoạt thủ công (V0),
// Servo sẽ chạy 10 giây rồi dừng.
unsigned long servoManualActivationStartTime = 0;
const long servoManualRunDuration = 10000; // 10 giây
bool servoManualRunning = false; // Cờ báo hiệu Servo có đang chạy do V0 không

// --- Biến cho lịch trình BẬT/TẮT từ Blynk V1 ---
// Lưu trữ thời gian bắt đầu và kết thúc (tổng số giây từ nửa đêm)
long scheduleStartTime = 999999; // Thời gian bắt đầu lịch trình (mặc định là vô hiệu hóa)
long scheduleEndTime = 999999;   // Thời gian kết thúc lịch trình (mặc định là vô hiệu hóa)
bool isScheduleSetV1 = false; // Cờ báo hiệu đã nhận được lịch trình từ Blynk V1
bool servoStateScheduledV1 = false; // Trạng thái hiện tại của Servo do lịch trình V1 điều khiển
unsigned long lastCheckSecondV1 = 0; // Để tránh kiểm tra quá thường xuyên trong cùng một giây cho V1

// --- Callback function (được gọi khi thời gian được điều chỉnh qua NTP) ---
void timeavailable(struct timeval *t) {
  Serial.println(">>> NTP: Thời gian đã được đồng bộ từ NTP!");
}

// --- Hàm để kích hoạt Servo thủ công (cho V0) ---
void activateServoManual() {
  // Chỉ kích hoạt nếu Servo không đang chạy (từ thủ công hoặc lịch trình)
  // Logic này phức tạp hơn một chút nếu bạn muốn ưu tiên lịch trình hoặc thủ công.
  // Hiện tại, nó ưu tiên thủ công và sẽ chạy trong 10 giây.
  if (!servoManualRunning && !servoStateScheduledV1) {
    myServo.write(0); // Bắt đầu quay (hoặc góc mở feeder của bạn)
    Serial.println(">>> SERVO: Servo ON (Kích hoạt thủ công từ V0).");
    servoManualActivationStartTime = millis();
    servoManualRunning = true;
    // Cập nhật trạng thái lên Blynk nếu có widget hiển thị trạng thái chung
    // Blynk.virtualWrite(V_OVERALL_SERVO_STATUS, 1);
  } else {
    Serial.println(">>> SERVO: Servo đã đang chạy (thủ công hoặc lịch trình), bỏ qua kích hoạt V0.");
  }
}

// --- Hàm để BẬT Servo theo lịch trình (cho V1) ---
void turnOnServoScheduled() {
  // Chỉ bật nếu Servo không đang ở trạng thái lịch trình ON VÀ không đang chạy thủ công từ V0
  if (!servoStateScheduledV1 && !servoManualRunning) {
    myServo.write(0); // Bật Servo
    Serial.println(">>> SERVO: Servo ON (Lịch trình V1).");
    servoStateScheduledV1 = true;
    // Cập nhật trạng thái lên Blynk nếu có widget hiển thị trạng thái lịch trình V1
    // Blynk.virtualWrite(V_SCHEDULED_SERVO_STATUS_V1, 1);
  }
}

// --- Hàm để TẮT Servo theo lịch trình (cho V1) ---
void turnOffServoScheduled() {
  // Chỉ tắt nếu Servo đang ở trạng thái lịch trình ON VÀ không đang chạy thủ công từ V0
  if (servoStateScheduledV1 && !servoManualRunning) {
    myServo.write(90); // Tắt Servo (quay về góc đóng/dừng)
    Serial.println(">>> SERVO: Servo OFF (Lịch trình V1).");
    servoStateScheduledV1 = false;
    // Cập nhật trạng thái lên Blynk nếu có widget hiển thị trạng thái lịch trình V1
    // Blynk.virtualWrite(V_SCHEDULED_SERVO_STATUS_V1, 0);
  }
}


// --- BLYNK_WRITE cho Nút điều khiển Servo thủ công (V0) ---
BLYNK_WRITE(V0) {
  int buttonState = param.asInt();
  Serial.print(">>> BLYNK_V0: Trạng thái nút nhận được: ");
  Serial.println(buttonState);
  if (buttonState == 1) {
    activateServoManual(); // Kích hoạt servo cho 10 giây
    Serial.println(">>> BLYNK_V0: Servo được kích hoạt bởi nút V0.");
  }
}

// --- BLYNK_WRITE cho Lịch trình BẬT/TẮT (V1) ---
// Nhận 2 tham số: param[0] = startTime (tổng giây từ nửa đêm), param[1] = endTime (tổng giây từ nửa đêm)
// Từ một widget Time Input của Blynk, cấu hình gửi theo "Seconds"
BLYNK_WRITE(V1) { // Đã đổi từ V4 sang V1
  long receivedStartTime = param[0].asLong(); // Đọc startTime dưới dạng long
  long receivedEndTime = param[1].asLong();   // Đọc endTime dưới dạng long

  Serial.printf(">>> BLYNK_V1: Nhận lịch trình: Bắt đầu %ld s, Kết thúc %ld s\n", receivedStartTime, receivedEndTime);

  // Nếu cả hai thời gian đều là 0, điều đó chỉ ra rằng lịch trình đang được xóa/vô hiệu hóa
  if (receivedStartTime == 0 && receivedEndTime == 0) {
    scheduleStartTime = 999999; // Đặt một số rất lớn để vô hiệu hóa lịch trình
    scheduleEndTime = 999999;   // Đặt một số rất lớn để vô hiệu hóa lịch trình
    isScheduleSetV1 = false; // Xóa cờ lịch trình
    turnOffServoScheduled(); // Đảm bảo servo tắt nếu lịch trình bị xóa
    Serial.println(">>> BLYNK_V1: Lịch trình đã được xóa (vô hiệu hóa).");
  } else {
    scheduleStartTime = receivedStartTime;
    scheduleEndTime = receivedEndTime;
    isScheduleSetV1 = true; // Đánh dấu lịch trình đã được đặt

    // In ra thời gian đã đặt (chuyển đổi ngược từ giây sang HH:MM:SS để dễ đọc)
    Serial.printf(">>> BLYNK_V1: Lịch trình đã đặt: BẬT lúc %02ld:%02ld:%02ld, TẮT lúc %02ld:%02ld:%02ld\n",
                  scheduleStartTime / 3600, (scheduleStartTime % 3600) / 60, scheduleStartTime % 60,
                  scheduleEndTime / 3600, (scheduleEndTime % 3600) / 60, scheduleEndTime % 60);
    lastCheckSecondV1 = -1; // Đặt lại giây kiểm tra cuối cùng để cho phép cập nhật trạng thái ngay lập tức nếu thời gian khớp
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Bắt đầu Setup ---");

  // --- Cấu hình Servo ---
  myServo.attach(servoPin);
  myServo.write(90); // Đảm bảo Servo ở trạng thái dừng ban đầu
  Serial.printf(">>> SERVO: Servo đã gắn vào chân %d, đặt ở góc 90 độ.\n", servoPin);

  // --- Kết nối WiFi ---
  Serial.printf(">>> WIFI: Đang kết nối tới %s ", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" KẾT NỐI.");
  Serial.printf(">>> WIFI: Địa chỉ IP của ESP32: %s\n", WiFi.localIP().toString().c_str());

  // --- Cấu hình NTP ---
  sntp_set_time_sync_notification_cb(timeavailable); // Đăng ký hàm callback để nhận thông báo khi thời gian được đồng bộ
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); // Cấu hình múi giờ và các máy chủ NTP
  Serial.printf(">>> NTP: Đã cấu hình với GMT offset %ld, máy chủ: %s, %s\n", gmtOffset_sec, ntpServer1, ntpServer2);

  // --- Kết nối Blynk ---
  Serial.print(">>> BLYNK: Đang kết nối tới Blynk...");
  Blynk.begin(auth, ssid, password, blynk_server, blynk_port);
  while (Blynk.connect() == false) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" KẾT NỐI tới Blynk");
  Serial.println(">>> BLYNK: ESP32 đã kết nối tới Blynk Cloud.");

  // Kiểm tra và in ra thời gian hiện tại sau khi setup và NTP đồng bộ
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char formattedTime[64];
    strftime(formattedTime, sizeof(formattedTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    Serial.print(">>> NTP: Thời gian hiện tại sau Setup: ");
    Serial.println(formattedTime);
  } else {
    Serial.println(">>> NTP: Không thể lấy được thời gian NTP sau Setup.");
  }
  Serial.println("--- Setup Hoàn tất ---");
}

void loop() {
  Blynk.run(); // Luôn chạy Blynk để duy trì kết nối và xử lý các sự kiện

  // --- Logic dừng Servo cho kích hoạt thủ công (V0) ---
  if (servoManualRunning) {
    if (millis() - servoManualActivationStartTime >= servoManualRunDuration) {
      myServo.write(90); // Dừng Servo (quay về góc đóng/dừng)
      Serial.println(">>> SERVO: Servo TẮT (10s kích hoạt thủ công V0 đã kết thúc).");
      servoManualRunning = false;
      // Cập nhật trạng thái lên Blynk nếu có widget hiển thị trạng thái chung
      // Blynk.virtualWrite(V_OVERALL_SERVO_STATUS, 0);
    }
  }

  // --- Logic so sánh thời gian cho Lịch trình BẬT/TẮT V1 ---
  if (isScheduleSetV1) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      long currentTotalSeconds = (long)timeinfo.tm_hour * 3600 +
                                 (long)timeinfo.tm_min * 60 +
                                 (long)timeinfo.tm_sec;

      // In ra thường xuyên để gỡ lỗi, có thể bình luận khi không cần thiết
      // if (timeinfo.tm_sec != lastCheckSecondV1) {
      //   Serial.printf(">>> V1_SCHEDULE: Current Time: %02d:%02d:%02d (%ld s) | Lịch trình: BẬT %ld s, TẮT %ld s\n",
      //                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, currentTotalSeconds, scheduleStartTime, scheduleEndTime);
      //   lastCheckSecondV1 = timeinfo.tm_sec;
      // }

      // Trường hợp 1: Thời gian bắt đầu trước thời gian kết thúc (lịch trình trong cùng một ngày, ví dụ: 07:00 đến 08:00)
      if (scheduleStartTime < scheduleEndTime) {
        if (currentTotalSeconds >= scheduleStartTime && currentTotalSeconds < scheduleEndTime) {
          turnOnServoScheduled(); // Bật Servo nếu trong khoảng thời gian BẬT
        } else {
          turnOffServoScheduled(); // Tắt Servo nếu ngoài khoảng thời gian BẬT
        }
      }
      // Trường hợp 2: Thời gian bắt đầu sau thời gian kết thúc (lịch trình kéo dài qua nửa đêm, ví dụ: 22:00 đến 06:00)
      else if (scheduleStartTime > scheduleEndTime) {
        if (currentTotalSeconds >= scheduleStartTime || currentTotalSeconds < scheduleEndTime) {
          turnOnServoScheduled(); // Bật Servo nếu sau thời gian bắt đầu HOẶC trước thời gian kết thúc (kéo dài qua nửa đêm)
        } else {
          turnOffServoScheduled(); // Tắt Servo nếu trong khoảng thời gian nghỉ (ví dụ: 06:00 đến 22:00)
        }
      }
      // Trường hợp 3: scheduleStartTime == scheduleEndTime (và không phải 0, tức là không bị xóa)
      // Trong trường hợp này, không có khoảng thời gian BẬT thực sự.
      // Hàm turnOffServoScheduled() sẽ được gọi nếu không có lịch trình hợp lệ.
    } else {
      Serial.println(">>> NTP: Không thể lấy được thời gian NTP để so sánh lịch trình V1 trong loop().");
    }
  }


  // --- Đảm bảo cập nhật thời gian NTP thường xuyên ---
  static unsigned long lastNtpSync = 0;
  const long ntpSyncInterval = 600000; // Đồng bộ lại sau mỗi 5 phút (300,000 ms)
  if (millis() - lastNtpSync >= ntpSyncInterval) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    lastNtpSync = millis();
    Serial.println(">>> NTP: Đang cố gắng đồng bộ lại NTP.");
  }
}