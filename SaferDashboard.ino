#include <WiFi.h>
#include <ESPAsyncWebServer.h>    //HTTP requrst
#include <WebSocketsServer.h>  //communication
#include <ArduinoJson.h>
#include <Ticker.h>  // periodic task
#include <Wire.h>   //i2c communication
#include <MPU6050.h>
#include <ESP32Servo.h>

#define LED1 13       // LED with brightness control
#define LED2 12       // Pressure sensor-controlled LED
#define LED3 33       // LED on PIN 33
#define SERVO_PIN 16  // Servo motor pin
#define S0_PIN 26
#define S1_PIN 27
#define S2_PIN 14
#define S3_PIN 25
#define OUT_PIN 34
#define PRESSURE_SENSOR_PIN 32

bool led33State = false; // Initial state of LED on PIN33

MPU6050 mpu;
Servo servo;  // Create Servo object
Ticker timer;
WebSocketsServer websockets(81);
AsyncWebServer server(80);

// Calibration variables for MPU6050
int16_t ax_offset = 0, ay_offset = 0, az_offset = 0;

// LED states and settings
int brightness = 0;                   // Brightness level for LED1 (0-255)
bool pressureLEDState = false;        // State of LED2
bool pressureManualOverride = false;  // Manual override flag for LED2
int pressureThreshold = 500;          // Threshold for pressure sensor in Pascals (Pa)
int pressureBaseline = 10000;          // Baseline value when the sensor is not pressed
int servoState = 0;  // 0 for 0 degrees, 1 for 180 degrees

