#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

// ==================== KONFIGURASI ====================
// WiFi
const char* ssid = "POCO C75";
const char* password = "pppppppp";
const char* serverUrl = "http://192.168.60.153:5000/absen";

// UART: Arduino Nano TX ke ESP32-CAM RX (GPIO16)
HardwareSerial mySerial(1);

// LED indikator (opsional)
#define LED_INDICATOR 33

// ==================== VARIABEL GLOBAL ====================
int fingerID = -1;
bool prosesBerjalan = false;  // ✅ Tambahkan ini!

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);

  Serial.println("⏳ Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Terhubung ke WiFi");

  initUART();
  Serial.println("📡 Mode: Menunggu ID fingerprint dari Arduino Nano");
}

// ==================== LOOP UTAMA ====================
void loop() {
  if (!prosesBerjalan && mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      Serial.println("📥 Data diterima dari Arduino Nano: " + data);

      // Ekstrak angka dari format "ID: 2"
      int delimiterIndex = data.indexOf(':');
      if (delimiterIndex != -1) {
        String angkaStr = data.substring(delimiterIndex + 1);
        angkaStr.trim();
        int id = angkaStr.toInt();

        if (id > 0) {
          fingerID = id;
          prosesBerjalan = true;
        }
      }
    }
  }

  if (prosesBerjalan) {
    mySerial.end();  // Stop UART agar kamera bisa pakai GPIO
    delay(100);

    if (setupCamera()) {
      int user_id = ambilIDWajahDariServer();
      esp_camera_deinit();

      Serial.println("Fingerprint ID: " + String(fingerID));
      Serial.println("User ID dari server: " + String(user_id));

      if (user_id == fingerID) {
        Serial.println("✅ Akses DITERIMA, ID cocok!");
      } else {
        Serial.println("❌ Akses DITOLAK, ID tidak cocok.");
      }
    } else {
      Serial.println("❌ Kamera gagal diinisialisasi.");
    }

    // Reset ulang untuk sesi berikutnya
    fingerID = -1;
    prosesBerjalan = false;
    initUART();
    Serial.println("🔁 Kembali menunggu ID fingerprint...");
  }
}

// ==================== INISIALISASI UART ====================
void initUART() {
  mySerial.begin(9600, SERIAL_8N1, 13, -1);  // RX = GPIO16 (TX Arduino Nano)
}

// ==================== SETUP KAMERA ====================
bool setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) == ESP_OK) {
    Serial.println("📷 Kamera aktif");
    return true;
  } else {
    Serial.println("❌ Kamera gagal inisialisasi");
    return false;
  }
}

// ==================== AMBIL ID WAJAH DARI SERVER ====================
int ambilIDWajahDariServer() {
  for (int i = 0; i < 3; i++) {
    camera_fb_t* flush = esp_camera_fb_get();
    if (flush) esp_camera_fb_return(flush);
    delay(50);
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Gagal mengambil gambar.");
    return -1;
  }

  HTTPClient http;
  WiFiClient client;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("📩 Respons server:\n" + response);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    if (!error && doc["status"] == "success") {
      int id = doc["user_id"];
      const char* nama = doc["nama"];
      Serial.println("👤 Nama terdeteksi: " + String(nama));
      http.end();
      return id;
    } else {
      Serial.println("❌ JSON parsing gagal atau status bukan 'success'");
    }
  } else {
    Serial.println("❌ Gagal kirim ke server. HTTP Code: " + String(httpResponseCode));
  }

  http.end();
  return -1;
}
