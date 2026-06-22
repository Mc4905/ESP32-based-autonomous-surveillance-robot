# ESP32-Based Autonomous Surveillance Robot

An autonomous IoT surveillance robot built on the ESP32. The robot drives itself, avoids obstacles, and continuously scans its environment for hazards — gas leaks, tilting/falls, and human presence (via a thermal sensor) — while streaming live sensor data to a Wi-Fi web dashboard.

---

## How It Works

1. ESP32 boots and starts its own Wi-Fi access point (`SMART_ROBOT`) hosting a live dashboard
2. On `/start`, the robot drives forward autonomously using an ultrasonic sensor to detect and steer around obstacles
3. A thermal camera (8x8 pixel array) continuously scans for body-heat signatures to flag human presence
4. A gas sensor and an accelerometer/gyroscope (MPU6050) monitor for hazardous gas levels and dangerous tilt/fall events
5. If gas or tilt thresholds are exceeded, the robot immediately stops driving as a safety cutoff
6. All readings (distance, gas level, temperature, human detection, tilt status) are pushed to the web dashboard in real time
7. The dome enclosure (3D modeled in Fusion 360) houses the hardware for a clean, deployable form factor

---

## Features

- Fully autonomous driving with ultrasonic obstacle avoidance
- Thermal-based human presence detection (AMG8833 8x8 thermal array)
- Gas/hazard detection with automatic stop on threshold breach
- Tilt/fall detection via MPU6050 with automatic stop
- Live web dashboard (auto-refreshing) with visual + audible human-detection alerts
- Self-hosted Wi-Fi access point — no router or external network required
- Remote Start/Stop control from the dashboard
- Compact dome enclosure designed in Fusion 360

---

## Hardware Used

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller — Wi-Fi AP, sensor processing, motor control |
| AMG8833 Thermal Camera | 8x8 pixel thermal array for human/heat detection |
| MPU6050 (Accelerometer/Gyro) | Detects tilt and fall events |
| HC-SR04 Ultrasonic Sensor | Measures distance for obstacle avoidance |
| MQ-series Gas Sensor | Detects hazardous gas concentration |
| L298N (or similar) Motor Driver | Drives the two DC drive motors |
| 2x DC Geared Motors | Robot drivetrain |
| Power Supply / Battery Pack | Regulated supply for ESP32, sensors, and motors |

---

## Pin Configuration

| Function | ESP32 Pin |
|---|---|
| Ultrasonic Trig | GPIO 5 |
| Ultrasonic Echo | GPIO 18 |
| Gas Sensor (Analog) | GPIO 34 |
| I2C SDA (Thermal Cam / MPU6050) | GPIO 21 |
| I2C SCL (Thermal Cam / MPU6050) | GPIO 22 |
| Motor A — Enable (PWM) | GPIO 25 |
| Motor A — IN1 / IN2 | GPIO 26 / GPIO 27 |
| Motor B — Enable (PWM) | GPIO 33 |
| Motor B — IN3 / IN4 | GPIO 32 / GPIO 4 |

---

## Project Files

| File | Description |
|---|---|
| `Smart_Suvreillance_main.ino` | Main Arduino/ESP32 source code |
| `Robot_Dome.f3d` | Fusion 360 3D model of the enclosure |
| `Circuit Diagram & Component Specications pdf.pdf` | Full circuit diagram and component details |
| `Experiment Results & Discussion.docx` | Testing results and analysis |

---

## 3D Enclosure

The surveillance dome was custom designed in Fusion 360. Open `Robot_Dome.f3d` in Fusion 360 to view or modify the model.

---

## Setup & Usage

**Requirements:**
- Arduino IDE with ESP32 board support installed
- ESP32 board package: `https://dl.espressif.com/dl/package_esp32_index.json`
- Libraries: `Adafruit_AMG88xx`, `Wire`, `WiFi`, `WebServer` (install via Arduino Library Manager)

**Steps:**

1. Clone the repository
   ```bash
   git clone https://github.com/Mc4905/ESP32-based-autonomous-surveillance-robot.git
   ```

2. Open `Smart_Suvreillance_main.ino` in Arduino IDE

3. Install the required libraries via Library Manager if prompted

4. Select the correct ESP32 board and COM port, then upload the code

5. On boot, the ESP32 creates its own Wi-Fi network:
   ```
   SSID:     SMART_ROBOT
   Password: 12345678
   ```

6. Connect a phone or laptop to that network and open `http://<esp32-ip>/` in a browser (check Serial Monitor for the IP, typically `192.168.4.1` in AP mode)

7. Use the **START** / **STOP** buttons on the dashboard to control the robot

> **Security note:** the default Wi-Fi credentials and gas/temperature/tilt thresholds are hardcoded in the `.ino` file. Change the SSID/password before any real-world deployment.

---

## Dashboard

The web dashboard auto-refreshes every second and displays:
- Live distance reading (cm)
- Gas sensor value
- Max thermal temperature (°C)
- Human detection status (with audible beep + yellow background alert)
- Tilt/fall status (orange background alert)

---

## Key Learnings

- ESP32 programming for autonomous IoT robotics
- Multi-sensor fusion (ultrasonic, thermal, gas, IMU) for real-time decision-making
- Self-hosted Wi-Fi access point and REST-style JSON API design on embedded hardware
- Motor control via PWM (`ledc`) for differential drive steering
- 3D enclosure design using Fusion 360 for hardware deployment

---

## Author

**Mc4905**
ECE | IoT & Embedded Systems
[GitHub Profile](https://github.com/Mc4905)
