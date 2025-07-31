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
const int ledPin = D5;      // Real LED controlled by button on V0
const int green = D1;       // Speed indicator
const int pwmLed = D2;      // PWM LED controlled by slider on V1

// HX711 load cell pins
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

// Load cell setup
HX711 scale;
float weight;
float calibration_factor = 790.4571429;

// V0 → On/Off button for LED on D5
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  digitalWrite(ledPin, value);
}

// V1 → Slider to control brightness of LED on D2
BLYNK_WRITE(V1)
{
  int brightness = param.asInt();         // 0–255
  analogWrite(pwmLed, brightness);        // Set PWM brightness
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Set pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(pwmLed, OUTPUT);       // Set D2 (pwmLed) as output
  digitalWrite(ledPin, LOW);     // Start with D5 off
  analogWrite(pwmLed, 0);        // Start with D2 LED OFF

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
    Serial.println("Insufficient Speed");
  } 
  else if (weight > 25 && weight <= 140) {
    digitalWrite(green, HIGH);
    Serial.println("Optimum Speed");
  } 
  else if (weight > 140 && weight <= 1100) {
    digitalWrite(green, HIGH);
    Serial.println("Overspeed, Brakes ON");
  } 
  else {
    digitalWrite(green, LOW);
  }

  // Sleep HX711 briefly
  scale.power_down();
  delay(150);
  scale.power_up();
}
