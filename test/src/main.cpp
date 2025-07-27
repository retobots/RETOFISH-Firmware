#include <Arduino.h>
#include <Stepper.h>
#include <ESP32Servo.h>  // Sử dụng thư viện ESP32Servo thay vì Servo

// Khai báo đối tượng Stepper cho trục X và trục Y
Stepper stepperX(200, 27, 14);  // 200 bước, chân D27 (STEP-X), D14 (DIR-X)
Stepper stepperY(200, 4, 2);    // 200 bước, chân D4 (STEP-Y), D2 (DIR-Y)

// Khai báo đối tượng Servo cho trục Z
Servo zServo;  // Khai báo đối tượng Servo

void setup() {
  // Khởi tạo Serial Monitor để debug
  Serial.begin(9600);

  // Đặt tốc độ cho động cơ stepper (bước/phút)
  stepperX.setSpeed(300);  // Tăng tốc độ động cơ Stepper X lên 300 bước/phút
  stepperY.setSpeed(300);  // Tăng tốc độ động cơ Stepper Y lên 300 bước/phút

  // Khởi tạo Servo (gắn vào chân 25)
  zServo.attach(25);  // Đảm bảo chân 25 hỗ trợ PWM
}

void loop() {
  // Di chuyển 2 vòng cho Stepper X và Y (400 bước cho mỗi vòng)
  stepperX.step(13000);  // Di chuyển 2 vòng cho Stepper X

    delay(3000);  // Đợi 1 giây

  

  






}
