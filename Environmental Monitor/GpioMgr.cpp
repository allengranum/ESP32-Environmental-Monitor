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

GpioMgr::GpioMgr(DeviceInfo info) {
    deviceInfo = info;
}

void GpioMgr::loop() {
    // read and update all sensor values;
}

void GpioMgr::begin() {
    dht.begin();
    Wire.begin();
    motionSensorInit();
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

void readData() {
//  Serial.printf("----------------------\n");
  readTemp();
#ifndef HEADLESS
  if (currentTemp != oldTemp) {
    displayTemp(false);
  }
#endif

  readHumidity();
#ifndef HEADLESS
  if (currentHumidity != oldHumidity) {
    displayHumidity(false);
  }
#endif
  
  readMotionData();
  readLightData();
  
  if (WiFi.status() == WL_CONNECTED) {
    readDateTime();
#ifndef HEADLESS
    displayDateTime(false);
#endif
  }

  timeToSendData = false;
}

void readTemp() {
  oldTemp = currentTemp;
  currentTemp = dht.readTemperature() + g_temperatureCalibrationValue;
//  Serial.print("Temperature: ");
//  Serial.print(currentTemp);
//  Serial.println(" *C ");

//  Serial.printf("Temperature: %d  *C \n", currentTemp);
}

void readHumidity() {
  oldHumidity = currentHumidity;
  currentHumidity = dht.readHumidity() + g_humidityCalibrationValue;
//  Serial.print("Humidity: ");
//  Serial.print(currentHumidity);
//  Serial.println(" %\t");
  
//  Serial.printf("Humidity: %d %\n", currentHumidity);
}


void GpioMgr::readLightData() {
  // Light data is read at the time of publishing.
  // Serial.printf("Light: %d\n", analogRead(LIGHT_SENSOR_PIN));
}

bool GpioMgr::readMotionData() {
  bool motionStateChanged = false;
  
  if(digitalRead(MOTION_SENSOR_PIN) == HIGH ){
    motionDetectedTime = millis();
    if (!currentMotionDetectedState) {
      currentMotionDetectedState = true;
      motionStateChanged = true;
//      digitalWrite(STATUS_LED, HIGH);
      displayMotionIcon();
      Serial.println("Motion detected");      
    }
  } else {
    if( currentMotionDetectedState == true && (millis() -  motionDetectedTime) > noMotionDelay ) {
      currentMotionDetectedState = false;
      motionStateChanged = true;
//      digitalWrite(STATUS_LED, LOW);
      removeMotionIcon();
      Serial.println("No motion detected");
    }
  }
  return motionStateChanged;
}