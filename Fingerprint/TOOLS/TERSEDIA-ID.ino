#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Gunakan pin 2 (RX) dan 3 (TX) untuk komunikasi ke sensor fingerprint
SoftwareSerial fingerSerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(9600);        // Untuk output ke Serial Monitor
  delay(1000);

  fingerSerial.begin(57600); // Komunikasi ke sensor fingerprint
  finger.begin(57600);       // Inisialisasi sensor

  if (!finger.verifyPassword()) {
    Serial.println("❌ Sensor fingerprint tidak terdeteksi!");
    while (1) delay(1);
  }

  finger.getTemplateCount(); // Ambil jumlah template terdaftar
  Serial.print("📋 Jumlah template: ");
  Serial.println(finger.templateCount);

  Serial.println("📌 Daftar ID terdaftar:");
  for (int id = 1; id <= 127; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      Serial.print("✅ ID tersedia: ");
      Serial.println(id);
    }
  }
}

void loop() {
  // Tidak melakukan apa-apa di loop
}
