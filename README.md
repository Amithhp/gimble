# 3-Axis Servo Gimbal Stabilizer with Joystick & ESP32-CAM

A real-time **3-Axis Camera Gimbal** built using an **Arduino Uno**, **MPU6050 6-axis IMU**, three micro servo motors, and a **2-axis analog joystick** for manual pan/tilt override and instant center reset. Designed with a modular payload tray to mount an **ESP32-CAM** for low-latency wireless video streaming over a standalone local Wi-Fi Access Point.

---

## Key Features

- **Real-Time 2-Axis IMU Stabilization (Roll & Pitch):** Fuses raw accelerometer and gyroscope data from the MPU6050 using a complementary filter ($96\%$ gyro + $4\%$ accel) with automatic startup offset calibration.
- **Manual 3-Axis Joystick Control:**
  - **VRx (Pan / Yaw):** Smooth accumulative yaw steering ($0^\circ\text{–}180^\circ$).
  - **VRy (Tilt / Pitch):** Accumulative pitch angle adjustment while maintaining active IMU stabilization.
  - **SW (Push Button):** Instant reset of all axes back to neutral ($90^\circ$).
- **Brownout-Protected Power Architecture:** Uses a **12V battery** stepped down to a regulated **5V rail** via an **L298N module's onboard 5V regulator** with a common ground to prevent microcontroller resets under servo load.
- **Wireless Video Feed (V2 Module):** Standalone **ESP32-CAM** SoftAP (`192.168.4.1`) paired with a lightweight Python OpenCV desktop viewer.

---

## Hardware & Pinout

### Components Used
- **Microcontroller:** Arduino Uno
- **IMU Sensor:** MPU6050 (I2C)
- **Actuators:** 3× Micro Servo Motors (SG90 / MG90S) for Pitch, Roll, and Yaw
- **Input:** KY-023 Dual-Axis Analog Joystick Module
- **Power Supply:** 12V Battery + L298N Voltage Regulator Module
- **Camera Module (Optional/V2):** AI-Thinker ESP32-CAM

### Wiring Connections

| Component | Pin | Arduino Uno Pin | Notes |
| :--- | :--- | :--- | :--- |
| **Pitch Servo** | Signal (Orange/Yellow) | `D9` | Tilts camera tray up/down |
| **Roll Servo** | Signal (Orange/Yellow) | `D10` | Levels horizon left/right |
| **Yaw Servo** | Signal (Orange/Yellow) | `D11` | Base rotation (pan) |
| **MPU6050** | `VCC` / `GND` | `5V` / `GND` | Keep sensor flat during boot calibration |
| **MPU6050** | `SDA` / `SCL` | `A4` / `A5` | I2C communication bus |
| **Joystick** | `VRx` / `VRy` | `A0` / `A1` | Analog pan & tilt steering |
| **Joystick** | `SW` | `D2` | Configured with `INPUT_PULLUP` |
| **Servo Power** | `VCC` / `GND` | External `5V` / `GND` | **Must share Common GND with Arduino** |

---

## Getting Started

### 1. Arduino Gimbal Firmware
1. Open `arduino_gimbal/arduino_gimbal.ino` in the **Arduino IDE**.
2. Select **Board:** `Arduino Uno` and your active **COM Port**.
3. Place the gimbal completely flat and still, then upload the sketch.
4. Wait ~2 seconds after power-on while `calculate_IMU_error()` calibrates the accelerometer and gyroscope offsets.

### 2. ESP32-CAM Wireless Viewer (Optional)
1. Flash `esp32_cam_stream/esp32_cam_stream.ino` to your AI-Thinker ESP32-CAM.
2. Connect your PC to the Wi-Fi network **`ESP32-CAM-Gimbal`** (Password: `12345678`).
3. Install Python dependencies and launch the OpenCV viewer:
   ```bash
   pip install opencv-python numpy
   python esp32_cam_stream/viewer_rgb565.py
