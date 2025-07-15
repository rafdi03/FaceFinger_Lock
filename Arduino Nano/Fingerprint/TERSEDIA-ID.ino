#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(2); // UART2 ESP32
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  delay(100);

  mySerial.begin(57600, SERIAL_8N1, 3, 1); // RX, TX
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    Serial.println("Sensor fingerprint tidak terdeteksi!");
    while (1) delay(1);
  }

  finger.getTemplateCount();
  Serial.print("Jumlah template: ");
  Serial.println(finger.templateCount);

  Serial.println("Daftar ID terdaftar:");
  for (int id = 1; id <= 127; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      Serial.print("✅ ID tersedia: ");
      Serial.println(id);
    } else {
      // Kosong atau error
    }
  }
}

void loop() {
  // Tidak melakukan apa-apa
}
