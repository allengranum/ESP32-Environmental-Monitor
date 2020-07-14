#include "ConfigMgr.h"
#include <string.h>
#include "esp_onboarding.h"

ConfigMgr::ConfigMgr(char* fileName) {
    if (fileName != NULL)
      strcpy(&configFileName[0], fileName);
}

void ConfigMgr::begin() {
  initialize();
}

boolean ConfigMgr::initialize(void) {
  boolean status = false;
      
  if (!initialized) {
//    Serial.printf("\ninitialize: Initializing\n");
//    delay(1000);
    
    if (SPIFFS.begin(true)){
      if(!SPIFFS.exists(configFileName)) {
        Serial.println("Config file doesn't exist. Creating it...");
        delay(1000);
        File file = SPIFFS.open(configFileName, FILE_WRITE);
        if (file){
          initialized = true;
          status = true;
          file.close();
        } else {
          Serial.print("Failed to open file ");
          Serial.print(configFileName);
          Serial.println(" for reading.");
        }
      } else {
        Serial.println("initialize: Config file exists");
        delay(1000);
        initialized = true;
        status = true;
      }
    } else {
      Serial.println("An Error has occurred while mounting SPIFFS");
      delay(1000);
    }
  } else {
//    Serial.println("initialize: Already initialized");
//    delay(1000);
    status = true;
  }
//  Serial.println("initialize: Finished");
//  delay(1000);
  return status;
}

// ************************************
//          Get / Set APIs
// ************************************

char* ConfigMgr::getCustomValue(char* key) {
  char* result = (char*)malloc(sizeof(char*));
  result = NULL;
//  Serial.printf("getCustomValue: [%s]\n",key);
//  delay(500);
  if (initialize()) {
//    Serial.printf("getCustomValue: passed initialize\n",key);
//    delay(500);
    if (NULL != configFileName) {
      File file = SPIFFS.open(configFileName, FILE_READ);
      if (file){
      //  Serial.printf("getCustomValue: got a file handle\n",key);
      //  delay(500);
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
        Serial.print(configFileName);
        Serial.println(" for reading.");
      }
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

void ConfigMgr::deleteConfigData(void) {
  Serial.println("=============== DELETE CONFIG FILE ===============");
  if (initialize()) {
    SPIFFS.remove(configFileName);
  }
  Serial.println("=============== DELETE CONFIG FILE COMPLETE ===============");
}

/*
 * setDeviceName
 */
void ConfigMgr::setDeviceName(char* deviceName) {
  setKeyValue(DEVICE_NAME_KEY, deviceName, true);
}
 
/*
 * SaveWifiInfo
 */
void ConfigMgr::setWifiInfo(char* ssid, char* password) {
  char* value = (char*) malloc((strlen(ssid)+strlen(password)+2) * sizeof(char)); // +2 because of delimiter and null terminator
  sprintf(value, "%s%s%s", ssid, VALUE_DELIMITER, password);
  setKeyValue( WIFI_KEY, value, false);
}
// ********************************************
//               Helper Methods
// ********************************************

char* ConfigMgr::getNextLine(File file) {
  // TODO: ignore blank lines
  boolean eolDetected = false;
  char* line = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
  int index = 0;
  char tempBuf[8];
  
  while (file.available() && !eolDetected) {
    char currentChar = file.read();
//    sprintf(tempBuf, "0x%02x", currentChar);
//    Serial.print("getNextLine: Read character: [");
//    Serial.print(currentChar);
//    Serial.print("], hex:[");
//    Serial.print(tempBuf);
//    Serial.println("]");
    line[index] = currentChar;

//    file.read((uint8_t*)&line[index], sizeof(char));
    if (currentChar == '\n' || currentChar == '\0' || currentChar=='\r') {
      line[index] = '\0';
      eolDetected = true;
    }
    if (eolDetected) {
//      Serial.println("getNextLine: End of line detected.");
    }
    index++;
  }
  if (index == 0) {
    free(line);
    line = NULL;
  }
//  Serial.printf("getNextLine: line size: %d\n", strlen(line));
  return line;
}

char* ConfigMgr::getKey(char* line) {
  char tempBuf[256];
  strncpy(tempBuf, line, sizeof(line)+1);
  return strtok(tempBuf, KVP_DELIMITER);
}

char* ConfigMgr::getValue(char* line) {
  char* result = NULL;
  
  if (line != NULL) {
  char* tempBuf = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
    strcpy(tempBuf, line);
    char* foo = strtok(tempBuf, KVP_DELIMITER);
    foo = strtok(NULL, KVP_DELIMITER);
    result = foo;
  }
  return result;
}

boolean ConfigMgr::lineMatchesKey(char* line, char* key) {
  boolean status = false;
  if (strlen(line) > strlen(key)) {
    if (0 == (strncmp(line, key, strlen(key)))) {
      status = true;
    }
  }
  return status;
}

char* ConfigMgr::getDeviceName(void) {
  char* result = NULL;
  if (initialize()) {
    File file = SPIFFS.open(configFileName, FILE_READ);
    if (file){
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, DEVICE_NAME_KEY)) {
            result = getValue(line);
          }
          free(line);
        }
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(configFileName);
      Serial.println(" for reading.");
    }
  }
  return result;
}

