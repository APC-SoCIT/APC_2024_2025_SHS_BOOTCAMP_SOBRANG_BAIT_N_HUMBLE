#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
 
// WiFi credentials
const char* ssid = "Xiaomi_363F";
const char* password = "********";
 
// Device ID
const char* device_id = "arduino123";
 
// ACS712
const int ACS712_PIN = A0;
float energyWh = 0;
 
// Lights
struct Light {
  const char* id;
  int pin;
  String lastStatus;
};
 
Light lights[] = {
  {"living_room", 5, ""},
  {"bedroom", 6, ""},
  {"kitchen", 7, ""}
};
 
const int numLights = sizeof(lights) / sizeof(lights[0]);
 

WiFiSSLClient wifiClient;
HttpClient client(wifiClient, "***********", 443);
 

String userEmail = "*****************";
 
// GET Device info backend end
bool fetchUserEmail() {
  String path = "/device-info?device_id=" + String(device_id);
 
  Serial.println("Fetching user email...");
  client.get(path);
 
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
 
  Serial.print("Device info status: ");
  Serial.println(statusCode);
  Serial.println("Response:");
  Serial.println(response);
 
  if (statusCode == 200) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      return false;
    }
    userEmail = doc["email"].as<String>();
    Serial.print("User email: ");
    Serial.println(userEmail);
    return true;
  }
 
  Serial.println("Failed to get device info.");
  return false;
}
 
// POST register-light to databse
void registerLight(const char* lightId) {
  String path = "/register-light?light_id=" + String(lightId) + "&email=" + userEmail;
 
  String body = "{}";
  String contentType = "application/json";
 
  Serial.print("Registering light: ");
  Serial.println(lightId);
 
  client.post(path, contentType, body);
 
  int code = client.responseStatusCode();
  String resp = client.responseBody();
 
  Serial.print("Register Light Status: ");
  Serial.println(code);
  Serial.println("Response: ");
  Serial.println(resp);
}
 
// GET device-status 
void updateLightStatus() {
  Serial.println("Fetching device light status...");
 
  // Build URL with device_id and light_ids param
  String serverUrl = "/device-status?device_id=" + String(device_id);
  for (int i = 0; i < numLights; i++) {
    serverUrl += "&light_ids=" + String(lights[i].id);
  }
 
  client.get(serverUrl);
 
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
 
  Serial.print("Device Status HTTP code: ");
  Serial.println(statusCode);
 
  if (statusCode == 200) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);
 
    if (error) {
      Serial.print("JSON error: ");
      Serial.println(error.c_str());
      return;
    }
 
    // Update turn on and off the lights depending on the status on the backend
    for (int i = 0; i < numLights; i++) {
      const char* lightID = lights[i].id;
      int pin = lights[i].pin;
 
      if (doc.containsKey(lightID)) {
        String status = doc[lightID]["status"].as<String>();
 
        if (status != lights[i].lastStatus) {
          lights[i].lastStatus = status;
 
          if (status == "ON") {
            digitalWrite(pin, HIGH);
            Serial.print(lightID);
            Serial.println(" turned ON");
          } else {
            digitalWrite(pin, LOW);
            Serial.print(lightID);
            Serial.println(" turned OFF");
          }
        }
      } else {
        Serial.print("Missing light: ");
        Serial.println(lightID);
      }
    }
  } else {
    Serial.println("Failed to fetch device light data.");
    Serial.println("Response: ");
    Serial.println(response);
  }
}
 
// POST /energy
void postEnergyData(const char* lightId, float energyWh) {
  String path = "/energy";
  String contentType = "application/json";
 
  unsigned long epochSeconds = millis() / 1000; 
 
  String body = "{";
  body += "\"device_id\":\"" + String(device_id) + "\",";
  body += "\"light_id\":\"" + String(lightId) + "\",";
  body += "\"energy_wh\":" + String(energyWh, 2);
  body += "}";
 
  Serial.print("Posting energy data for ");
  Serial.print(lightId);
  Serial.print(": ");
  Serial.print(energyWh, 2);
  Serial.print(" Wh at ");
  Serial.println(epochSeconds);
 
  client.post(path, contentType, body);
 
  int code = client.responseStatusCode();
  String resp = client.responseBody();
 
  Serial.print("Energy POST status: ");
  Serial.println(code);
  Serial.println("Response: ");
  Serial.println(resp);
}
 
// Measure current simulated voltage 
float readCurrentACS712() {
  int sensorValue = analogRead(ACS712_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  float current = (voltage - 2.5) / 0.185;  
 
  current = abs(current);  
  Serial.print("ACS712 Current RMS: ");
  Serial.print(current, 2);
  Serial.println(" A");
 
  return current;
}
 
// Setup
void setup() {
  Serial.begin(9600);
  delay(2000);
 
  // Init pins
  for (int i = 0; i < numLights; i++) {
    pinMode(lights[i].pin, OUTPUT);
    digitalWrite(lights[i].pin, LOW);
  }
 
  // WiFi connect
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 
  // Get user email from backend
  if (!fetchUserEmail()) {
    Serial.println("Could not fetch user email. Stopping.");
    while (true) { delay(1000); }
  }
 
  // Register each light
  for (int i = 0; i < numLights; i++) {
    registerLight(lights[i].id);
    delay(500);
  }
 
  Serial.println("Setup complete!");
}
 
// Main loop
void loop() {
  static unsigned long lastLightUpdate = 0;
  static unsigned long lastEnergyUpdate = 0;
  unsigned long now = millis();
 
  // Update light status every 10 sec
  if (now - lastLightUpdate >= 10000) {
    updateLightStatus();
    lastLightUpdate = now;
  }
 
  // Post energy data every 15 sec
  if (now - lastEnergyUpdate >= 15000) {
    float current = readCurrentACS712();
    float powerW = current * 220.0;  
    float energyWhDelta = (powerW * 15.0) / 3600.0; 
 
    for (int i = 0; i < numLights; i++) {
      postEnergyData(lights[i].id, energyWhDelta);
    }
 
    lastEnergyUpdate = now;
  }
 
  delay(500);
}