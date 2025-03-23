#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <SFE_BMP180.h>

// Create BMP180 object
SFE_BMP180 bmp180;

// Replace with your network credentials
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";

// Set up the web server on port 80
WebServer server(80);

// Sensor pins
int sensor1_pin = 34; // GPIO 34 for sensor 1 (ADC1)
int sensor2_pin = 35; // GPIO 35 for sensor 2 (ADC2)

// Constants for moisture levels
const int dry1 = 595; // Dry value for sensor 1
const int wet1 = 239; // Wet value for sensor 1

const int dry2 = 650; // Dry value for sensor 2
const int wet2 = 300; // Wet value for sensor 2

// Global variables for sensor data
int sensor1_val = 0, sensor2_val = 0;
int sensor1_percent = 0, sensor2_percent = 0;
String status1, status2;
double temperature = 0, pressure = 0;

// Function to handle Wi-Fi connection
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to initialize BMP180 sensor
bool initializeBMP180() {
  if (bmp180.begin()) {
    Serial.println("BMP180 initialized successfully");
    return true;
  } else {
    Serial.println("BMP180 initialization failed");
    return false;
  }
}

// Function to read moisture sensor values and calculate percentage
void readMoistureSensors() {
  // Read sensor 1
  sensor1_val = analogRead(sensor1_pin);
  sensor1_percent = map(sensor1_val, wet1, dry1, 100, 0);

  // Read sensor 2
  sensor2_val = analogRead(sensor2_pin);
  sensor2_percent = map(sensor2_val, wet2, dry2, 100, 0);

  // Determine soil status based on percentage for sensor 1
  status1 = (sensor1_percent > 75) ? "Dry" :
            (sensor1_percent > 30 && sensor1_percent <= 75) ? "Medium" : "Wet";

  // Determine soil status based on percentage for sensor 2
  status2 = (sensor2_percent > 75) ? "Dry" :
            (sensor2_percent > 30 && sensor2_percent <= 75) ? "Medium" : "Wet";
}

// Function to read BMP180 temperature and pressure
void readBMP180() {
  char status = bmp180.startTemperature();
  
  if (status) {
    delay(1000); // Allow time for temperature measurement
    status = bmp180.getTemperature(temperature);
    if (status) {
      status = bmp180.startPressure(3); // '3' represents high-resolution mode
      if (status) {
        delay(status); // Wait for pressure reading
        status = bmp180.getPressure(pressure, temperature);
        if (!status) {
          Serial.println("Error reading pressure");
          pressure = -1;  // Mark error
        }
      } else {
        Serial.println("Error starting pressure measurement");
      }
    } else {
      Serial.println("Error getting temperature");
      temperature = -1; // Mark error
    }
  } else {
    Serial.println("Error starting temperature measurement");
  }
}

// Function to handle web root
void handleRoot() {
  String message = "<html><body><h1>Soil Moisture and BMP180 Sensor Data</h1>";
  message += "<p>Sensor 1 Data: <span id='sensor1Data'>Loading...</span></p>";
  message += "<p>Sensor 2 Data: <span id='sensor2Data'>Loading...</span></p>";
  message += "<p>Pressure: <span id='pressureData'>Loading...</span></p>";
  message += "<p>Temperature: <span id='temperatureData'>Loading...</span></p>";
  
  // JavaScript to periodically request new data from the server
  message += "<script>setInterval(function() {"
             "var xhr = new XMLHttpRequest();"
             "xhr.open('GET', '/data', true);"
             "xhr.onreadystatechange = function() {"
             "if (xhr.readyState == 4 && xhr.status == 200) {"
             "var data = JSON.parse(xhr.responseText);"
             "document.getElementById('sensor1Data').innerHTML = data.sensor1 + '% (' + data.status1 + ')';"
             "document.getElementById('sensor2Data').innerHTML = data.sensor2 + '% (' + data.status2 + ')';"
             "document.getElementById('pressureData').innerHTML = data.pressure + ' hPa';"
             "document.getElementById('temperatureData').innerHTML = data.temperature + ' °C';"
             "}};}, 1000);</script>";

  message += "</body></html>";
  server.send(200, "text/html", message);
}

// Function to handle sensor data response
void handleSensorData() {
  // Prepare JSON response with sensor data
  String jsonResponse = "{";
  jsonResponse += "\"sensor1\": " + String(sensor1_percent) + ", ";
  jsonResponse += "\"status1\": \"" + status1 + "\", ";
  jsonResponse += "\"sensor2\": " + String(sensor2_percent) + ", ";
  jsonResponse += "\"status2\": \"" + status2 + "\", ";
  jsonResponse += "\"pressure\": " + String(pressure) + ", ";
  jsonResponse += "\"temperature\": " + String(temperature);
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}

// Main setup function to initialize everything
void setup() {
  Serial.begin(115200);
  
  // Initialize Wi-Fi and sensors
  connectToWiFi();
  initializeBMP180();

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/data", handleSensorData);
  server.begin();

  pinMode(sensor1_pin, INPUT);
  pinMode(sensor2_pin, INPUT);

  Serial.println("Server started, listening for requests...");
}

// Main loop function to handle everything
void loop() {
  // Read sensor data
  readMoistureSensors();
  readBMP180();
  
  // Handle web server requests
  server.handleClient();
  
  // Optional: You can add a delay here if required
}
