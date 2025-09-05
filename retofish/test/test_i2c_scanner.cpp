#include <Wire.h>
#include <Arduino.h>

void setup() {
  Wire.begin(21, 22);  // SDA, SCL của bạn
  Serial.begin(9600);
  delay(1000);
  Serial.println("I2C Scanner...");

  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", address);
      delay(10);
    }
  }
}

void loop() {
}
