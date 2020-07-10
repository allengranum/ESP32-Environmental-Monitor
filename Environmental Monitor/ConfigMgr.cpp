#include "ConfigMgr.h"
#include <string.h>

ConfigMgr::ConfigMgr(char* fileName) {
    strcpy(&configFileName[0], fileName);
}

int ConfigMgr::getDisplayBackligtBrightness() {

}

void begin() {

}

void ConfigMgr::setHumidityCalibration(float value) {
  humidityCalibrationValue = value;
}

void ConfigMgr::setHumidityCalibration(float value) {
  temperatureCalibrationValue = value;
}

char* ConfigMgr::getCustomValue(char* key) {
  char* result = (char*)malloc(sizeof(char*));
  result = NULL;
//  Serial.printf("getCustomValue: [%s]\n",key);
//  delay(500);
  if (espConfigInit()) {
//    Serial.printf("getCustomValue: passed espConfigInit\n",key);
//    delay(500);
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
//      Serial.printf("getCustomValue: got a file handle\n",key);
//      delay(500);
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  } else {
//    Serial.printf("getCustomValue: wtf!\n",key);
//    delay(500);
  }
  return result;
}

float ConfigMgr::getCustomValueFloat(char* key) {
  char* result = getCustomValue(key);
  float value = 0;
  if (result) {
    value = atof(result);
  }
  return value;
}

int ConfigMgr::getCustomValueInt(char* key) {
  char* result = getCustomValue(key);
  int value = 0;
  if (result) {
    value = atoi(result);
  }
  return value;
}