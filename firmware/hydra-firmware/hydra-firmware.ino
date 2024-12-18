#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // Include ArduinoJson library
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// WiFi Credentials
const char* ssid = "Fullhouse";
const char* password = "279levansy";

// MQTT Broker Configuration
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC_LED "thcntt3/v1/device/control/leds"
#define MQTT_TOPIC_WATER "thcntt3/v1/device/water/level"

// Pin Definitions
const int relayPin = D6;      // Pin for Relay
#define WATER_SENSOR A0        // Water Level Sensor Pin
#define LED_RED D4            // Red Warning LED
#define LED_GREEN D5          // Green Status LED

// Water Level Thresholds
#define WATER_THRESHOLD_LOW 300
#define WATER_THRESHOLD_HIGH 700

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16x2 LCD
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Timing Variables
unsigned long previousMillis = 0;
const long interval = 30000;  // Interval for sending sensor data (30 seconds)

// Function to connect to WiFi
void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Convert payload to String
  String jsonMessage = "";
  for (unsigned int i = 0; i < length; i++) {
    jsonMessage += (char)payload[i];
  }
  Serial.println(jsonMessage);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonMessage);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Control relay based on the command
  String command = doc["message"];
  Serial.print("Command: ");
  Serial.println(command);
  if (command == "ON") {
    digitalWrite(relayPin, LOW); // Relay active low
    Serial.println("Relay turned ON");
  } else if (command == "OFF") {
    digitalWrite(relayPin, HIGH); // Relay off
    Serial.println("Relay turned OFF");
  } else {
    Serial.println("Invalid command");
  }
}

// Function to connect to the MQTT broker
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("WaterLevelMonitor")) {
      Serial.println("connected");
      mqttClient.subscribe(MQTT_TOPIC_LED); // Subscribe to LED control topic
      Serial.print("Subscribed to topic: ");
      Serial.println(MQTT_TOPIC_LED);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Update LCD display based on water level
void updateDisplay(int waterLevel) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Level:");
  lcd.setCursor(0, 1);
  lcd.print(waterLevel);
  lcd.print(" ");  // Space for clarity

  // Update status based on water level
  if (waterLevel < WATER_THRESHOLD_LOW) {  // Low water level
    lcd.print("LOW");
    digitalWrite(LED_RED, HIGH);   // Turn on Red LED
    digitalWrite(LED_GREEN, LOW);  // Turn off Green LED
  } else {  // Normal water level or higher
    lcd.print("NORMAL");
    digitalWrite(LED_RED, LOW);    // Turn off Red LED
    digitalWrite(LED_GREEN, HIGH); // Turn on Green LED
  }
}

// Publish water level to MQTT broker
void publishWaterLevel(int waterLevel) {
  char levelStr[10];
  snprintf(levelStr, sizeof(levelStr), "%d", waterLevel);
  mqttClient.publish(MQTT_TOPIC_WATER, levelStr);
  Serial.print("Published water level: ");
  Serial.println(levelStr);
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize I2C and LCD
  Wire.begin(D2, D1);  // SDA, SCL for I2C
  lcd.init();
  lcd.backlight();

  // Configure Pin Modes
  pinMode(relayPin, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(WATER_SENSOR, INPUT);
  digitalWrite(relayPin, HIGH); // Start with relay off
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Connect to WiFi
  setupWiFi();

  // Setup MQTT Client
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);

  // Initial LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Monitor");
}

void loop() {
  // Ensure MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Check timing for water level monitoring
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read water level
    int waterLevel = analogRead(WATER_SENSOR);
    
    // Update LCD display
    updateDisplay(waterLevel);
    
    // Publish to MQTT broker
    publishWaterLevel(waterLevel);
  }
}