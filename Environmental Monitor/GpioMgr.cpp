#include "GpioMgr.h"
#include "EspPins.h"
#include <Wire.h>
#include <DHT.h>

float g_temperatureCalibrationValue = 0;
float g_humidityCalibrationValue = 0;

DHT dht(DHT_SENSOR_PIN, DHTTYPE);

void configButtonPressHandler() {
  Serial.println("CONFIG_BUTTON pressed!!!");
  //deleteConfigData();
}

GpioMgr::GpioMgr(DeviceInfo* info) {
    deviceInfo = info;
}

void GpioMgr::begin() {
    dht.begin();
    Wire.begin();
    motionSensorInit();
}

void GpioMgr::loop() {
    // read and update all sensor values;
    readData();
}

void GpioMgr::motionSensorInit() {
  Serial.printf("\n\ncalibrating...\n");
  int motionSensorCalibrationTime = 5;
  digitalWrite(MOTION_SENSOR_PIN, LOW);
  delay(motionSensorCalibrationTime * 1000);
  Serial.printf("Calibration complete.\n");
}

int GpioMgr::buttonInit() {
    int retVal = 0;
    int configButtonState = digitalRead(CONFIG_BUTTON);
    int deleteConfigButtonState = digitalRead(DELETE_CONFIG_BUTTON);
   
    // if (deleteConfigButtonState == LOW) {
    //     Serial.printf("\n\nDelete config button pressed during boot.\n");
    //     Serial.printf("Deleting saved onboarding data...\n");
    //     envMonConfig.deleteConfigData();
    // }
    
    if (configButtonState == LOW) {
        Serial.printf("\n\nConfig button pressed during boot.\n");
        Serial.printf("    Performing onboarding...\n");
        retVal = 1;
    }
    
    attachInterrupt(CONFIG_BUTTON, configButtonPressHandler, FALLING);
    Serial.printf("CONFIG_BUTTON ISR attached.\n");

    return retVal;
}

void GpioMgr::readData() {
  readTemperature();
  readHumidity();
  readMotionData();
  readLight();
}

void GpioMgr::readTemperature() {
    deviceInfo->setTemperature(dht.readTemperature() + g_temperatureCalibrationValue);
}

void GpioMgr::readHumidity() {
    deviceInfo->setHumidity(dht.readHumidity() + g_humidityCalibrationValue);
}

void GpioMgr::readLight() {
    deviceInfo->setLightValue(analogRead(LIGHT_SENSOR_PIN));
}

void GpioMgr::readMotion() {
    deviceInfo->setMotionDetected(digitalRead(MOTION_SENSOR_PIN));
}