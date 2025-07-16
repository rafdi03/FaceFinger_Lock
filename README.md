# FaceFinger_Lock

FaceFinger_Lock is a smart home security system that implements **dual authentication** by combining **face recognition** and **fingerprint scanning** for high-level, reliable access control.

## ✨ Main Features

- **Dual Authentication:** Requires both fingerprint and face verification for door access, enhancing security.
- **Hardware Integration:** Utilizes a fingerprint sensor and camera module (ESP32-CAM) managed by Arduino Nano.
- **WiFi Connectivity:** Supports WiFi for server communication and user data retrieval.
- **Real-time Feedback:** Provides access confirmation (granted/denied) via LED indicators and serial monitor messages.
- **User Management:** Detects, adds, and manages registered user IDs (both face and fingerprint).

## 🛠️ How It Works

1. **Device Initialization**
   - Arduino Nano initializes the fingerprint sensor and camera. WiFi is enabled for server communication.

2. **Authentication Process**
   - **Step 1:** User scans their fingerprint on the sensor.
   - **Step 2:** If the fingerprint matches a registered ID, the system triggers the camera to capture the user's face.
   - **Step 3:** The captured face image is sent to the server (or processed locally) using the `face_recognition` library.
   - **Step 4:** If the face recognition ID matches the fingerprint ID, access is granted and the relay unlocks the door.
   - **Step 5:** If the IDs do not match, access is denied.

3. **Feedback**
   - LED indicators and serial monitor outputs provide immediate status updates (access granted/denied, user ID, error handling).

## ⚙️ Technologies & Libraries

- **Hardware:** Arduino Nano, ESP32-CAM, Adafruit Fingerprint Sensor, Relay, LED
- **Software:** 
  - Languages: Python (face recognition), C++ (Arduino)
  - Libraries: `face_recognition`, `dlib`, `Adafruit_Fingerprint`, `SoftwareSerial`
  - Communication: WiFi (ESP32), UART (between Arduino and ESP32)
- **Operating System:** Compatible with Windows, Linux, and MacOS.

## 📦 Example Directory Structure

```
Arduino Nano/
  ├── FinalCodeFingerprintRelay.ino
  └── TOOLS/
        ├── DETEKSI-ID.ino
        └── TERSEDIA-ID.ino
venv/
  └── Lib/site-packages/face_recognition/
```

## 🚀 Installation & Usage

1. **Upload Arduino code** to Arduino Nano and ESP32-CAM using `FinalCodeFingerprintRelay.ino` and the relevant tools.
2. **Install Python dependencies** on your PC/server:
   ```
   pip install face_recognition dlib
   ```
3. **Prepare the face database folder** on the server as needed.
4. **Run the face recognition script** to verify the captured photo.
5. **Connect devices to WiFi** and ensure communication between hardware and server is functioning.

## 📄 License

This repository uses open-source libraries under their respective licenses. Please refer to each library’s LICENSE file for details.

---

**Author:** [rafdi03](https://github.com/rafdi03)  
_Automatic documentation by Copilot. Please complete or adapt this README as needed for your project._
