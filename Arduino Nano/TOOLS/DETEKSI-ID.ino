#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial fingerSerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(9600);         // Serial ke ESP32
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint ready");
  } else {
    Serial.println("Fingerprint not found");
    while (1);
  }
}

void loop() {
  if (getFingerprintID()) {
    delay(2000);
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return false;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return false;

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("ID: ");
    Serial.println(finger.fingerID);
    return true;
  } else {
    Serial.println("Fingerprint not found");
    return false;
  }
}