// HTML for the dashboard
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Safter Dashboard</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f2f5; margin: 0; padding: 0; }
        h1 { text-align: center; padding: 20px; background-color: #007BFF; color: white; margin: 0; }
        .container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; margin: 20px; }
        .card { background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); padding: 15px; width: 300px; text-align: center; }
        .value { font-size: 20px; color: #007BFF; }
        .color-preview { width: 50px; height: 50px; border-radius: 50%; margin: 0 auto; }
        .timestamp { font-size: 12px; color: #555; margin-top: 5px; }
        .status-green { color: green; font-weight: bold; }
        .status-red { color: red; font-weight: bold; }

        .cube-container {
    perspective: 600px;
    width: 150px;
    height: 150px;
    margin: 0 auto;
    position: relative;
}

#cube {
    width: 100px;
    height: 100px;
    position: absolute;
    top: 25px;
    left: 25px;
    transform-style: preserve-3d;
    transform: rotateX(0deg) rotateY(0deg) rotateZ(0deg);
    transition: transform 0.1s ease-out;
}

.face {
    position: absolute;
    width: 100px;
    height: 100px;
    background: rgba(0, 123, 255, 0.5);
    border: 1px solid #007BFF;
}

.front {
    transform: rotateY(0deg) translateZ(50px);
}

.back {
    transform: rotateY(180deg) translateZ(50px);
}

.left {
    transform: rotateY(-90deg) translateZ(50px);
}

.right {
    transform: rotateY(90deg) translateZ(50px);
}

.top {
    transform: rotateX(90deg) translateZ(50px);
}

.bottom {
    transform: rotateX(-90deg) translateZ(50px);
}

    </style>
</head>
<body>
    <h1>Safer Dashboard</h1> 
    <div class="container">
        <div class="card">
    <h3>Gyroscope</h3>
    <div class="cube-container">
        <div id="cube">
            <div class="face front"></div>
            <div class="face back"></div>
            <div class="face left"></div>
            <div class="face right"></div>
            <div class="face top"></div>
            <div class="face bottom"></div>
        </div>
    </div>
    <p>X: <span id="accel_x" class="value">0.00</span> m/s²</p>
    <p>Y: <span id="accel_y" class="value">0.00</span> m/s²</p>
    <p>Z: <span id="accel_z" class="value">0.00</span> m/s²</p>
    <div class="timestamp">Last updated: <span id="gyro_timestamp">N/A</span></div>
</div>

        <div class="card">
            <h3>Color Sensor</h3>
            <div id="colorPreview" class="color-preview" style="background-color: rgb(0,0,0);"></div>
            <p>Red: <span id="red_value" class="value">0</span> RGB</p>
            <p>Green: <span id="green_value" class="value">0</span> RGB</p>
            <p>Blue: <span id="blue_value" class="value">0</span> RGB</p>
        </div>
        <div class="card">
    <h3>Pressure Sensor</h3>
    <p>Pressure: <span id="pressure_value" class="value">Pressed</span></p>
    <p>Status: <span id="pressure_status" class="status-green">Normal</span></p>
    <div class="timestamp">Last updated: <span id="pressure_timestamp">N/A</span></div>
    <canvas id="pressureGraph" width="300" height="150" style="border:1px solid #ccc; margin-top: 10px;"></canvas>
</div>

        <div class="card">
            <h3>LED Brightness</h3>
            <input type="range" min="0" max="255" id="brightnessSlider" value="0" oninput="changeBrightness(this.value)">
            <p>Brightness: <span id="brightnessValue" class="value">0</span></p>
        </div>
        <div class="card">
            <h3>Pressure Sensor LED Control</h3>
            <button id="togglePressureLED" onclick="togglePressureLED()">Toggle Pressure LED</button>
        </div>

        <div class="card">
    <h3>LED Control</h3>
    <button id="toggleLED33" onclick="toggleLED33()">Toggle LED</button>
    </div>

    </div>





    
<script>
    const ws = new WebSocket('ws://' + location.hostname + ':81/');

    const canvas = document.getElementById('pressureGraph');
    const ctx = canvas.getContext('2d');





    // Data storage for pressure values
    const pressureData = [];
    const maxDataPoints = 50; // Maximum number of points to display on the graph
    const graphWidth = canvas.width;
    const graphHeight = canvas.height;

    function drawGraph() {
        ctx.clearRect(0, 0, graphWidth, graphHeight); // Clear the canvas

        // Draw axes
        ctx.beginPath();
        ctx.moveTo(30, 0); // Y-axis
        ctx.lineTo(30, graphHeight - 20);
        ctx.lineTo(graphWidth, graphHeight - 20); // X-axis
        ctx.strokeStyle = '#000';
        ctx.stroke();

        // Draw data points and line
        ctx.beginPath();
        ctx.strokeStyle = '#007BFF';
        for (let i = 0; i < pressureData.length; i++) {
            const x = 30 + (i * (graphWidth - 30) / maxDataPoints); // Scale X-axis
            const y = graphHeight - 20 - (pressureData[i] * (graphHeight - 40) / 100000); // Scale Y-axis (assuming 100000 is max pressure)
            if (i === 0) {
                ctx.moveTo(x, y);
            } else {
                ctx.lineTo(x, y);
            }
        }
        ctx.stroke();
    }

    function updateGraph(pressureValue) {
        // Add the new pressure value to the data array
        pressureData.push(pressureValue);
        if (pressureData.length > maxDataPoints) {
            pressureData.shift(); // Remove the oldest data point to keep the graph within the limit
        }

        drawGraph(); // Redraw the graph with updated data
    }

    ws.onmessage = (event) => {
        const data = JSON.parse(event.data);

        // Update gyroscope values
        document.getElementById('accel_x').textContent = (data.accel_x / 16384).toFixed(2);
        document.getElementById('accel_y').textContent = (data.accel_y / 16384).toFixed(2);
        document.getElementById('accel_z').textContent = (data.accel_z / 16384).toFixed(2);
        document.getElementById('gyro_timestamp').textContent = new Date().toLocaleTimeString();

        // Rotate the cube based on gyroscope values
        const xRotation = data.accel_x / 182; // Adjust for smoother rotation
        const yRotation = data.accel_y / 182;
        const zRotation = data.accel_z / 182;
        document.getElementById('cube').style.transform = `rotateX(${xRotation}deg) rotateY(${yRotation}deg) rotateZ(${zRotation}deg)`;

        // Update color sensor values
        document.getElementById('red_value').textContent = data.red;
        document.getElementById('green_value').textContent = data.green;
        document.getElementById('blue_value').textContent = data.blue;
        document.getElementById('colorPreview').style.backgroundColor = `rgb(${data.red}, ${data.green}, ${data.blue})`;

        // Update pressure sensor value and status
        if (data.pressure === "0") {
            document.getElementById('pressure_value').textContent = "0";
            document.getElementById('pressure_status').textContent = "Not Pressed";
            document.getElementById('pressure_status').className = "status-green";
            updateGraph(0); // Add "0" pressure value to the graph
        } else {
            const pressureValue = parseInt(data.pressure); // Make sure pressure is an integer
            document.getElementById('pressure_value').textContent = `${pressureValue} Pa`;
            const pressureStatus = pressureValue > 50000 ? "High" : "Normal";
            document.getElementById('pressure_status').textContent = pressureStatus;
            document.getElementById('pressure_status').className = pressureStatus === "High" ? "status-red" : "status-green";
            updateGraph(pressureValue); // Add the new pressure value to the graph
        }

        document.getElementById('pressure_timestamp').textContent = new Date().toLocaleTimeString();
    };

    // Function to change brightness (LED control)
    function changeBrightness(value) {
        document.getElementById('brightnessValue').textContent = value;
        ws.send(JSON.stringify({ command: "setBrightness", value: parseInt(value) }));
    }

    // Function to toggle pressure LED
    function togglePressureLED() {
        ws.send(JSON.stringify({ command: "togglePressureLED" }));
        console.log("Toggle Pressure LED button clicked"); // Debug log
    }


    // Function to toggle LED33
    function toggleLED33() {
        ws.send(JSON.stringify({ command: "toggleLED33" }));
    }

</script>

</body>
</html>
)=====";

