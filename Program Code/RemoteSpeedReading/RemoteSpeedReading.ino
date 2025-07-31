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
float speed;
float calibration_factor = 790.4571429;

BlynkTimer timer;

bool alertSent = false;
bool shutdownSent = false;

// V0 → On/Off button for LED on D5
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  digitalWrite(ledPin, value);
}

// V1 → Slider to control brightness of LED on D2
BLYNK_WRITE(V1)
{
  int brightness = param.asInt();         
  analogWrite(pwmLed, brightness);        
}

// V2 → Display wind speed
void sendSpeedToBlynk() {
  Blynk.virtualWrite(V2, speed);  
}

// V3 → Update system status
void readSpeedAndUpdateStatus() {
  speed = scale.get_units();

  // Force small drifts to zero
  if (abs(speed) < 0.9) {
    speed = 0.0;
  }

  Serial.print("Speed reading:\t");
  Serial.print(speed, 1);
  Serial.println(" m/s");

  if (speed == 0.0) {
    digitalWrite(green, LOW);
    Serial.println("Insufficient Speed");
    Blynk.virtualWrite(V3, "Low Speed");
    alertSent = false;
    shutdownSent = false;
  } 
  else if (speed > 25 && speed <= 140) {
    digitalWrite(green, HIGH);
    Serial.println("Optimum Speed");
    Blynk.virtualWrite(V3, "Optimum Speed");
    alertSent = false;
    shutdownSent = false;
  } 
  else if (speed > 140 && speed <= 200) {
    digitalWrite(green, HIGH);
    Serial.println("Brakes ON");
    Blynk.virtualWrite(V3, "Brakes ON");

    if (!alertSent) {
      Blynk.logEvent("overspeed_alert", String("Speed reached ") + speed + " m/s");
      alertSent = true;
    }
    shutdownSent = false;
  } 
  else if (speed > 200) {
    digitalWrite(green, LOW);
    digitalWrite(ledPin, LOW);  
    Serial.println("⚠️ Emergency Shutdown Activated!");
    Blynk.virtualWrite(V3, "⚠ SHUTDOWN");

    if (!shutdownSent) {
      Blynk.logEvent("emergency_shutdown", String("Speed exceeded safe limit: ") + speed + " m/s. System shut down.");
      shutdownSent = true;
    }
    alertSent = false;
  }

  scale.power_down();
  delayMicroseconds(150000);  // 150 ms
  scale.power_up();
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(ledPin, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(pwmLed, OUTPUT);
  digitalWrite(ledPin, LOW);
  analogWrite(pwmLed, 0);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();

  // Timers
  timer.setInterval(1000L, sendSpeedToBlynk);
  timer.setInterval(1000L, readSpeedAndUpdateStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}