boolean ConfigMgr::keyExists(char* key) {
  boolean result = false;
  File file = SPIFFS.open(configFileName, FILE_READ);
    if (file){
      while (result == false && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
              result = true;
          }
        }
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(configFileName);
      Serial.println(" for reading.");
    }
  return result;
}

/*
 * keyValueExists
 */
boolean ConfigMgr::keyValueExists(char* key, char* value) {
  boolean result = false;
  File file = SPIFFS.open(configFileName, FILE_READ);
    if (file){
      while (result == false && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
            if (0 == strcmp(value, getValue(line))) {
              result = true;
            }
          }
        }
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(configFileName);
      Serial.println(" for reading.");
    }
  return result;  
}

boolean ConfigMgr::setKeyValue(char* key, char* value, boolean replaceExisting) {
  boolean result = false;
  Serial.printf("saveKeyValue: key: [%s], value: [%s]\n", key, value);

  if (initialize()) {
//    Serial.println("saveKeyValue: CP 0.1");
    if (!keyExists(key)) {
//      Serial.println("saveKeyValue: CP 1");
      File file = SPIFFS.open(configFileName, FILE_APPEND);
      if (file){
//        Serial.println("saveKeyValue: CP 2");
        file.print(key);
        file.print(KVP_DELIMITER);
        file.println(value);
        file.print("\0");
        file.println("");
        file.close();
//        dumpConfig();
//        Serial.println("saveKeyValue: CP 2.1");
      } else {
        Serial.printf("Failed to open file [%s] for reading\n", configFileName);
      }
    } else {
      Serial.printf("WARNING: key/value [%s]/[%s] already exists\n", key, value);
      if (replaceExisting) {
        Serial.println("Replacing existing key/value with new one");
        deleteKey(key);
//        dumpConfig();
        setKeyValue(key, value, false);
      } else {
        Serial.println("replaceExisting = false. Not replacing existing key/value.");
      }
    }
  } else {
    Serial.println("saveKeyValue: CP WTF");
  }
  return result;
}

void ConfigMgr::deleteKey(char* key) {
  char* tempFileBuffer[MAX_LINES_IN_CONFIG_FILE];
  int index = 0;
//  Serial.printf("deleteKey: key: [%s]\n", key);
  if (initialize()) {
    // read all lines into buffer EXCEPT the matching one (there should be no duplicates)
    if (keyExists(key)) {
//      Serial.println("deleteKey: opening the file");
      File file = SPIFFS.open(configFileName, FILE_READ);
      if (file){
        int lineCount = 0;
        while (file.available()) {
          char* line = getNextLine(file);
//          Serial.printf("deleteKey: Current line[%d] = [%s], length = %d\n", lineCount++, line, strlen(line));
          if (!(lineMatchesKey(line, key)) && 0 != strlen(line)) {
            tempFileBuffer[index++] = line;
          } else {
//            Serial.printf("deleteKey: skipping line = [%s]\n",line);
          }
        }
//        Serial.println("deleteKey: closing the file");
        file.close();

        // delete the file
//        Serial.printf("deleteKey: deleting the config file\n");
        SPIFFS.remove(configFileName);
        
        // re-write the file from the buffer
//        Serial.println("deleteKey: opening the file");
        File file = SPIFFS.open(configFileName, FILE_WRITE);
        for (int currentLine=0 ; currentLine<index ; currentLine++) {
//          Serial.printf("deleteKey: writing line = [%s]\n", tempFileBuffer[currentLine]);
          file.println(tempFileBuffer[currentLine]);
        }
//        Serial.println("deleteKey: closing the file");
        file.close();
      }
    }
  }
//  Serial.printf("deleteKey: Dump the config file with the key [%s] removed\n", key);
//  dumpConfig();
//  Serial.println("deleteKey: End");
}

