#include <Arduino.h>
#include "HX711.h"
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

// === WiFi Credentials ===
const char* ssid = "Space, Time & Gravity";
const char* password = "Wilhelm&30";

// === ThingSpeak Settings ===
WiFiClient client;
unsigned long counterChannelNumber = 3019455;
const char* myCounterReadAPIKey = "K4DO472XCRON3J78";
const int FieldNumber1 = 1;

// === HX711 Pins (D6 = GPIO12, D7 = GPIO13) ===
const int LOADCELL_DOUT_PIN = D6;
const int LOADCELL_SCK_PIN  = D7;
float calibration_factor = 790.4571429;
float weight;

// === Output Pins ===
const int LOCK_PIN  = D1; // Green — locks assembly
const int BRAKE_PIN = D2; // Red — brake control
const int POWER_PIN = D5; // Blue — remote control

// === Timing Intervals (in milliseconds) ===
unsigned long lastThingSpeakRead = 0;
const unsigned long readInterval = 5000;     // Remote command read every 5s

unsigned long lastWeightRead = 0;
const unsigned long weightInterval = 1000;   // Weight reading every 1s

// === HX711 Object ===
HX711 scale;

// === Setup ===
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BRAKE_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  ThingSpeak.begin(client);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("Scale initialized.");
}

// === Remote Control Handler (D5) ===
void handleRemoteControl() {
  int A = ThingSpeak.readLongField(counterChannelNumber, FieldNumber1, myCounterReadAPIKey);
  Serial.print("Remote Power Command (A): ");
  Serial.println(A);

  if (A == 0 || A == 1) {
    digitalWrite(POWER_PIN, A);
  } else {
    Serial.println("Invalid value received from ThingSpeak.");
  }
}

// === Load Cell Logic (D1 & D2) ===
void handleWeightLogic() {
  weight = scale.get_units(3); // faster average

  // Clamp near-zero noise
  if (abs(weight) < 1.0) {
    weight = 0;
  }

  Serial.print("Weight: ");
  Serial.print(weight, 1);
  Serial.println(" kg");

  if (weight <= 0) {
    digitalWrite(LOCK_PIN, LOW);
    digitalWrite(BRAKE_PIN, LOW);
    Serial.println("Status: Insufficient Speed");
  } else if (weight > 25 && weight <= 140) {
    digitalWrite(LOCK_PIN, HIGH);
    digitalWrite(BRAKE_PIN, LOW);
    Serial.println("Status: Optimum Speed");
  } else if (weight > 140 && weight <= 1100) {
    digitalWrite(LOCK_PIN, HIGH);
    digitalWrite(BRAKE_PIN, HIGH);
    Serial.println("Status: Overspeed, Brakes ON");
  }

  scale.power_down();
  delay(100);
  scale.power_up();
}

// === Main Loop ===
void loop() {
  unsigned long now = millis();

  // Check remote power control
  if (now - lastThingSpeakRead >= readInterval) {
    handleRemoteControl();
    lastThingSpeakRead = now;
  }

  // Check weight logic
  if (now - lastWeightRead >= weightInterval) {
    handleWeightLogic();
    lastWeightRead = now;
  }
}