// Function prototypes
void readColorSensor(int &red, int &green, int &blue);
void calibrateMPU();
void send_sensor_data();

void readColorSensor(int &red, int &green, int &blue) {
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  delay(10);
  red = pulseIn(OUT_PIN, HIGH);

  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  delay(10);
  green = pulseIn(OUT_PIN, HIGH);

  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  delay(10);
  blue = pulseIn(OUT_PIN, HIGH);

  red = map(red, 0, 1023, 0, 255);
  green = map(green, 0, 1023, 0, 255);
  blue = map(blue, 0, 1023, 0, 255);
}

void calibrateMPU() {
  int32_t ax_sum = 0, ay_sum = 0, az_sum = 0;
  for (int i = 0; i < 100; i++) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    delay(10);
  }
  ax_offset = ax_sum / 100;
  ay_offset = ay_sum / 100;
  az_offset = az_sum / 100;
}


void send_sensor_data() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  int red, green, blue;
  readColorSensor(red, green, blue);

  // Read the pressure sensor value
  int pressure_raw = analogRead(PRESSURE_SENSOR_PIN);
  Serial.print("Raw Pressure Value: ");
    Serial.println(pressure_raw);

  // Define a threshold to determine whether the sensor is pressed or not
  int threshold = 50;  // Adjust this threshold based on your sensor
  

   // If the pressure sensor is not pressed, show "0"
  int pressure_value;
  if (pressure_raw < threshold) {
    pressure_value = 0;  // No pressure detected
  } else {
    pressure_value = pressure_raw;
    // Map the value from high to low for a reverse effect
    pressure_value = map(pressure_raw, threshold, 1023, 150000, 0);
    if (pressure_value < 0) {
      pressure_value = 0;  // Ensure no negative values
    }
  }

    Serial.print("Mapped Pressure Value: ");
    Serial.println(pressure_value);


  StaticJsonDocument<200> doc;
  doc["accel_x"] = ax - ax_offset;
  doc["accel_y"] = ay - ay_offset;
  doc["accel_z"] = az - az_offset;
  doc["red"] = red;
  doc["green"] = green;
  doc["blue"] = blue;
  doc["pressure"] = pressure_value;


   if (!pressureManualOverride) {
        if (pressure_value > 0) {
            digitalWrite(LED2, HIGH);
        } else {
            digitalWrite(LED2, LOW);
        }
    }


  String jsonString;
  serializeJson(doc, jsonString);
  websockets.sendTXT(0, jsonString);
}

void toggle_servo() {
  if (servoState == 0) {
    servo.write(90);  // Move servo to 180 degrees
    servoState = 90;    // Update state
  } else {
    servo.write(0);    // Move servo to 0 degrees
    servoState = 0;    // Update state
  }
}



void setup() {
  Serial.begin(115200);

  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);

  

  WiFi.softAP("SensorDashboard", "");

  Wire.begin();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1)
      ;
  }
  mpu.initialize();
  calibrateMPU();

  

websockets.onEvent([&](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        StaticJsonDocument<100> doc;
        deserializeJson(doc, payload);

        // Extract command from JSON
        if (doc.containsKey("command")) {
            String command = doc["command"].as<String>(); // Get the command from the JSON


        // Control LED Brightness
            if (command == "setBrightness") {
                brightness = doc["value"];
                analogWrite(LED1, brightness);
            }

            // Toggle Pressure LED (Manual Control)
            if (command == "togglePressureLED") {
                pressureManualOverride = !pressureManualOverride;
                 if (pressureManualOverride) {
                pressureLEDState = !pressureLEDState;
                digitalWrite(LED2, pressureLEDState ? HIGH : LOW);
                Serial.print("Pressure LED State: ");
                Serial.println(pressureLEDState ? "ON" : "OFF");
            }
            }

            // Toggle LED on PIN 33 (Manual Control)
            if (command == "toggleLED33") {
                led33State = !led33State;
                digitalWrite(LED3, led33State ? HIGH : LOW);
                Serial.print("LED33 state: ");
                Serial.println(led33State ? "ON" : "OFF");
            }
      }
    }
});




  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", webpage);
  });

  server.begin();
  websockets.begin();
}


void loop() {
  websockets.loop();
  send_sensor_data();
  delay(10);
}