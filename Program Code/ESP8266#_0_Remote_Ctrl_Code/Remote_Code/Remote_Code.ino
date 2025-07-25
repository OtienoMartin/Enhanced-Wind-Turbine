#include <SoftwareSerial.h>
#include <Arduino.h>
#include "HX711.h"
#include <stdlib.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

float weight;
float calibration_factor = 790.4571429; // enter the calibration factor obtained earlier

// Pin wiring
int green = D1;
int red = D2;
int blue = D5;

/*
   D1 powers the locking mechanism which inturn powers the assembly rotation.
   D2 powers the electric breaking system. It has remote functionality too.
   D5 powers the remotepower on/off. 
 */

HX711 scale;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);

  scale.set_scale(790.4571429);
  //scale.set_scale(790.4571429);  // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare(); // reset the scale to 0

  Serial.println("After setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);

  long zero_factor = scale.read_average();

  Serial.println("weight:");
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D5, OUTPUT);
}

void loop() {
  float weight = scale.get_units(10); // average of 10 readings

  // === Clamp small values near zero to exactly zero ===
  // This ensures that when the load is removed (or nearly zero), no residual noise appears as a reading
  if (abs(weight) < 1.0) {  // You can adjust the threshold (1.0) if needed
    weight = 0;
  }

  Serial.print("one reading:\t");
  Serial.println(weight, 1);
  Serial.println(" m/s");

  if (weight <= 0) {
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    Serial.println(" Insufficient Speed");
  } 
  else if (weight > 25 && weight <= 140) {
    digitalWrite(D1, HIGH);
    digitalWrite(D2, LOW);
    Serial.println(" Optimum Speed");
  } 
  else if (weight > 140 && weight <= 1100) {
    digitalWrite(D1, HIGH);
    digitalWrite(D2, HIGH);
    Serial.println(" Overspeed, Brakes ON");
  }

  scale.power_down();  // put the ADC in sleep mode
  delay(150);
  scale.power_up();
}
