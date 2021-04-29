
#include <Wire.h>
#include "Adafruit_MCP9808.h"

#define LED_OUT_PIN        21
#define MOTION_SENSOR_PIN  19
#define HEAT_BED_PIN       18

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

float temperature = 0;
int ledTimeoutMarker = 0;
int noMotionTimeout = 300000; /* ms */
int ledState = 0;

void motionSensorInit() {
  Serial.printf("\n\ncalibrating...\n");
  int motionSensorCalibrationTime = 5;
  digitalWrite(MOTION_SENSOR_PIN, LOW);
  delay(motionSensorCalibrationTime * 1000);
  Serial.printf("Calibration complete.\n");
}
void setup() {
  pinMode(LED_OUT_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(HEAT_BED_PIN, INPUT);

  Serial.begin(9600);
  while (!Serial); //waits for serial terminal to be open, necessary in newer arduino boards.

  motionSensorInit();
  
  if (!tempsensor.begin(0x18)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  }
  Serial.println("Found MCP9808!");

  tempsensor.setResolution(1); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms
}

void loop() {
  Serial.println("-------------------------------------------------------");
  
  readTemperature();

  readMotion();

  //readHeatbed();

  setLight();

//  delay(2000);
}


void readTemperature() {
  tempsensor.wake();   // wake up, ready to read!

  float c = tempsensor.readTempC();
  if (c != temperature) {
    temperature = c;
    /* Send data */
  }

  tempsensor.shutdown_wake(1);
}

void readMotion() {
  if (HIGH == digitalRead(MOTION_SENSOR_PIN)) {
    ledTimeoutMarker = millis();
//    Serial.println("Motion detected = 1");
  } else {
//    Serial.println("Motion detected = 0");
  }
}

void readHeatbed() {
  if (HIGH == digitalRead(HEAT_BED_PIN)) {
    ledTimeoutMarker = millis();
    Serial.println("Heat bed turned on.1");
  } else {
    Serial.println("Heat bed turned off.");
  }
}
void setLight() {
  int diff = millis() - ledTimeoutMarker;
  Serial.print("setLight: diff = ");
  Serial.println(diff);
  Serial.print("setLight: ledState = ");
  Serial.println(ledState);
  if ((millis() - ledTimeoutMarker) >  noMotionTimeout) {
    if (ledState == 1) {
      Serial.println("setLight: noMotionTimeout expired");
      digitalWrite(LED_OUT_PIN, LOW);
      ledState = 0;
      Serial.println("led OFF");
    }
  } else {
    Serial.print("setLight: noMotionTimeout time left = ");
    Serial.println(noMotionTimeout - diff);
    if (ledState == 0) {
      digitalWrite(LED_OUT_PIN, HIGH);
      ledState = 1;
      Serial.println("led ON");
    }
  }
}
