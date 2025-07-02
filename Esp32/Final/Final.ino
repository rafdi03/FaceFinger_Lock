#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ===== GANTI DENGAN JARINGAN KAMU =====
const char* ssid = "Warkopyoi";
const char* password = "warkopreborn";
const char* serverUrl = "http://192.168.100.125:5000/absen";

#define LED_FLASH_PIN 4 // GPIO flash bawaan ESP32-CAM

// ==== Konfigurasi Kamera AI Thinker ====
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  Serial.println("\nTerhubung ke WiFi: " + WiFi.localIP().toString());

  // Setup pin LED
  pinMode(LED_FLASH_PIN, OUTPUT);
  digitalWrite(LED_FLASH_PIN, LOW); // Pastikan LED mati di awal

  // Konfigurasi kamera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Inisialisasi kamera
  if (esp_camera_init(&config) != ESP_OK) {
  Serial.println("Gagal inisialisasi kamera");
  return;
  }

  // Set sensor kamera
  sensor_t *s = esp_camera_sensor_get();
  s->set_quality(s, 10);
  s->set_brightness(s, -2);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_ae_level(s, -2);
  s->set_gain_ctrl(s, 1);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 0);
  s->set_vflip(s, 0);
  s->set_dcw(s, 1);
  s->set_colorbar(s, 0);

  Serial.println("Ketik angka 0-255 untuk atur LED atau ketik 'kirim'");
}

void loop() {
  if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.equalsIgnoreCase("kirim")) {
      ambilDanKirimGambar();
  } else {
      int nilai = input.toInt();
      if (nilai >= 0 && nilai <= 255) {
      analogWrite(LED_FLASH_PIN, nilai);
      Serial.print("Kecerahan LED: ");
      Serial.println(nilai);
      } else {
      Serial.println("Masukkan 0-255 atau 'kirim'");
      }
    }
  }
}

void ambilDanKirimGambar() {
// Flush 3 frame pertama agar dapat frame terbaru
  for (int i = 0; i < 3; i++) {
  camera_fb_t *flush_fb = esp_camera_fb_get();
  if (flush_fb) {
  esp_camera_fb_return(flush_fb);
  }
  delay(50); // jeda kecil untuk kestabilan
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
  Serial.println("❌ Gagal ambil gambar");
  return;
  }

  Serial.println("📤 Mengirim gambar ke server...");

  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  if (httpResponseCode > 0) {
  Serial.printf("✅ Terkirim! Kode: %d\n", httpResponseCode);
  Serial.println(http.getString());
  } else {
  Serial.printf("❌ Gagal kirim! Kode: %d\n", httpResponseCode);
  }

  http.end();
  esp_camera_fb_return(fb);
}