#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Gunakan pin D2 = RX, D3 = TX
SoftwareSerial fingerSerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  finger.begin(57600);
  delay(100);

  if (finger.verifyPassword()) {
    Serial.println("✅ Sensor sidik jari terdeteksi!");
  } else {
    Serial.println("❌ Gagal terhubung ke sensor fingerprint!");
    while (1);
  }

  // Tampilkan jumlah ID tersimpan
  finger.getTemplateCount();
  Serial.print("Jumlah sidik jari tersimpan: ");
  Serial.println(finger.templateCount);

  Serial.println("Masukkan ID yang ingin didaftarkan (1-127), lalu tekan ENTER:");
}

void loop() {
  if (Serial.available()) {
    int id = Serial.parseInt();
    if (id < 1 || id > 127) {
      Serial.println("❌ ID harus antara 1–127");
      return;
    }

    Serial.print("📝 Mulai pendaftaran untuk ID ");
    Serial.println(id);
    enrollFinger(id);
    Serial.println("Silakan masukkan ID lain atau reset untuk keluar.");
  }
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("✅ Gambar diambil.");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("❌ Kesalahan komunikasi");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("❌ Gagal ambil gambar");
        break;
      default:
        Serial.println("❌ Kesalahan tidak diketahui");
        break;
    }
    delay(100);
  }

  // Konversi ke fitur sidik jari
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("✅ Gambar dikonversi.");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("❌ Gambar buram");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("❌ Kesalahan komunikasi");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("❌ Gagal ekstrak fitur");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("❌ Gambar tidak valid");
      return p;
    default:
      Serial.println("❌ Kesalahan tidak diketahui");
      return p;
  }

  return FINGERPRINT_OK;
}

void enrollFinger(int id) {
  uint8_t p;

  Serial.println("👉 Letakkan jari pertama...");
  while ((p = getFingerprintEnroll()) != FINGERPRINT_OK);

  Serial.println("🛑 Angkat jari...");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("👉 Letakkan jari yang sama lagi...");
  while ((p = getFingerprintEnroll()) != FINGERPRINT_OK);

  // Konversi gambar kedua ke template buffer 2
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Gagal konversi gambar ke buffer 2");
    return;
  }

  // Bandingkan buffer 1 dan 2
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Kedua sidik jari cocok");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("❌ Kesalahan komunikasi");
    return;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("❌ Sidik jari tidak cocok");
    return;
  } else {
    Serial.println("❌ Kesalahan tidak diketahui");
    return;
  }

  // Simpan ke ID yang ditentukan
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("📦 Data disimpan dengan sukses!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("❌ Kesalahan komunikasi saat simpan");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("❌ Lokasi ID tidak valid");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("❌ Gagal simpan ke memori");
  } else {
    Serial.println("❌ Kesalahan tidak diketahui saat simpan");
  }
}
