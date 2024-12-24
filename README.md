# Safer Dashboard with ESP32

## Overview

This project is a real-time sensor monitoring and control dashboard, implemented using an ESP32 microcontroller and various sensors. The system integrates features such as gyroscope visualization, color sensing, pressure monitoring, and LED/servo motor control. The dashboard is accessible via a web-based interface, providing live data updates and control functionalities.

https://github.com/user-attachments/assets/63502074-32f2-48d5-9176-565f2838490a

---

## Features

1. **Web-Based Dashboard**:
   - Real-time updates using WebSockets.
   - Displays sensor data, including gyroscope, pressure, and color sensor values.
   - Allows user control of LEDs.

2. **Sensor Integration**:
   - **MPU6050**: Monitors accelerometer data for gyroscope visualization.
   - **Color Sensor (TCS3200)**: Detects RGB values and displays live color preview.
   - **Pressure Sensor**: Detects and monitors pressure values with threshold-based LED control.

3. **Interactive Controls**:
   - Adjust LED brightness via a slider.
   - Toggle LEDs directly from the web dashboard.

4. **Visualization**:
   - Live gyroscope data visualized as a 3D rotating cube.
   - Pressure graph updates dynamically with real-time data.

---

## Components and Circuit Diagram

### Components

1. **ESP32 Development Board**
2. **MPU6050 Accelerometer and Gyroscope Module**
3. **TCS3200 Color Sensor Module**
4. **Pressure Sensor**
5. **3 LEDs** for visual indication
6. Jumper wires for connections

### Circuit Diagram

Refer to the `circuit_diagram.png` file in the repository for a detailed wiring schematic.

---

## Hardware Connections

| Component        | ESP32 Pin      | Description                       |
|------------------|----------------|-----------------------------------|
| LED1             | GPIO13         | Brightness-controlled LED         |
| LED2             | GPIO12         | Pressure sensor-controlled LED    |
| LED3             | GPIO33         | Manual toggle LED                 |
| TCS3200 Sensor   | GPIO26, GPIO27 | S0 and S1 control pins            |
|                  | GPIO14, GPIO25 | S2 and S3 control pins            |
|                  | GPIO34         | Output pin                        |
| MPU6050          | GPIO21, GPIO22 | I2C SDA and SCL pins              |
| Pressure Sensor  | GPIO32         | Analog input pin                  |

---

## Software Setup

### Dependencies

Install the following Arduino libraries before uploading the code:
- [WiFi](https://github.com/espressif/arduino-esp32)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [Ticker](https://github.com/sstaub/Ticker)
- [Wire](https://github.com/arduino-libraries/Wire)

### Setup Instructions

1. Clone this repository:
   ```bash
   git clone https://github.com/ravindu135madushan/Safer-Dashboard.git
   ```
2. Open the project in the Arduino IDE.
3. Configure the ESP32 board:
   - Go to `Tools > Board` and select `ESP32 Dev Module`.
4. Upload the code to your ESP32 using the correct COM port.
5. Connect to the Wi-Fi network named `SensorDashboard` (no password).

---

## Web Interface

### Accessing the Dashboard
1. Connect your device to the `SensorDashboard` Wi-Fi network.
2. Open a browser and navigate to `http://192.168.4.1`.

### Dashboard Features
- **Gyroscope Visualization**: View live accelerometer data as a rotating 3D cube.
- **Color Sensor**: Displays RGB values with a live color preview.
- **Pressure Monitoring**: Shows real-time pressure values and graph updates.
- **Controls**:
  - Adjust LED1 brightness.
  - Toggle LEDs.

---

## Code Description

1. **WebSocket Communication**:
   - Enables real-time data exchange between the ESP32 and the web interface.
   - Updates sensor data dynamically without refreshing the page.

2. **Sensor Data Collection**:
   - MPU6050 provides accelerometer data for gyroscope visualization.
   - Color sensor detects RGB values.
   - Pressure sensor measures analog pressure and maps it to a scaled value.

3. **Control Functions**:
   - LED brightness and state control via WebSocket commands.
   - Servo motor toggling between 0° and 90°.

4. **Graphical Representation**:
   - Pressure data is visualized on a dynamic graph.

---

## Future Enhancements

- Add more sensors for extended functionality.
- Implement a secure connection for web-based access.
- Expand control options for automation features.

---

## Troubleshooting

- **No Web Dashboard**: Check Wi-Fi connection and ensure you are connected to `SensorDashboard`.
- **Sensor Values Not Updating**: Verify hardware connections and sensor functionality.