void ConfigMgr::deleteKeyValue(char* key, char* value) {
  char* tempFileBuffer[MAX_LINES_IN_CONFIG_FILE];
  int index = 0;
  Serial.printf("deleteKeyValue: key: [%s] value[%s]\n", key, value);
  if (initialize()) {
    // read all lines into buffer EXCEPT the matching one (there should be no duplicates)
    if (keyValueExists(key, value)) {
      File file = SPIFFS.open(configFileName, FILE_READ);
      if (file){
        while (file.available()) {
          char* line = getNextLine(file);
          Serial.printf("deleteKeyValue: Current line = [%s]\n", line);
          if (!(lineMatchesKey(line, key) && (0 == strcmp(getValue(line), value)))) {
            tempFileBuffer[index++] = getNextLine(file);
          }
        }
        file.close();

        // delete the file
        SPIFFS.remove(configFileName);
        
        // re-write the file from the buffer
        File file = SPIFFS.open(configFileName, FILE_WRITE);
        for (int currentLine=0 ; currentLine<index ; currentLine++) {
          file.println(tempFileBuffer[currentLine]);
        }
        file.close();
      }
    }
  }
//  Serial.println("deleteKeyValue: CP END");
}

WifiInfo* ConfigMgr::getSsids(void) {
  WifiInfo* result = (wifiInfo_t*)malloc(sizeof(struct wifiInfo_t));
  if (espConfigInit()) {    
    char** savedSSIDs = (char**)malloc(MAX_WIFI_ENTRIES * sizeof(char*));
    int ssidCount = 0;
    
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);

    if (file){
      while (file.available()) {
        char* line = getNextLine(file);        
        if (NULL != line) {
          if (lineMatchesKey(line, WIFI_KEY)) {
            char* knownSsid = getSsidFromValue(getValue(line));
            savedSSIDs[ssidCount] = knownSsid;
            ssidCount++;
          }
        }
      }
      file.close();
    } else {
      Serial.printf("Failed to open file %s for reading.\n", CONFIG_FILE_NAME);
    }
    Serial.printf("Found %d saved SSID", ssidCount);
    if (ssidCount >1 ) {
      Serial.printf("s");
    }
    Serial.printf("\n");

    result->ssid = savedSSIDs;
    result->numberOfSSIDsFound = ssidCount;
  }
  return result;
}

/*
 * getWifiPasswordFromValue
 */
static char* ConfigMgr::getWifiPasswordFromValue(char* value) {
  char* result = NULL;
  if (NULL != value) {
    char* tempBuf = (char*)malloc(strlen(value)+1 * sizeof(char));
    strncpy(tempBuf, value, strlen(value)+1);
    char* tmpPtr=NULL;
    tmpPtr = strtok(tempBuf, VALUE_DELIMITER);
    tmpPtr = strtok(NULL, VALUE_DELIMITER);
    result = tmpPtr;        
  }
  return result;
}

/*
 * getWifiPassword
 */
char* ConfigMgr::getWifiPassword(char* ssid) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (result == NULL && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, WIFI_KEY)) {
            char* currentKnownSsid = getSsidFromValue(getValue(line));
            if(0 == strcmp(ssid, currentKnownSsid)) {
              result = getWifiPasswordFromValue(getValue(line));
            } else {
              free(currentKnownSsid);
            }
          }
        }
      }
      file.close();
    } else {
      Serial.printf("Failed to open file %s for reading\n", CONFIG_FILE_NAME);
    }
  }
  return result;
}

void ConfigMgr::dumpConfig(void) {
//  Serial.println("dumpConfig: starting");
//  delay(500);
  if (initialize()) {
    File file = SPIFFS.open(configFileName, FILE_READ);
    if (NULL != file) {
      Serial.println("File Contents:");
      Serial.println("---------------------------------------");
      File file = SPIFFS.open(configFileName, FILE_READ);
      if (file){
        if (file.available()) {
          Serial.print("    ");
        } else {
          Serial.printf("    File is empty\n");
        }
        while (file.available()) {
          char currentChar = file.read();
          Serial.write(currentChar);
          if (currentChar == '\n' && file.available()) {
            Serial.print("    ");
          }
        }
        file.close();
      } else {
        Serial.print("Failed to open file ");
        Serial.print(configFileName);
        Serial.println(" for reading.");
      }
      Serial.println("---------------------------------------");
    } else {
      Serial.println("dumpConfig: Config file does not exist");
    }
  }
}
