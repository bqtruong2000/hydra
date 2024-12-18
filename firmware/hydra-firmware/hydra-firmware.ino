#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// WiFi Credentials
#define WIFI_SSID "Fullhouse"
#define WIFI_PASSWORD "279levansy"

// MQTT Broker Configuration
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC "thcntt3/v1/device/water/level"

// Pin Definitions
#define WATER_SENSOR A0  // Water Level Sensor Pin
#define LED_RED D4       // Red Warning LED
#define LED_GREEN D5     // Green Status LED

// Water Level Thresholds
#define WATER_THRESHOLD_LOW 300
#define WATER_THRESHOLD_HIGH 700

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 column and 2 rows
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Timing Variables
unsigned long previousMillis = 0;
const long interval = 30000;  // Interval for sending sensor data (5 seconds)

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize I2C and LCD
  Wire.begin(D2, D1);  // SDA, SCL for I2C
  lcd.init();
  lcd.backlight();

  // Configure Pin Modes
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(WATER_SENSOR, INPUT);

  // Initial LED States
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Connect to WiFi
  connectToWiFi();

  // Setup MQTT Client
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  // Initial LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Monitor");
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect("WaterLevelMonitor")) {
      Serial.println("Connected to MQTT Broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds");
      delay(5000);
    }
  }
}

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


void publishWaterLevel(int waterLevel) {
  // Convert water level to string for MQTT publish
  char levelStr[10];
  snprintf(levelStr, sizeof(levelStr), "%d", waterLevel);

  // Publish to MQTT broker
  mqttClient.publish(MQTT_TOPIC, levelStr);

  Serial.print("Published water level: ");
  Serial.println(levelStr);
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
    // Update previous time
    previousMillis = currentMillis;

    // Read water level
    int waterLevel = analogRead(WATER_SENSOR);

    // Update LCD display
    updateDisplay(waterLevel);

    // Publish to MQTT broker
    publishWaterLevel(waterLevel);
  }
}