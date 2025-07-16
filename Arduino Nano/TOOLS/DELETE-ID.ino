#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Gunakan SoftwareSerial untuk komunikasi dengan sensor fingerprint
SoftwareSerial fingerSerial(2, 3); // RX, TX → sesuaikan dengan wiring
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(9600);
  while (!Serial); // Tunggu serial aktif

  Serial.println("🔒 Menghubungkan ke sensor sidik jari...");

  finger.begin(57600);
  delay(100);

  if (finger.verifyPassword()) {
    Serial.println("✅ Sensor fingerprint terdeteksi!");
  } else {
    Serial.println("❌ Gagal terhubung ke sensor. Cek wiring!");
    while (1);
  }

  // Tampilkan jumlah data tersimpan
  finger.getTemplateCount();
  Serial.print("Jumlah ID tersimpan: ");
  Serial.println(finger.templateCount);

  Serial.println("Masukkan ID yang ingin dihapus, lalu tekan ENTER:");
}

void loop() {
  if (Serial.available()) {
    int id = Serial.parseInt();
    Serial.print("🗑️ Menghapus ID ");
    Serial.println(id);

    uint8_t result = finger.deleteModel(id);

    if (result == FINGERPRINT_OK) {
      Serial.println("✅ ID berhasil dihapus.");
    } else if (result == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("❌ Kesalahan komunikasi.");
    } else if (result == FINGERPRINT_BADLOCATION) {
      Serial.println("❌ ID tidak ditemukan.");
    } else if (result == FINGERPRINT_FLASHERR) {
      Serial.println("❌ Gagal menghapus dari memori.");
    } else {
      Serial.println("❌ Kesalahan tidak diketahui.");
    }

    // Tampilkan kembali ID yang tersedia
    Serial.println("\n📋 ID tersisa:");
    for (int i = 1; i < 128; i++) {
      if (finger.loadModel(i) == FINGERPRINT_OK) {
        Serial.print("ID: ");
        Serial.println(i);
      }
    }

    Serial.println("\nMasukkan ID lain untuk dihapus:");
  }
}
