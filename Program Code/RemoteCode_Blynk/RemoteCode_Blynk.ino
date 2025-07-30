#define BLYNK_TEMPLATE_ID "TMPL2Q6tmZ_Cq"
#define BLYNK_TEMPLATE_NAME "WTurbine"
#define BLYNK_AUTH_TOKEN "SIEeJ54SaIcyeiJoXek66JHBSdaKKpRi"

#include <SoftwareSerial.h>
#include <Arduino.h>
#include "HX711.h"
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "Space, Time & Gravity";
char pass[] = "Wilhelm&30";

// Pin definitions
const int ledPin = D5; // Real LED connected to D5
const int green = D1;
const int red = D2;

// HX711 load cell pins
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

// Load cell setup
HX711 scale;
float weight;
float calibration_factor = 790.4571429;

// Button widget on V0 controls physical LED
BLYNK_WRITE(V0)
{
  int value = param.asInt();         // Get button state (0 or 1)
  digitalWrite(ledPin, value);       // Control real LED on D5
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Set pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off

  // Initialize load cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare(); // Set current reading as 0
}

void loop() {
  Blynk.run();

  // Read weight value
  weight = scale.get_units();
  Serial.print("Speed reading:\t");
  Serial.print(weight, 1);
  Serial.println(" m/s");

  // Decision logic based on weight
  if (weight <= 0) {
    digitalWrite(green, LOW);
    digitalWrite(red, LOW);
    Serial.println("Insufficient Speed");
  } 
  else if (weight > 25 && weight <= 140) {
    digitalWrite(green, HIGH);
    digitalWrite(red, LOW);
    Serial.println("Optimum Speed");
  } 
  else if (weight > 140 && weight <= 1100) {
    digitalWrite(green, HIGH);
    digitalWrite(red, HIGH);
    Serial.println("Overspeed, Brakes ON");
  } 
  else {
    digitalWrite(green, LOW);
    digitalWrite(red, LOW);
  }

  // Sleep HX711 briefly
  scale.power_down();
  delay(150);
  scale.power_up();
}
